#ifndef ECU_REPORT_H
#define ECU_REPORT_H

#include <Arduino.h>
#define ETL_NO_STL
#define ETL_NO_INITIALIZER_LIST

#include "etl/bit_stream.h"
#include "etl/array.h"

#define ECU_REPORT_REV 1

#define ECU_REPORT_SIZE_BITS 96
#define ECU_REPORT_SIZE_BYTES (ECU_REPORT_SIZE_BITS/8)

struct ECUReport_t
{
    uint8_t rev :        4;
    uint8_t heat_on :    1;  // Heater on (bool)
    uint16_t v5 :        9;  // V5*100  (0-511  : 0.00V-5.11V)
    uint16_t v12 :      11;  // V12*100 (0-2047 : 0.00V-20.47V)
    uint16_t v56 :      13;  // V56*100 (0-8191 : 0.00V-81.91V)
    uint8_t gps_valid :  1;
    int32_t gps_lat :   32;  // GPS Latitude*1e6 (degrees*1e6)
    int32_t gps_lon :   32;  // GPS Longitude*1e6 (degrees*1e6)
    uint16_t gps_alt:   16;  // GPS Altitude (meters)
};

void bin_print(uint32_t n, uint8_t w=8);
void ecu_report_init(ECUReport_t* report);
void ecu_report_print(ECUReport_t* ecu_report);

#endif //_ECU_REPORT_H_

