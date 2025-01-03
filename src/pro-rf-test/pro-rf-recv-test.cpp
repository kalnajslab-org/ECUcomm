#include "ECUHardware.h"
#include "ECULoRa.h"

int LED = 13;

ECULoRaMsg_t ecu_msg;

void setup()
{
    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);

    SerialUSB.begin(115200);
    delay(1000);

    SerialUSB.println(String(__FILE__) + " build " + __DATE__ + " " + __TIME__);

    if (!ECULoRaInit(12, 7, 6, &SPI, SCK, MISO, MOSI)) {
        SerialUSB.println("LoRa Init Failed");
        while (1); // halt
    } else {
        SerialUSB.println("LoRa Init Success");
    }
}

void loop()
{
    if (ecu_lora_get_msg(&ecu_msg))
    {
        char pbuf[100];
        snprintf(pbuf, sizeof(pbuf),
            "n:%05lu id:%05lu rssi:%5d snr:%5.1f ferr:%5ld",
            ecu_msg.count, ecu_msg.id, ecu_lora_rssi(), ecu_lora_snr(), ecu_lora_frequency_error());
        // received a packet
        SerialUSB.print(pbuf);
        SerialUSB.print(" <");
        SerialUSB.print((char*)ecu_msg.data);
        SerialUSB.println(">");
    }
    delay(500);
    digitalWrite(LED, !digitalRead(LED));
}