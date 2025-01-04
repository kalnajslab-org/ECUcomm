#include "ECULoRa.h"
#include "ECUHardware.h"

volatile uint lora_packet_bytes = 0;
volatile ECULoRaPacket_t lora_packet;
volatile uint32_t recvd_msg_count = 0;
volatile uint32_t xmitd_msg_count = 0;
void onReceive(int packetSize);

bool ECULoRaInit(
    int ss_pin,
    int reset_pin,
    int interrupt_pin,
    SPIClass *spi,
    int lora_sck,
    int lora_miso,
    int lora_mosi)
{
    spi->setSCK(lora_sck);
    spi->setMISO(lora_miso);
    spi->setMOSI(lora_mosi);
    LoRa.setSPI(*spi);

    LoRa.setPins(ss_pin, reset_pin, interrupt_pin);

    delay(1);
    if (!LoRa.begin(FREQUENCY))
    {
        return false;
    }
    delay(1);
    LoRa.setSpreadingFactor(SF);
    delay(1);
    LoRa.setSignalBandwidth(BANDWIDTH);
    delay(1);
    LoRa.setTxPower(RF_POWER);
    delay(1);
    LoRa.enableCrc();

    // Enable receive interrupt handling for LoRa
    LoRa.onReceive(onReceive);
    LoRa.receive();

    return true;
}

bool ecu_lora_send_msg(uint8_t *data, uint8_t len)
{
    ECULoRaPacket_t payload;
    payload.id = xmitd_msg_count;
    LoRa.beginPacket();
    LoRa.write((uint8_t *)(&payload.id), sizeof(payload.id));
    LoRa.write(data, len);
    bool status = LoRa.endPacket();
    if (status)
    {
        xmitd_msg_count++;
    }
    // The receive interrupt must be re-enabled after sending a message.
    // This doesn't seem to be mentioned in the LoRa library documentation,
    // but it does appear in the LoRaDuplexCallback example.
    LoRa.receive();
    return status;
}

bool ecu_lora_get_msg(ECULoRaMsg_t *msg)
{
    noInterrupts();
    uint i = 0;
    if (lora_packet_bytes > 0)
    {
        if (lora_packet_bytes <= sizeof(ECULoRaPacket_t))
        {
            uint8_t data_len = lora_packet_bytes - ECU_LORA_PACKET_HDR_SIZE;
            if (data_len > 0)
            {
                for (i = 0; i < data_len && i < (ECU_LORA_DATA_BUFSIZE - 1); i++)
                {
                    msg->data[i] = lora_packet.data[i];
                }
                msg->data[i] = 0;
                msg->id = lora_packet.id;
                msg->count = recvd_msg_count;
                msg->data_len = data_len;
                lora_packet_bytes = 0;
            }
            else
            {
                SerialUSB.println("ECULoRa: Invalid data length in received packet");
            }
        }
    }
    interrupts();
    return (i > 0);
}

void onReceive(int packetSize)
{
    uint i;
    for (i = 0; i < (uint)packetSize && i < sizeof(ECULoRaPacket_t); i++)
    {
        ((uint8_t *)(&lora_packet))[i] = (uint8_t)LoRa.read();
    }
    lora_packet_bytes = i;
    recvd_msg_count++;
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