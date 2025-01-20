#include <map>
#include "ECUHardware.h"
#include "ECULoRa.h"

#ifdef ECUCOMMLEADER
ECULoRaMode_t my_ecu_lora_mode = LORA_LEADER;
#else
#ifdef ECUCOMMFOLLOWER
ECULoRaMode_t my_ecu_lora_mode = LORA_FOLLOWER;
#else
#ifdef ECUCOMMFREERUN
ECULoRaMode_t my_ecu_lora_mode = LORA_FREERUN;
#endif
#endif
#endif

// Status LED on pin 13
int LED = 13;

#define MSG_LEN 50
uint8_t toSend[MSG_LEN + 1];
unsigned long lastSendTime = 0;                      // last send time
unsigned long interval = SEND_INTERVAL_MS;           // interval between sends
unsigned long lastBlinkTime = 0;                     // last blink time
unsigned long blink_interval = SEND_INTERVAL_MS / 4; // interval between blinks
uint32_t last_received_id = 0;                       // to identify dropped packets

std::map<ECULoRaMode_t, String> ecu_lora_mode_names = {
    {LORA_LEADER, "LEADER"},
    {LORA_FOLLOWER, "FOLLOWER"},
    {LORA_FREERUN, "FREERUN"}};

void setup()
{
    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH); // Turn on status LED

    SerialUSB.begin(115200);
    delay(3000);

    SerialUSB.println(String(__FILE__) + " build:" + __DATE__ + " " + __TIME__);
    SerialUSB.println(ecu_lora_mode_names[my_ecu_lora_mode] + " Frequency:" + FREQUENCY + " BW:" + BANDWIDTH + " SF:" + SF + " Interval:" + SEND_INTERVAL_MS);

    if (!ECULoRaInit(
            my_ecu_lora_mode,
            SEND_INTERVAL_MS,
            12, 7, 6,
            &SPI, SCK, MISO, MOSI))
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

void loop()
{

    // Generate a random message
    for (int i = 0; i < MSG_LEN; i++)
    {
        toSend[i] = (char)('0' + random(0, 9));
    }
    toSend[MSG_LEN] = 0;

    // Always send the message when in leader mode.
    // In FOLLOWER or FREERUN mode, send the message
    // when the interval has passed.
    if (my_ecu_lora_mode == LORA_LEADER || (millis() - lastSendTime > interval))
    {
        //        SerialUSB.println("interval:"+String(interval));
        ecu_lora_tx(toSend, MSG_LEN);
        lastSendTime = millis();
        if (my_ecu_lora_mode == LORA_FREERUN)
        {
            // In FREERUN mode, the interval is random
            interval = random(SEND_INTERVAL_MS * 0.1, SEND_INTERVAL_MS * 1.1);
        }
    }

    if (millis() - lastBlinkTime > blink_interval)
    {
        digitalWrite(LED, !digitalRead(LED));
        lastBlinkTime = millis();
    }

    ECULoRaMsg_t ecu_msg;
    if (ecu_lora_rx(&ecu_msg))
    {
        // received a packet
        int id_delta;
        char pbuf[100];
        if (last_received_id == 0)
        {
            last_received_id = ecu_msg.id;
            id_delta = 0;
        }
        else
        {
            id_delta = ecu_msg.id - last_received_id;
            last_received_id = ecu_msg.id;
        }
        snprintf(pbuf, sizeof(pbuf),
                 "%s n:%05lu id:%05lu delta:%d rssi:%5d snr:%5.1f ferr:%5ld l:%d",
                 ecu_lora_mode_names[my_ecu_lora_mode].c_str(), ecu_msg.count, ecu_msg.id, id_delta, ecu_lora_rssi(), ecu_lora_snr(), ecu_lora_frequency_error(), ecu_msg.data_len);
        SerialUSB.print(pbuf);
        SerialUSB.print(" <");
        for (int i = 0; i < ecu_msg.data_len; i++)
        {
            SerialUSB.print((char)ecu_msg.data[i]);
        }
        SerialUSB.println(">");
    }
}