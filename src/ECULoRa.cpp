#include "ECULoRa.h"
#include "ECUHardware.h"

volatile int lora_data_received = 0;
volatile char lora_data[ECU_LORA_BUFSIZE];

int get_ecu_lora_data(uint8_t* buf, int buf_len)
{
    noInterrupts();
    int i = 0;
    if (lora_data_received > 0)
    {
        for (i = 0; i < lora_data_received && i < buf_len; i++)
        {
            buf[i] = lora_data[i];
        }
        buf[i] = 0;
        lora_data_received = 0;
    }
    interrupts();
    return i;
}

void onReceive(int packetSize)
{
    int i;
    for (i = 0; i <  packetSize && i < ECU_LORA_BUFSIZE; i++)
    {
        lora_data[i] = (char)LoRa.read();
    }
    lora_data_received = i;
}

void ECULoRaInit(int ss_pin, int reset_pin, int interrupt_pin, SPIClass* spi, int lora_sck, int lora_miso, int lora_mosi)
{
#ifdef ARDUINO_TEENSY41
    spi->setSCK(lora_sck);
    spi->setMISO(lora_miso);
    spi->setMOSI(lora_mosi);

    LoRa.setSPI(*spi);
#endif

    LoRa.setPins(ss_pin, reset_pin, interrupt_pin);

    delay(1);
    LoRa.setSpreadingFactor(SF);
    delay(1);
    LoRa.setSignalBandwidth(BANDWIDTH);
    delay(1);
    LoRa.setTxPower(RF_POWER);
    delay(1)
    if (!LoRa.begin(FREQUENCY))
    {
        SerialUSB.println("WARN: LoRa Initialization Failed");
    }

    // Enable interrupt handling for LoRa
    LoRa.onReceive(onReceive);
    LoRa.receive();
}
