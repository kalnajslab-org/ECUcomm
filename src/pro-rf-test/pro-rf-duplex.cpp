#include "ECUHardware.h"
#include "ECULoRa.h"

// Status LED on pin 13
int LED = 13;

#define MSG_LEN 50
uint8_t toSend[MSG_LEN + 1];
unsigned long lastSendTime = 0;     // last send time
unsigned long interval = SEND_INTERVAL_SECS*1000;           // interval between sends
unsigned long lastBlinkTime = 0;                            // last blink time
unsigned long blink_interval = (SEND_INTERVAL_SECS*1000)/4; // interval between blinks

void setup()
{
    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH); // Turn on status LED

    SerialUSB.begin(115200);
    delay(1000);

    SerialUSB.println(String(__FILE__) + " build " + __DATE__ + " " + __TIME__ + " interval " + SEND_INTERVAL_SECS);

    if (!ECULoRaInit(12, 7, 6, &SPI, SCK, MISO, MOSI))
    {
        SerialUSB.println("LoRa Init Failed");
        while (1)
            ; // halt
    }
    else
    {
        SerialUSB.println("LoRa Initialized");
    }
}

int counter = 0;
void loop()
{
    counter++;
    if (millis() - lastSendTime > interval)
    {
        for (int i = 0; i < MSG_LEN; i++)
        {
            toSend[i] = (char)('0' + random(0, 9));
        }
        toSend[MSG_LEN] = 0;

        ecu_lora_send_msg(toSend, MSG_LEN);
        //SerialUSB.println(String("sending ") + (char *)toSend);
        lastSendTime = millis();
    }

    if (millis() - lastBlinkTime > blink_interval)
    {
        digitalWrite(LED, !digitalRead(LED));
        lastBlinkTime = millis();
    }
    ECULoRaMsg_t ecu_msg;
    if (ecu_lora_get_msg(&ecu_msg))
    {
        char pbuf[100];
        snprintf(pbuf, sizeof(pbuf),
                 "received n:%05lu id:%05lu rssi:%5d snr:%5.1f ferr:%5ld",
                 ecu_msg.count, ecu_msg.id, ecu_lora_rssi(), ecu_lora_snr(), ecu_lora_frequency_error());
        // received a packet
        SerialUSB.print(pbuf);
        SerialUSB.print(" <");
        SerialUSB.print((char *)ecu_msg.data);
        SerialUSB.println(">");
    }
}