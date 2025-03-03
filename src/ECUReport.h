#ifndef ECU_REPORT_H
#define ECU_REPORT_H

#include <Arduino.h>
#define ETL_NO_STL
#define ETL_NO_INITIALIZER_LIST

#include "etl/array.h"

#define ECU_REPORT_REV 1

// *** NOTE: There are several functions in ECUReport.cpp that must be updated whenever 
// the report is modified, and they must exactly match the data structure defined here. 
// Don't forget to check the data types (uint16_t, etc.) as well as the bitfield sizes ***

// The ECUReport_t defines the ECU data fields that will be sent over the LoRa network.
// AN INSTANTATION OF THIS STRUCTURE DOES NOT CONTAIN BIT-PACKED DATA. THE MEMORY
// ALLOCATION OF C BITFIELDS IS COMPLETELY DEPENDENT ON THE COMPILER. THIS STRUCTURE IS USED
// JUST TO DEFINE THE FIELDS AND THEIR SIZES. THE ACTUAL BIT-(UN)/PACKING IS DONE IN 
// ecu_report_serialize() and ecu_report_deserialize().
// However, one of the fringe benefits of using bitfields is that the compiler will 
// truncate assignments to fit the bitfield size.

// The ECUReportBytes_t is a byte array that will hold the serialized ECUReport_t data structure.
// etl::bit_stream_writer is used to serialize the data structure into a byte array (ECUReportBytes_t).
// etl::bit_stream_reader is used to deserialize the byte array back into the data structure.

// Since we don't have an automatic method to determine the number of bits in the struct,
// we have to define it here.
// *** Correct ECU_REPORT_SIZE_BITS when ECUReport_t is modified ***
// (Use copliot to create this sum by by propmting: "sum of bitfield sizes in ECUReport_t")
#define ECU_REPORT_SIZE_BITS (4 + 1 + 9 + 11 + 13 + 11 + 8 + 1 + 32 + 32 + 16 + 5 + 17 + 25 + 8 + 1 + 14 + 10 + 8 + 17 + 1 + 12 + 24 + 24)
// Total bits: 304 bits = 38 bytes
// Round up to full byte adding 1. Perhaps there is a way to do this more elegantly with math in the #define?
#define ECU_REPORT_SIZE_BYTES (1+ECU_REPORT_SIZE_BITS/8)

// ECUReport_t defines documents/defines the data structure that will be sent over the LoRa network.
// The data structure is defined with bitfields to establish the bit packing.
// In compliance with C bitfield rules, the bitfield type must be large enough to hold the bitfield.
struct ECUReport_t
{
    uint8_t  rev :        4;
    uint8_t  heat_on :    1;  // Heater on (bool)
    uint16_t v5 :         9;  // V5*100  (0-511  : 0.00V to 5.11V)
    uint16_t v12 :       11;  // V12*100 (0-2047 : 0.00V to 20.47V)
    uint16_t v56 :       13;  // V56*100 (0-8191 : 0.00V to 81.91V)
    uint16_t board_t :   11;  // (Board temperature+100)*10 (0-2047 : -100.0C to 104.8C)
    uint8_t  switch_mA :  8;  // Switch current (mA): (0ma - 255mA)
    uint8_t  gps_valid :  1;  // GPS Valid (bool)
    int32_t  gps_lat :   32;  // GPS Latitude*1e6 (degrees*1e6)
    int32_t  gps_lon :   32;  // GPS Longitude*1e6 (degrees*1e6)
    uint16_t gps_alt:    16;  // GPS Altitude (meters)
    uint8_t  gps_sats:    5;  // Number of satellites n (0 to 31)
    uint32_t gps_date:   17;  // GPS Date (MMDDYY - Year is 20YY)
    uint32_t gps_time:   25;  // GPS Time (HHMMSSSS - Seconds are in 100ths)
    uint8_t  gps_age_secs:8;  // Age of GPS data in seconds (0 to 255) 255 = greater than 254
    uint8_t  rs41_valid:  1;  // RS41 data valid (bool)
    uint16_t rs41_airt : 14;  // (RS41 Air Temperature+100)*100 (0-16383 : -100.00C to 63.83C) 
    uint16_t rs41_hum:   10;  // RS41 Humidity*10 (0-1023 : 0.0% to 102.3%)
    uint8_t  rs41_hst:    8;  // RS41 Humidity Sensor Temperature+100 (0-255 : -100C to 125C)
    uint32_t rs41_pres:  17;  // RS41 Pressure*100 (0-131071 : 0.0hPa to 1310.71hPa) (should we do log10?)
    uint8_t  rs41_pcb_h:  1;  // RS41 PCB Heater On (bool)
    uint16_t tsen_airt:  12;  // Raw
    uint32_t tsen_ptemp: 24;  // Raw
    uint32_t tsen_pres:  24;  // Raw
};

// A byte array to hold the serialized ECUReport_t data structure.
// etl::bit_stream_writer is used to serialize ECUReport_t into ECUReportBytes_t.
// etl::bit_stream_reader is used to deserialize ECUReportBytes_t into ECUReport_t.
typedef etl::array<uint8_t, ECU_REPORT_SIZE_BYTES> ECUReportBytes_t;

// Initialize all fields in an ECUReport_t.
void ecu_report_init(ECUReport_t& report);
// Add ECU health to an ECUReport_t.
void add_ecu_health(float v5, float v12, float v56, float board_t, float switch_mA, ECUReport_t& report);
// Add statuses to an ECUReport_t.
void add_status(bool heat_on, ECUReport_t& report);
// Add GPS data to an ECUReport_t.
void add_gps(bool valid, double lat, double lon, double alt, uint sats, uint32_t date, uint32_t time, uint age_secs, ECUReport_t& report);
// Add RS41 data to an ECUReport_t.
void add_rs41(bool valid, float airt, float hum, float hst, float pres, bool pcb_h, ECUReport_t& report);
// Add TSEN data to an ECUReport_t.
void add_tsen(uint16_t airt, uint32_t prest, uint32_t pres, ECUReport_t& report);
// Serialize an ECUReport_t.
ECUReportBytes_t ecu_report_serialize(ECUReport_t& report);
// Deserialize an ECUReportBytes_t.
ECUReport_t ecu_report_deserialize(ECUReportBytes_t& data);
// Print an ECUReport_t.
// If print_bin is true, the binary representation of the report will be included in the print.
void ecu_report_print(ECUReport_t& ecu_report, bool print_bin=false);
// Print a uint32_t in binary, with zero fill.
// n is the number to print.
// w is the number of bits to print.
void ecu_bin_print(uint32_t n, uint8_t w=8);

#endif //_ECU_REPORT_H_
