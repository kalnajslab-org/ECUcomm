#include "ECUReport.h"
#include "etl/bit_stream.h"
#include "etl/vector.h"

void ecu_bin_print(uint32_t n, uint8_t w)
{   
    for (int i = w-1; i >= 0; i--)
    {
        SerialUSB.print((n & (1 << i)) ? "1" : "0");
    }
    SerialUSB.print(" ");
}

void ecu_report_init(ECUReport_t& ecu_report)
{
    // *** Modify this function whenever the ECUReport_t struct is modified ***
    ecu_report.rev = ECU_REPORT_REV;
    ecu_report.msg_type = ECU_REPORT_MSG_TYPE_DATA;
    ecu_report.heat_on = 0;
    ecu_report.rs41_en = 0;
    ecu_report.tsen_power = 0;
    ecu_report.v5 = 0;
    ecu_report.v12 = 0;
    ecu_report.v56 = 0;
    ecu_report.board_t = 0;
    ecu_report.temp_setpoint = 0;
    ecu_report.switch_mA = 0;
    ecu_report.gps_valid = 0;
    ecu_report.gps_lat = 0;
    ecu_report.gps_lon = 0;
    ecu_report.gps_alt = 0;
    ecu_report.gps_sats = 0;
    ecu_report.gps_date = 0;
    ecu_report.gps_time = 0;
    ecu_report.gps_age_secs = 255;
    ecu_report.board_t = 0;
    ecu_report.rs41_valid = 0;
    ecu_report.rs41_regen = 0;
    ecu_report.rs41_airt = 0;
    ecu_report.rs41_hum = 0;
    ecu_report.rs41_hst = 0;
    ecu_report.rs41_pres = 0;
    ecu_report.rs41_pcb_h = 0;
    ecu_report.tsen_airt = 0;
    ecu_report.tsen_ptemp = 0;
    ecu_report.tsen_pres = 0;
    ecu_report.cpu_temp = 0;
}

void add_status(bool heat_on, uint8_t temp_setpoint, bool rs41_regen_active, bool rs41_en, bool tsen_power, ECUReport_t& report) {
    report.temp_setpoint = temp_setpoint+100;
    report.heat_on = heat_on;
    report.rs41_en = rs41_en;
    report.tsen_power = tsen_power;
}

void add_ecu_health(float v5, float v12, float v56, float board_t, float switch_mA, float cpu_temp, ECUReport_t& report) {
    report.v5 = v5 * 100;
    report.v12 = v12 * 100;
    report.v56 = v56 * 100;
    report.board_t = (board_t + 100) * 10;
    report.switch_mA = (switch_mA <= 255) ? switch_mA : 255;
    report.cpu_temp = (cpu_temp + 100) * 10;
}

void add_gps(bool valid, double lat, double lon, double alt, uint sats, uint32_t date, uint32_t time, uint age_secs, ECUReport_t& report) {
    report.gps_valid = valid;
    report.gps_lat      = lat * 1.0e6;
    report.gps_lon      = lon * 1.0e6;
    report.gps_alt      = alt;
    report.gps_sats     = (sats > 31)       ? 31 : sats;
    report.gps_date     = date;
    report.gps_time     = time;
    report.gps_age_secs = (age_secs > 255) ? 255 : age_secs;
}

void add_rs41(bool valid, bool rs41_regen_active, float airt, float hum, float hst, float pres, bool pcb_h, ECUReport_t& report) {
    report.rs41_valid = valid;
    report.rs41_regen = rs41_regen_active;

    if (airt < -100.0) {
        report.rs41_airt = 0;
    } else if (airt > 63.83) {
        report.rs41_airt = 16383;
    } else {
        report.rs41_airt = (airt + 100) * 100;
    }

    if  (hum < 0.0) {
        report.rs41_hum = 0;
    } else if (hum > 102.3) {
        report.rs41_hum = 1023;
    } else {
        report.rs41_hum = hum * 10;
    }

    if (hst < -100.0) {
        report.rs41_hst = 0;
    } else if (hst > 125.0) {
        report.rs41_hst = 255;
    } else {
        report.rs41_hst = hst + 100;
    }

    if (pres < 0.0) {
        report.rs41_pres = 0;
    } else if (pres > 1310.71) {
        report.rs41_pres = 131071;
    } else {
        report.rs41_pres = pres * 100;
    }

    report.rs41_pcb_h = pcb_h;
}

void add_tsen(uint16_t airt, uint32_t prest, uint32_t pres, ECUReport_t& report) {
    if (airt <0x10000) {
        report.tsen_airt = airt;
    } else {
        report.tsen_airt = 0xFFF;  // Error    
    }

    if (prest < 0x1000000) {
        report.tsen_ptemp = prest;
    } else {
        report.tsen_ptemp = 0xFFFFFF;  // Error
    }

    if (pres < 0x1000000) {
        report.tsen_pres = pres;
    } else {
        report.tsen_pres = 0xFFFFFF;  // Error
    }
}

ECUReportBytes_t ecu_report_serialize(ECUReport_t& report) {

    // *** Modify this function whenever the ECUReport_t struct is modified ***

    ECUReportBytes_t data;
    etl::span<uint8_t> data_span(data.data(), data.size());
    etl::bit_stream_writer writer(data_span, etl::endian::big);

    writer.write_unchecked(report.rev,          4);
    writer.write_unchecked(report.msg_type,     4);
    writer.write_unchecked(report.heat_on,      1);
    writer.write_unchecked(report.rs41_en,      1);
    writer.write_unchecked(report.tsen_power,   1);
    writer.write_unchecked(report.v5,           9);
    writer.write_unchecked(report.v12,         11);
    writer.write_unchecked(report.v56,         13);
    writer.write_unchecked(report.board_t,     11);
    writer.write_unchecked(report.temp_setpoint,8);
    writer.write_unchecked(report.switch_mA,    8);
    writer.write_unchecked(report.gps_valid,    1);
    writer.write_unchecked(report.gps_lat,     32);
    writer.write_unchecked(report.gps_lon,     32);
    writer.write_unchecked(report.gps_alt,     16);
    writer.write_unchecked(report.gps_sats,     5);
    writer.write_unchecked(report.gps_date,    19);
    writer.write_unchecked(report.gps_time,    25);
    writer.write_unchecked(report.gps_age_secs, 8);
    writer.write_unchecked(report.rs41_valid,   1);
    writer.write_unchecked(report.rs41_regen,   1);
    writer.write_unchecked(report.rs41_airt,   14);
    writer.write_unchecked(report.rs41_hum,    10);
    writer.write_unchecked(report.rs41_hst,     8);
    writer.write_unchecked(report.rs41_pres,   17);
    writer.write_unchecked(report.rs41_pcb_h,   1);
    writer.write_unchecked(report.tsen_airt,   12);
    writer.write_unchecked(report.tsen_ptemp,  24);
    writer.write_unchecked(report.tsen_pres,   24);
    writer.write_unchecked(report.cpu_temp,    11);

    return data;
}
ECUReport_t ecu_report_deserialize(ECUReportBytes_t& data) {
    
    // *** Modify this function whenever the ECUReport_t struct is modified ***

    etl::span<uint8_t> data_span(data.data(), data.size());
    etl::bit_stream_reader reader(data_span, etl::endian::big);

    ECUReport_t report;
    report.rev = reader.read_unchecked<uint8_t>                 (4);
    report.msg_type = reader.read_unchecked<ECU_REPORT_MSG_TYPE>(4);

    switch (report.msg_type) {
        case ECU_REPORT_MSG_TYPE_DATA:
            // OK
            report.heat_on = reader.read_unchecked<uint8_t>      (1); // Heater on (bool)
            report.rs41_en = reader.read_unchecked<uint8_t>      (1); // RS41 enabled (bool)
            report.tsen_power = reader.read_unchecked<uint8_t>   (1); // TSEN power (bool)
            report.v5 = reader.read_unchecked<uint16_t>          (9); // V5*100  (0-511  : 0.00V to 5.11V) 
            report.v12 = reader.read_unchecked<uint16_t>        (11); // V12*100 (0-2047 : 0.00V to 20.47V)
            report.v56 = reader.read_unchecked<uint16_t>        (13); // V56*100 (0-8191 : 0.00V to 81.91V)
            report.board_t = reader.read_unchecked<uint16_t>    (11); // (Board temperature+100)*10 (0-2047 : -100.0C to 104.7C)
            report.temp_setpoint = reader.read_unchecked<uint8_t>(8); // Setpoint temperature+100: (0-255 : -100C to 155C)
            report.switch_mA = reader.read_unchecked<uint8_t>    (8); // Switch current (mA): (0ma - 255mA)
            report.gps_valid = reader.read_unchecked<uint8_t>    (1); // GPS Valid (bool)
            report.gps_lat = reader.read_unchecked<int32_t>     (32); // GPS Latitude*1e6 (degrees*1e6)
            report.gps_lon = reader.read_unchecked<int32_t>     (32); // GPS Longitude*1e6 (degrees*1e6)
            report.gps_alt = reader.read_unchecked<uint16_t>    (16); // GPS Altitude (meters)
            report.gps_sats = reader.read_unchecked<uint8_t>     (5); // Number of satellites n (0 to 31)
            report.gps_date = reader.read_unchecked<uint32_t>   (19); // GPS Date (DDMMYY - Year is 20YY)
            report.gps_time = reader.read_unchecked<uint32_t>   (25); // GPS Time (HHMMSSSS - Seconds are in 100ths)
            report.gps_age_secs = reader.read_unchecked<uint8_t> (8); // Age of GPS data in seconds (0 to 255) 255 = greater than 254
            report.rs41_valid = reader.read_unchecked<uint8_t>   (1); // RS41 data valid (bool)
            report.rs41_regen = reader.read_unchecked<uint8_t>   (1); // RS41 regeneration active (bool)
            report.rs41_airt = reader.read_unchecked<uint16_t>  (14); // (RS41 Air Temperature+100)*100 (0-16383 : -100.00C to 63.83C)
            report.rs41_hum = reader.read_unchecked<uint16_t>   (10); // RS41 Humidity*10 (0-1023 : 0.0% to 102.3%)
            report.rs41_hst = reader.read_unchecked<uint8_t>     (8); // RS41 Humidity Sensor Temperature+100 (0-255 : -100C to 125C)
            report.rs41_pres = reader.read_unchecked<uint32_t>  (17); // RS41 Pressure*100 (0-131071 : 0.0hPa to 1310.71hPa) (should we do log10?)
            report.rs41_pcb_h = reader.read_unchecked<uint8_t>   (1); // RS41 PCB Heater On (bool)
            report.tsen_airt = reader.read_unchecked<uint16_t>  (12); // Raw
            report.tsen_ptemp = reader.read_unchecked<uint32_t> (24); // Raw
            report.tsen_pres = reader.read_unchecked<uint32_t>  (24); // Raw
            report.cpu_temp = reader.read_unchecked<uint16_t>   (11); // (CPU temperature+100)*10 (0-2047 : -100.0C to 104.8C)
            break;
        case ECU_REPORT_MSG_TYPE_STRING:
            // Not implemented yet
            break;
        default:
            // Unknown message type
            break;
    }

    return report;
}

void ecu_report_print(ECUReport_t& ecu_report, bool print_bin)
{

    // *** Modify this function whenever the ECUReport_t struct is modified ***

    SerialUSB.println("ECU Report:");
    SerialUSB.print("rev: "); if (print_bin) ecu_bin_print(ecu_report.rev,                    4); SerialUSB.print(String(ecu_report.rev)); SerialUSB.println();
    SerialUSB.print("msg_type: "); if (print_bin) ecu_bin_print(ecu_report.msg_type,          4); SerialUSB.print(String(ecu_report.msg_type)); SerialUSB.println();

    switch (ecu_report.msg_type) {
        case ECU_REPORT_MSG_TYPE_DATA:
            SerialUSB.print("heat_on: "); if (print_bin) ecu_bin_print(ecu_report.heat_on,            1); SerialUSB.print(ecu_report.heat_on?"True":"False"); SerialUSB.println();
            SerialUSB.print("rs41_en: "); if (print_bin) ecu_bin_print(ecu_report.rs41_en,            1); SerialUSB.print(ecu_report.rs41_en?"True":"False"); SerialUSB.println();
            SerialUSB.print("tsen_power: "); if (print_bin) ecu_bin_print(ecu_report.tsen_power,      1); SerialUSB.print(ecu_report.tsen_power?"True":"False"); SerialUSB.println();
            SerialUSB.print("v5: "); if (print_bin) ecu_bin_print(ecu_report.v5,                      9); SerialUSB.print(String(ecu_report.v5/100.0, 2) + "V"); SerialUSB.println();
            SerialUSB.print("v12: "); if (print_bin) ecu_bin_print(ecu_report.v12,                   11); SerialUSB.print(String(ecu_report.v12/100.0, 2) + "V"); SerialUSB.println();
            SerialUSB.print("v56: "); if (print_bin) ecu_bin_print(ecu_report.v56,                   13); SerialUSB.print(String(ecu_report.v56/100.0, 2) + "V"); SerialUSB.println();
            SerialUSB.print("board_t: "); if (print_bin) ecu_bin_print(ecu_report.board_t,           11); SerialUSB.print(String((ecu_report.board_t/10.0)-100.0, 1) + "degC"); SerialUSB.println();
            SerialUSB.print("temp_setpoint: "); if (print_bin) ecu_bin_print(ecu_report.temp_setpoint,8); SerialUSB.print(String((ecu_report.temp_setpoint)-100.0, 1) + "degC"); SerialUSB.println();
            SerialUSB.print("switch_mA: "); if (print_bin) ecu_bin_print(ecu_report.switch_mA,        8); SerialUSB.print(String(ecu_report.switch_mA) + "mA"); SerialUSB.println();
            SerialUSB.print("gps_valid: "); if (print_bin) ecu_bin_print(ecu_report.gps_valid,        1); SerialUSB.print(ecu_report.gps_valid?"True":"False"); SerialUSB.println();
            SerialUSB.print("gps_lat: "); if (print_bin) ecu_bin_print(ecu_report.gps_lat,           32); SerialUSB.print(String(ecu_report.gps_lat/1.0e6, 6) + "deg"); SerialUSB.println();
            SerialUSB.print("gps_lon: "); if (print_bin) ecu_bin_print(ecu_report.gps_lon,           32); SerialUSB.print(String(ecu_report.gps_lon/1.0e6, 6) + "deg"); SerialUSB.println();
            SerialUSB.print("gps_alt: "); if (print_bin) ecu_bin_print(ecu_report.gps_alt,           16); SerialUSB.print(String(ecu_report.gps_alt*1.0, 1) + "m"); SerialUSB.println();
            SerialUSB.print("gps_sats: "); if (print_bin) ecu_bin_print(ecu_report.gps_sats,          5); SerialUSB.print(String(ecu_report.gps_sats)); SerialUSB.println();
            SerialUSB.print("gps_date: "); if (print_bin) ecu_bin_print(ecu_report.gps_date,         19); SerialUSB.print(String(ecu_report.gps_date)); SerialUSB.println();
            SerialUSB.print("gps_time: "); if (print_bin) ecu_bin_print(ecu_report.gps_time,         25); SerialUSB.print(String(ecu_report.gps_time)); SerialUSB.println();
            SerialUSB.print("gps_age_secs: "); if (print_bin) ecu_bin_print(ecu_report.gps_age_secs,  8); SerialUSB.print(String(ecu_report.gps_age_secs) + "s"); SerialUSB.println();
            SerialUSB.print("rs41_valid: "); if (print_bin) ecu_bin_print(ecu_report.rs41_valid,      1); SerialUSB.print(ecu_report.rs41_valid?"True":"False"); SerialUSB.println();
            SerialUSB.print("rs41_regen: "); if (print_bin) ecu_bin_print(ecu_report.rs41_regen,      1); SerialUSB.print(ecu_report.rs41_regen?"True":"False"); SerialUSB.println();
            SerialUSB.print("rs41_airt: "); if (print_bin) ecu_bin_print(ecu_report.rs41_airt,       14); SerialUSB.print(String((ecu_report.rs41_airt/100.0)-100.0, 2) + "degC"); SerialUSB.println();
            SerialUSB.print("rs41_hum: "); if (print_bin) ecu_bin_print(ecu_report.rs41_hum,         10); SerialUSB.print(String(ecu_report.rs41_hum/10.0, 1) + "%"); SerialUSB.println();
            SerialUSB.print("rs41_hst: "); if (print_bin) ecu_bin_print(ecu_report.rs41_hst,          8); SerialUSB.print(String((ecu_report.rs41_hst/1.0)-100.0, 1) + "degC"); SerialUSB.println();
            SerialUSB.print("rs41_pres: "); if (print_bin) ecu_bin_print(ecu_report.rs41_pres,       17); SerialUSB.print(String(ecu_report.rs41_pres/100.0, 2) + "hPa"); SerialUSB.println();
            SerialUSB.print("rs41_pcb_h: "); if (print_bin) ecu_bin_print(ecu_report.rs41_pcb_h,      1); SerialUSB.print(ecu_report.rs41_pcb_h?"True":"False"); SerialUSB.println();
            SerialUSB.print("tsen_airt: "); if (print_bin) ecu_bin_print(ecu_report.tsen_airt,       12); SerialUSB.print(ecu_report.tsen_airt, HEX); SerialUSB.println();
            SerialUSB.print("tsen_ptemp: "); if (print_bin) ecu_bin_print(ecu_report.tsen_ptemp,     24); SerialUSB.print(ecu_report.tsen_ptemp, HEX); SerialUSB.println();
            SerialUSB.print("tsen_pres: "); if (print_bin) ecu_bin_print(ecu_report.tsen_pres,       24); SerialUSB.print(ecu_report.tsen_pres,HEX); SerialUSB.println();
            SerialUSB.print("cpu_temp: "); if (print_bin) ecu_bin_print(ecu_report.cpu_temp,         11); SerialUSB.print(String((ecu_report.cpu_temp/10.0)-100.0, 1) + "degC"); SerialUSB.println();
            break;
        case ECU_REPORT_MSG_TYPE_STRING:
            SerialUSB.println("  String Message:");
            break;
        default:
            SerialUSB.println("  Unknown Message Type:");
            break;
    }
}