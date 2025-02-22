#include "ECUReport.h"
#include "etl/bit_stream.h"
#include "etl/vector.h"

void bin_print(uint32_t n, uint8_t w)
{
    for (int i = w-1; i >= 0; i--)
    {
        SerialUSB.print((n & (1 << i)) ? "1" : "0");
    }
}

void ecu_report_init(ECUReport_t& ecu_report)
{
    ecu_report.rev = ECU_REPORT_REV;
    ecu_report.heat_on = 0;
    ecu_report.v5 = 0;
    ecu_report.v12 = 0;
    ecu_report.v56 = 0;
    ecu_report.gps_valid = 0;
    ecu_report.gps_lat = 0;
    ecu_report.gps_lon = 0;
    ecu_report.gps_alt = 0;
}

void add_ecu_health(float v5, float v12, float v56, float board_t, ECUReport_t& report) {
    report.v5 = (uint16_t)(v5 * 100);
    report.v12 = (uint16_t)(v12 * 100);
    report.v56 = (uint16_t)(v56 * 100);
    report.board_t = (uint8_t)((board_t + 100) * 10);
}

void add_gps(bool valid, double lat, double lon, double alt, ECUReport_t& report) {
    report.gps_valid = valid;
    report.gps_lat = (int32_t)(lat * 1.0e6);
    report.gps_lon = (int32_t)(lon * 1.0e6);
    report.gps_alt = (uint16_t)alt;
}

etl::array<uint8_t, ECU_REPORT_SIZE_BYTES> ecu_report_serialize(ECUReport_t& report) {

    etl::array<uint8_t, ECU_REPORT_SIZE_BYTES> data;
    etl::span<uint8_t> data_span(data.data(), data.size());
    etl::bit_stream_writer writer(data_span, etl::endian::big);

    writer.write_unchecked(report.rev, 4);
    writer.write_unchecked(report.heat_on, 1);  // Heater on (bool)
    writer.write_unchecked(report.v5, 9);  // V5*100  (0-511  : 0.00V to 5.11V) 
    writer.write_unchecked(report.v12, 11);  // V12*100 (0-2047 : 0.00V to 20.47V)
    writer.write_unchecked(report.v56, 13);  // V56*100 (0-8191 : 0.00V to 81.91V)
    writer.write_unchecked(report.board_t, 11);  // (Board temperature+100)*10 (0-2047 : -100.0C to 104.7C)
    writer.write_unchecked(report.gps_valid, 1);  // GPS Valid (bool)
    writer.write_unchecked(report.gps_lat, 32);  // GPS Latitude*1e6 (degrees*1e6)
    writer.write_unchecked(report.gps_lon, 32);  // GPS Longitude*1e6 (degrees*1e6)
    writer.write_unchecked(report.gps_alt, 16);  // GPS Altitude (meters)

    return data;
}

void ecu_report_print(ECUReport_t* ecu_report)
{
    SerialUSB.println("\nECU Report:");
    SerialUSB.print("rev: "); bin_print(ecu_report->rev, 4); SerialUSB.println();
    SerialUSB.print("heat_on: "); bin_print(ecu_report->heat_on, 1); SerialUSB.println();
    SerialUSB.print("v5: "); bin_print(ecu_report->v5, 9); SerialUSB.print(" (" + String(ecu_report->v5/100.0, 2) + "V)"); SerialUSB.println();
    SerialUSB.print("v12: "); bin_print(ecu_report->v12, 11); SerialUSB.print(" (" + String(ecu_report->v12/100.0, 2) + "V)"); SerialUSB.println();
    SerialUSB.print("v56: "); bin_print(ecu_report->v56, 13); SerialUSB.print(" (" + String(ecu_report->v56/100.0, 2) + "V)"); SerialUSB.println();
    SerialUSB.print("gps_valid: "); bin_print(ecu_report->gps_valid, 1); SerialUSB.println();
    SerialUSB.print("gps_lat: "); bin_print(ecu_report->gps_lat, 32); SerialUSB.print(" (" + String(ecu_report->gps_lat/1.0e6, 6) + " deg)"); SerialUSB.println();
    SerialUSB.print("gps_lon: "); bin_print(ecu_report->gps_lon, 32); SerialUSB.print(" (" + String(ecu_report->gps_lon/1.0e6, 6) + " deg)"); SerialUSB.println();
    SerialUSB.print("gps_alt: "); bin_print(ecu_report->gps_alt, 16); SerialUSB.print(" (" + String(ecu_report->gps_alt, 0) + " m)"); SerialUSB.println();
}