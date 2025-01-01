#ifndef _ECULORA_H_
#define _ECULORA_H_

// Docs for LoRa are at https://github.com/sandeepmistry/arduino-LoRa.git
#include <LoRa.h>

#define ECU_LORA_BUFSIZE 200

void ECULoRaInit(int ss_pin, int reset_pin, int interrupt_pin, SPIClass* spi, int lora_sck, int lora_miso, int lora_mosi);

extern int get_ecu_lora_data(char *buf, int len);

#endif //_ECULORA_H_
