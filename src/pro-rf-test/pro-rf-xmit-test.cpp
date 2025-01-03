#include "ECUHardware.h"
#include "ECULoRa.h"

// Status LED on pin 13
int LED = 13;

#define MSG_LEN 100
uint8_t toSend[MSG_LEN + 1];

void setup()
{
    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH); // Turn on status LED

    SerialUSB.begin(115200);
    delay(1000);

    SerialUSB.println(String(__FILE__) + " build " + __DATE__ + " " + __TIME__);

    if (!ECULoRaInit(12, 7, 6, &SPI, SCK, MISO, MOSI)) {    
        SerialUSB.println("LoRa Init Failed");
        while (1);  // halt
    } else {
        SerialUSB.println("LoRa Initialized");
    } 
}

int counter = 0;
void loop()
{
    counter++;
    for (int i = 0; i < MSG_LEN; i++)
    {
        toSend[i] = (char)('0' + random(0, 9));
    }
    toSend[MSG_LEN] = 0;

    ecu_lora_send_msg(toSend, MSG_LEN);

    digitalWrite(LED, !digitalRead(LED));
    SerialUSB.println((char*)toSend);

    delay(1000);
}