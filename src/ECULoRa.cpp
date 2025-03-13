#include "ECULoRa.h"

// Our operational mode.
volatile ECULoRaMode_t ecu_lora_mode;

// Set true when a message is received and available in rx_lora_packet
volatile bool rx_ready = false;
// The received packet.
volatile ECULoRaPacket_t rx_lora_packet;

// Set true when a message is in tx_lora_packet and ready to be send
volatile bool tx_queued = false;
// The packet to be sent.
volatile ECULoRaPacket_t tx_lora_packet;

volatile uint32_t recvd_msg_count = 0;
volatile uint32_t sent_msg_count = 0;

// The reporting interval for the leader mode.
// Messages will not be sent more frequently than this interval.
uint32_t leader_mode_report_interval_ms = 0;
volatile uint32_t last_send_time_ms = 0;

// Interrupt service routines.
void rxReadyISR(int packetSize);
void txDoneISR();

// Transmit the packet in tx_lora_packet now.
bool ecu_lora_tx_now();

bool ECULoRaInit(
    ECULoRaMode_t mode,
    int leader_report_interval_ms,
    int ss_pin,
    int reset_pin,
    int interrupt_pin,
    SPIClass *spi,
    int lora_sck,
    int lora_miso,
    int lora_mosi,
    long frequency,
    long bandwidth,
    int sf,
    int power)
{
#ifdef ARDUINO_TEENSY41
    spi->setSCK(lora_sck);
    spi->setMISO(lora_miso);
    spi->setMOSI(lora_mosi);
    LoRa.setSPI(*spi);
#endif

    ecu_lora_mode = mode;
    leader_mode_report_interval_ms = leader_report_interval_ms;

    LoRa.setPins(ss_pin, reset_pin, interrupt_pin);

    delay(1);
    if (!LoRa.begin(frequency))
    {
        return false;
    }
    delay(1);
    LoRa.setSpreadingFactor(sf);
    delay(1);
    LoRa.setSignalBandwidth(bandwidth);
    delay(1);
    LoRa.setTxPower(power);
    delay(1);
    LoRa.enableCrc();

    // Enable the receive and transmit interrupt handlers.
    LoRa.onReceive(rxReadyISR);
    LoRa.onTxDone(txDoneISR);

    // Start receiving messages.
    LoRa.receive();

    return true;
}

void rxReadyISR(int packetSize)
{
    uint i;

    // We can't be sure that this packet was sent by another ECULoRa system.
    // Make sure that a bogus packet will not overrun the buffer.
    // Recall that ECULoRaPacket_t.data_len is not transmitted in the packet.

    // So, just read the packet directly into the rx_lora_packet structure.
    for (i = 0; i < (uint)packetSize && i < sizeof(ECULoRaPacket_t) - sizeof(rx_lora_packet.data_len); i++)
    {
        ((uint8_t *)&rx_lora_packet)[i] = (uint8_t)LoRa.read();
    }

    // TODO: The following code must be updated whenever the ECULoRaPacket_t structure changes.
    rx_lora_packet.data_len = packetSize - sizeof(rx_lora_packet.id);

    // Signal that a packet is ready.
    rx_ready = true;

    // And count our received messages.
    recvd_msg_count++;

    // Send a queued tx packet now.
    if (tx_queued)
    {
        ecu_lora_tx_now();
        tx_queued = false;
    }
}

void txDoneISR()
{
    sent_msg_count++;

    // re-enable receive mode
    LoRa.receive();
}

bool ecu_lora_tx_now()
{
    // WARNING: This function may be called from the either interrupt or non-interrupt context.

    LoRa.beginPacket();

    // TODO: The following code must be updated whenever the ECULoRaPacket_t structure changes.
    LoRa.write((uint8_t *)(&tx_lora_packet.id), sizeof(tx_lora_packet.id));
    LoRa.write((uint8_t *)tx_lora_packet.data, tx_lora_packet.data_len);

    bool status = LoRa.endPacket(true);
    last_send_time_ms = millis();

    return status;
}

bool ecu_lora_tx(uint8_t *data, uint8_t len, bool immediate)
{
    if (immediate)
    {

        tx_lora_packet.id = sent_msg_count;
        tx_lora_packet.data_len = len;
        for (uint8_t i = 0; i < len && i < ECU_LORA_DATA_BUFSIZE; i++)
        {
            tx_lora_packet.data[i] = data[i];
        }
        ecu_lora_tx_now();
        return true;
    }

    if (ecu_lora_mode == LORA_FOLLOWER)
    {
        // Save the packet, to be sent from the rxReadyISR().

        // *** Disable interrupts while we access tx_queued and tx_lora_packet.
        noInterrupts();

        tx_lora_packet.id = sent_msg_count;
        tx_lora_packet.data_len = len;
        for (uint8_t i = 0; i < len && i < ECU_LORA_DATA_BUFSIZE; i++)
        {
            tx_lora_packet.data[i] = data[i];
        }
        tx_queued = true;

        // *** Re-enable interrupts.
        interrupts();

        return true;
    }

    // Do not send a message if we are in leader mode and the last message was sent too recently.
    if (ecu_lora_mode == LORA_LEADER && millis() - last_send_time_ms < leader_mode_report_interval_ms)
    {
        return false;
    }

    // Either we are in freerun mode, or the leader mode interval has passed.
    // Send the message.
    tx_lora_packet.id = sent_msg_count;
    tx_lora_packet.data_len = len;
    for (uint8_t i = 0; i < len && i < ECU_LORA_DATA_BUFSIZE; i++)
    {
        tx_lora_packet.data[i] = data[i];
    }

    bool status = ecu_lora_tx_now();

    return status;
}

bool ecu_lora_rx(ECULoRaMsg_t *msg)
{
    // *** Disable interrupts while we access rx_ready_bytes and received_lora_packet[].
    noInterrupts();
    bool msg_avail = rx_ready;

    if (rx_ready)
    {
        // Copy the received message into the user's message structure.
        msg->id = rx_lora_packet.id;
        msg->data_len = rx_lora_packet.data_len;
        {
            for (uint8_t i = 0; i < msg->data_len && i < (ECU_LORA_DATA_BUFSIZE - 1); i++)
            {
                msg->data[i] = rx_lora_packet.data[i];
            }
        }
        msg->count = recvd_msg_count;

        // Indicate that the message has been handled.
        rx_ready = false;
    }

    // *** Re-enable interrupts.
    interrupts();
    return msg_avail;
}

int ecu_lora_rssi()
{
    return LoRa.packetRssi();
}

float ecu_lora_snr()
{
    return LoRa.packetSnr();
}

long ecu_lora_frequency_error()
{
    return LoRa.packetFrequencyError();
}

// #endif // _ECULORA_H_