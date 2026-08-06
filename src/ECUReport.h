#ifndef ECU_REPORT_H
#define ECU_REPORT_H

#include <Arduino.h>
#define DIV_ROUND_UP(Numerator, Denominator) (((Numerator) + (Denominator) - 1) / (Denominator))

#define ETL_NO_STL
#define ETL_NO_INITIALIZER_LIST

#include "etl/array.h"

#define ECU_REPORT_REV 7

#define ECU_MAX_RAW_BYTES 200  // Maximum raw byte size for ECU report (for union in ECUReport_t)

typedef uint8_t ECU_REPORT_TYPE_t;
enum ECU_REPORT_TYPE_ENUM : ECU_REPORT_TYPE_t {
    ECU_REPORT_DATA = 0,  // Standard ECU data report
    ECU_REPORT_RAW  = 1   // Raw bytes data report (string or binary)
};

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

// There are two message types defined:
// 1) ECU_REPORT_DATA: This is the standard ECU data report with all fields defined in ECUReport_t.
// 2) ECU_REPORT_RAW: This is a raw byte data report. It can be used to send arbitrary data (string or binary).
// Both message types start with the same header fields (rev and msg_type) to allow identification of the message type
// during deserialization. The remainder of the message is interpreted based on the msg_type field, and is
// defined by a union in the ECUReport_t struct.

// Bit definitions for the rs41_status byte (matches RS41::RS41StatusFlags_t field order):
constexpr uint8_t ECU_RS41_HIGH_INTERNAL_TEMP  = (1u << 0); // S.2: high internal temperature
constexpr uint8_t ECU_RS41_REGEN_TEMP_LOW      = (1u << 1); // S.3: regen temperature low
constexpr uint8_t ECU_RS41_PTU_FAILURE         = (1u << 2); // S.4: PTU failure
constexpr uint8_t ECU_RS41_FLASH_FAILURE       = (1u << 3); // S.5: flash failure
constexpr uint8_t ECU_RS41_LOW_INPUT_VOLTAGE   = (1u << 4); // E.6: low input voltage
constexpr uint8_t ECU_RS41_NOT_CALIBRATED      = (1u << 5); // E.7: not calibrated
constexpr uint8_t ECU_RS41_NO_PRESSURE_MODULE  = (1u << 6); // E.8: no pressure module
constexpr uint8_t ECU_RS41_DISCONNECTED_BOOM   = (1u << 7); // E.9: disconnected boom

// ECUReport_t defines documents/defines the data structure that will be sent over the LoRa network.
// The data structure is defined with bitfields to establish the bit packing.
// In compliance with C bitfield rules, the bitfield type must be large enough to hold the bitfield.
struct ECUReport_t
{
    uint8_t            rev :      4; // Report structure revision number
    ECU_REPORT_TYPE_t  msg_type : 4;  // Message type defines which structure is used in the message.
    uint8_t  ecu_id:              8;  // ECU ID

        union {
        struct {
            // msg_type = ECU_REPORT_DATA
            uint8_t  heat_on :      1;  // Heater on (bool)
            uint8_t  rs41_en :      1;  // RS41 enabled (bool)
            uint8_t  tsen_power :   1;  // TSEN power (bool)
            uint16_t v5 :           9;  // V5*100  (0-511  : 0.00V to 5.11V)
            uint16_t v12 :         11;  // V12*100 (0-2047 : 0.00V to 20.47V)
            uint16_t v56 :         13;  // V56*100 (0-8191 : 0.00V to 81.91V)
            uint16_t board_t :     11;  // (Board temperature+100)*10 (0-2047 : -100.0C to 104.8C)
            uint8_t  temp_setpoint: 8;  // Setpoint temperature+100: (0-255 : -100C to 155C) 
            uint8_t  switch_mA :    8;  // Switch current (mA): (0ma - 255mA)
            uint8_t  gps_valid :    1;  // GPS Valid (bool)
            int32_t  gps_lat :     32;  // GPS Latitude*1e6 (degrees*1e6)
            int32_t  gps_lon :     32;  // GPS Longitude*1e6 (degrees*1e6)
            uint16_t gps_alt:      16;  // GPS Altitude (meters)
            uint8_t  gps_sats:      5;  // Number of satellites n (0 to 31)
            uint32_t gps_date:     19;  // GPS Date (DDMMYY - Year is 20YY)
            uint32_t gps_time:     25;  // GPS Time (HHMMSSSS - Seconds are in 100ths)
            uint8_t  gps_age_secs:  8;  // Age of GPS data in seconds (0 to 255) 255 = greater than 254
            uint8_t  rs41_valid:    1;  // RS41 data valid (bool)
            uint8_t  rs41_regen:    1; // RS41 regeneration active (bool)
            uint16_t rs41_airt :   16;  // (T+100) x436.9067 (0-65535 : -100 to +50 degC)
            uint16_t rs41_hum:     16;  // (RH+20) x543.1333 (0-65535 : -20 to +100 %RH)
            uint16_t rs41_hst:     16;  // (T+100) x436.9067 (0-65535 : -100 to +50 degC)
            uint16_t rs41_pres:    16;  // (ln(P)-3.9120) x21525.87 (0-65535 : ~50 to 1050 hPa)
            uint8_t  rs41_hdg:      8;  // RS41 magnetic heading (0-255 : 0 to 360 degrees)
            uint8_t  rs41_status:   8;  // RS41 status flags byte (8 flags packed as bits, see ECU_RS41_*)
            uint8_t  rs41_pcb_h:    1;  // RS41 PCB Heater On (bool)
            uint16_t tsen_airt:    12;  // Raw
            uint32_t tsen_ptemp:   24;  // Raw
            uint32_t tsen_pres:    24;  // Raw
            uint16_t cpu_temp :    11;   // (CPU temperature+100)*10 (0-2047 : -100.0C to 104.8C)
        }; 
        struct {
            // msg_type = ECU_REPORT_RAW
            // A generic raw data message. ECU_MAX_RAW_BYTES will be transmitted, 
            // but only n_bytes will be valid.
            uint8_t n_bytes : 8;  // Number of valid bytes in raw[]
            uint8_t raw[ECU_MAX_RAW_BYTES]; 
        };
    };
};

// Since we don't have an automatic method to determine the number of bits in the serialized ECU_REPORT_DATA struct,
// we have to define it here.
// *** Update ECU_DATA_REPORT_SIZE_BITS when ECUReport_t is modified ***
// (Use copliot to create this sum by by prompting: "sum of bitfield sizes in ECUReport_t")

#define ECU_DATA_REPORT_SIZE_BITS (4 + 4 + 8 + 1 + 1 + 1 + 9 + 11 + 13 + 11 + 8 + 8 + 1 + 32 + 32 + 16 + 5 + 19 + 25 + 8 + 1 + 1 + 16 + 16 + 16 + 16 + 8 + 8 + 1 + 12 + 24 + 24 + 11)
// Total bits: 371 bits = 47 bytes
// Round up to bytes
#define ECU_DATA_REPORT_SIZE_BYTES DIV_ROUND_UP(ECU_DATA_REPORT_SIZE_BITS, 8)

// The ECUReportBytes_t is a byte array that will hold the serialized ECUReport_t data structure.
// etl::bit_stream_writer is used to serialize the data structure into a byte array (ECUReportBytes_t).
// etl::bit_stream_reader is used to deserialize the byte array back into the data structure.

// The maximum size tha any ECU report will ever have. This is
// provided so that applications receiving an ECUReport can preallocate a buffer
// to hold the report without having to know the exact size of the report.
#define ECU_REPORT_MAX_SIZE_BYTES 256 
typedef etl::array<uint8_t, ECU_REPORT_MAX_SIZE_BYTES> ECUReportBytes_t;

static_assert(ECU_DATA_REPORT_SIZE_BYTES <= ECU_REPORT_MAX_SIZE_BYTES, "ECU_DATA_REPORT_SIZE_BYTES exceeds ECU_REPORT_MAX_SIZE_BYTES");

// Initialize all fields in an ECUReport_t.
void ecu_report_init(ECUReport_t& report, uint8_t ecu_id);
// Add ECU health to an ECUReport_t.
void add_ecu_health(float v5, float v12, float v56, float board_t, float switch_mA, float cpu_temp, ECUReport_t& report);
// Add statuses to an ECUReport_t.
void add_status(bool heat_on, float temp_setpoint, bool rs41_regen_active, bool rs41_en, bool tsen_power, ECUReport_t& report);
// Add GPS data to an ECUReport_t.
void add_gps(bool valid, double lat, double lon, double alt, uint sats, uint32_t date, uint32_t time, uint age_secs, ECUReport_t& report);
// Add RS41 data to an ECUReport_t.
void add_rs41(bool valid, bool rs41_regen_active, float airt, float hum, float hst, float pres, float hdg, uint8_t status, bool pcb_h, ECUReport_t& report);
// Add TSEN data to an ECUReport_t.
void add_tsen(uint16_t airt, uint32_t prest, uint32_t pres, ECUReport_t& report);
// Size of the serialized ECUReport_t in bytes, based on the message type.
size_t ecu_report_serialized_size(ECUReport_t& report);
// Serialize an ECUReport_t.
ECUReportBytes_t ecu_report_serialize(ECUReport_t& report);
// Deserialize the revision, message type and id from ECUReportBytes_t.
// Returns a pair: <rev, msg_type>
std::array<uint8_t, 3> ecu_report_deserialize_rev_msg_type_id(const ECUReportBytes_t& data);
// Deserialize an ECUReportBytes_t.
ECUReport_t ecu_report_deserialize(ECUReportBytes_t& data);
// Print an ECUReport_t.
// If print_bin is true, the binary representation of the report will be included in the print.
void ecu_report_print(ECUReport_t& ecu_report, bool print_bin=false);
// Print ECU_MSG_TYPE_STRING raw data as text.
void ecu_report_print_raw(ECUReport_t& ecu_report_raw);
// Print a uint32_t in binary, with zero fill.
// n is the number to print.
// w is the number of bits to print.
void ecu_bin_print(uint32_t n, uint8_t w=8);

#endif //_ECU_REPORT_H_
