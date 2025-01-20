# ECUcomm: End of Cable Unit Communications

## Overview

ECUcomm is a communication module designed for the End of Cable Unit (ECU) in the Strateole2 project. It facilitates reliable data exchange between the main board and the ECU using LoRa technology. The module is responsible for handling both low-level communication protocols and higher-level data structures and messaging.

## Key Features

- **LoRa Communication**: Utilizes LoRa radios for long-range, low-power communication.
- **Data Structures**: Defines and manages data structures for efficient data handling.
- **Messaging Protocol**: Implements a robust messaging protocol to ensure data integrity and reliability.

## LoRa

A LoRa radio cannot transmit and receive simultaneously. If both ends of the link transmit at
the nearly same time, the packets will interfere with each other and both will be lost.
A scheme has been devised to simulate a full-duplex link in this scenario.

Different modes are designated:
- **LEADER**: The node transmits messages periodically and continuously. These messages
  may simply perform a "keep-alive" function. The node which has a higher message bandwidth
  should use this mode.
- **FOLLOWER**: The node will transmit a queued message immediately after receiving a message.
- **FREERUN**: The node will receive and transmit messages, without any other consideration.
  Using this mode could lead to lossy communications, but is necessary for applications
  not using the LEADER/FOLLOWER mechanism.

The LEADER/FOLLOWER scheme relies on these rules:
- The LEADER has some "dead" time to receive a message, after it has sent one.
- The FOLLOWER does not require more than one message to reach the LEADER for
  every message sent by the LEADER.

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
|`onReceive()`| `LORA_LEADER,`LORA_FREERUN` | Called by a packet received interrupt. It reads the packet into a structure, and sets `
