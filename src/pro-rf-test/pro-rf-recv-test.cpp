#include "ECUHardware.h"
#include "ECULoRa.h"

int LED = 13;

uint8_t lora_buffer[ECU_LORA_BUFSIZE];

void setup()
{
    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);

    SerialUSB.begin(115200);
    delay(1000);

    SerialUSB.println(String(__FILE__) + " build " + __DATE__ + " " + __TIME__);

    ECULoRaInit(12, 7, 6, &SPI, SCK, MISO, MOSI); 
    SerialUSB.println("LoRa Initialized");
}

void loop()
{
    if (get_ecu_lora_data(lora_buffer, ECU_LORA_BUFSIZE))
    {
        char pbuf[100];
        snprintf(pbuf, sizeof(pbuf),
            "rssi:%5d snr:%6.1f ferr:%6ld",
            ecu_lora_rssi(), ecu_lora_snr(), ecu_lora_frequency_error());
        // received a packet
        SerialUSB.print(pbuf);
        SerialUSB.print(" <");
        SerialUSB.print((char*)lora_buffer);
        SerialUSB.println(">");
    }
    delay(500);
    digitalWrite(LED, !digitalRead(LED));
}