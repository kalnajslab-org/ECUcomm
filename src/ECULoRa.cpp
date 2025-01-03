#include "ECULoRa.h"
#include "ECUHardware.h"

volatile uint lora_data_received = 0;
volatile char lora_data[ECU_LORA_DATA_BUFSIZE];
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
    if (status) { 
        xmitd_msg_count++;
    }
    // The receive interrupt must be re-enabled after sending a message.
    // This doesn't seem to be mentioned in the LoRa library documentation.
    LoRa.receive();
    return status;
}

bool ecu_lora_get_msg(ECULoRaMsg_t *msg)
{
    noInterrupts();
    uint i = 0;
    if (lora_data_received > 0)
    {
        ECULoRaPacket_t *payload = (ECULoRaPacket_t *)lora_data;

        // WHENEVER YOU CHANGE THE PAYLOAD STRUCTURE, YOU MUST CHANGE THIS LINE
        uint data_len = lora_data_received - sizeof(payload->id);

        for (i = 0; i < data_len && i < (ECU_LORA_DATA_BUFSIZE - 1); i++)
        {
            msg->data[i] = payload->data[i];
        }
        msg->data[i] = 0;
        msg->id = payload->id;
        msg->count = recvd_msg_count;
        msg->data_len = data_len;

        lora_data_received = 0;
    }
    interrupts();
    return (i > 0);
}

void onReceive(int packetSize)
{
    uint i;
    for (i = 0; i < (uint)packetSize && i < ECU_LORA_DATA_BUFSIZE; i++)
    {
        lora_data[i] = (uint8_t)LoRa.read();
    }
    lora_data_received = i;
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