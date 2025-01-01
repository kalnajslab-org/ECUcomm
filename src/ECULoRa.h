#ifndef _ECULORA_H_
#define _ECULORA_H_

// Docs for LoRa are at https://github.com/sandeepmistry/arduino-LoRa.git
#include <LoRa.h>

#define ECU_LORA_BUFSIZE 200

bool ECULoRaInit(int ss_pin, int reset_pin, int interrupt_pin, SPIClass* spi, int lora_sck, int lora_miso, int lora_mosi);
extern u_int get_ecu_lora_data(uint8_t *buf, int len);
extern volatile uint32_t ecu_lora_msg_count;
extern int ecu_lora_rssi();
extern float ecu_lora_snr();
extern long ecu_lora_frequency_error();


#endif //_ECULORA_H_
