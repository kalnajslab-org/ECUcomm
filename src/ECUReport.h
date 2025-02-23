#ifndef ECU_REPORT_H
#define ECU_REPORT_H

#include <Arduino.h>
#define ETL_NO_STL
#define ETL_NO_INITIALIZER_LIST

#include "etl/array.h"

#define ECU_REPORT_REV 1

#define ECU_REPORT_SIZE_BITS (4 + 1 + 9 + 11 + 13 + 11 + 1 + 32 + 32 + 16)

// 4 + 1 + 9 + 11 + 13 + 11 + 1 + 32 + 32 + 16 = 130 bits
// 130 bits / 8 bits/byte = 16.25 bytes
// Round up by adding 1 byte. Perhaps there is a way to do this more elegantly with math in the #define?
#define ECU_REPORT_SIZE_BYTES (1+ECU_REPORT_SIZE_BITS/8)

struct ECUReport_t
{
    uint8_t rev :        4;
    uint8_t heat_on :    1;  // Heater on (bool)
    uint16_t v5 :        9;  // V5*100  (0-511  : 0.00V to 5.11V)
    uint16_t v12 :      11;  // V12*100 (0-2047 : 0.00V to 20.47V)
    uint16_t v56 :      13;  // V56*100 (0-8191 : 0.00V to 81.91V)
    uint16_t board_t :  11;  // (Board temperature+100)*10 (0-2047 : -100.0C to 104.7C)
    uint8_t gps_valid :  1;  // GPS Valid (bool)
    int32_t gps_lat :   32;  // GPS Latitude*1e6 (degrees*1e6)
    int32_t gps_lon :   32;  // GPS Longitude*1e6 (degrees*1e6)
    uint16_t gps_alt:   16;  // GPS Altitude (meters)
};

void bin_print(uint32_t n, uint8_t w=8);
void ecu_report_init(ECUReport_t& report);
void ecu_report_print(ECUReport_t* ecu_report);
void add_ecu_health(float v5, float v12, float v56, float board_t, ECUReport_t& report);
void add_status(bool heat_on, ECUReport_t& report);
void add_gps(bool valid, double lat, double lon, double alt, ECUReport_t& report);
etl::array<uint8_t, ECU_REPORT_SIZE_BYTES> ecu_report_serialize(ECUReport_t& report);
ECUReport_t ecu_report_deserialize(etl::array<uint8_t, ECU_REPORT_SIZE_BYTES>& data);

#endif //_ECU_REPORT_H_

