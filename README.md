# ECUcomm: End of Cable Unit Communications

## Overview

ECUcomm is a communication module designed for the End of Cable Unit (ECU) in the Strateole2 project. It facilitates reliable data exchange between the main board and the ECU using LoRa technology. The module is responsible for handling both low-level communication protocols and higher-level data structures and messaging.

## Key Features

- **LoRa Communication**: Utilizes LoRa radios for long-range, low-power communication.
- **Data Structures**: Defines and manages data structures for efficient data handling.
- **Messaging Protocol**: Implements a messaging protocol layered on top of the LoRa transport.

## LoRa

*Summary: The communications are interrupt driven in both directions, so that time is not
 wasted in polling. In the leader/follower mode, after the leader transmits a message,
 **it guarantees a reception period** in which to receive a message from the follower.*

A LoRa radio cannot transmit and receive simultaneously. If both ends of the link transmit at
the nearly same time, the packets will interfere with each other and both will be lost.
A scheme has been devised to simulate a full-duplex link in this scenario.

Different modes are designated:
- **LORA_LEADER**: The node transmits messages periodically and continuously. These messages
  may simply perform a "keep-alive" function. The node which has a higher message bandwidth
  should use this mode.
- **LORA_FOLLOWER**: The node will transmit a queued message immediately after receiving a message.
- **LORA_FREERUN**: The node will receive and transmit messages, without any other consideration.
  Using this mode could lead to lossy communications, but is provided for applications
  not using the leader/follower protocol.

The leader/follower scheme has these characteristics:
- After it has sent a message, the leader has some "dead" time to receive a message.
- Given the "dead" space for message reception, here is a high probability that the
  leader will receive messages sent by the follower.
- The leader is prioritized in that it can continuously broadcast messages, without
  requiring acknowledgment from the follower.
- The follower must detect the presence of the leader before it will send a message
  to the follower. However, the follower can switch to freerun mode if it's desirable
  to force a message to be transmitted.

The intention is for the ECU to use LORA_LEADER, and the main system to us LORA_FOLLOWER.
Upon power up, the ECU will immediately begin transmitting periodic messages. In the fault 
situation where the main board cannot send and/or the ECU cannot receive, at least
the data from the ECU (in its default configuration) can be captured by the main board.

### Implementation

Design considerations:
- The [LoRa library](https://github.com/sandeepmistry/arduino-LoRa.git) provides hardware support (via SPI)
  for the RFM95W LoRa transceiver.
- StratoCore applications are typically implemented as a continuously running loop which
  polls a collection of functions each time through the loop. One of these functions will
  determine if a message has been received from the LoRa radio.
- Because there are many activities which need to be handled during the loop, we don't 
  want to wait to poll for message receiving or sending. The LoRa library interrupt 
  support will be utilized to minimize this.
- Data structures and flags are used to pass data between interrupt and non-interrupt context.
  `noInterrupts()` and `interrupts()` provide guards for these shared resources in the
  non-interrupt context.

| Function | Mode | |
|----------|------|-|
|`ECULoRaInit()`| All | Configure the LoRa hardware and enable the interrupt handlers. |
|`rxReadyISR()`| `LORA_LEADER,LORA_FREERUN` | Called by a received packet interrupt. It reads the packet into a structure, bumps `recvd_msg_count` and sets `rx_ready`.
|              | `LORA_FOLLOWER` | All of the above, and if `tx_ready` is set, it will transmit the queued packet, and clear `tx_ready`.|
|`txDoneISR()`| All | Called by the packet transmitted interrupt, it increments `sent_msg_count` and puts the radio back into receive mode. Using this interrupt insures that the radio is in tx mode for only the time required to transmit the message.|
|`ecu_lora_tx()`|`LORA_FOLLOWER`| Saves the data packet and sets `tx_ready`. |
|               |`LORA_LEADER`| If the minimum transmit interval has passed, the data packet is transmitted.|
|               |`LORA_FREERUN`| The data packet is transmitted. |
|`ecu_lora_rx()`| ALL | If `rx_ready` is set, the message is returned to the caller. Since `rx_ready` is set in the ISR, `ecu_lora_rx()` can be polled whenever convenient.|

### LoRa Testing ###

A pair of _SparkFun Pro RF_ LoRa modules, running the _pro-rf-duplex_ program, are used to test the LoRa transport.
The LoRa pair exchange messages, print the received messages, and track lost messages. 

Results of the tests:
| Modes | Send interval (ms) | Results |
|---------------|--------------------|---------|
|`LORA_LEADER/LORA_FOLLOWER`| 1000 | An occasional packet is dropped. |
|`LORA_FOLLOWER/LORA_FOLLOWER`| 1000 | In this scenario, the transmit intervals are random (+/-10% of assigned interval). There are many lost packets, as the radios are frequently walking on each other. Occasionally two packets will be received as one. This is understandable, as the transmit time is a large fraction of 1000ms. But even when  the interval is increased to 2000ms, there are still a significant number of clashes.|