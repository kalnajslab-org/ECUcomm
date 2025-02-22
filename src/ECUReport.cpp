#include "ECUReport.h"

void bin_print(uint32_t n, uint8_t w)
{
    for (int i = w-1; i >= 0; i--)
    {
        SerialUSB.print((n & (1 << i)) ? "1" : "0");
    }
}

void ecu_report_init(ECUReport_t* ecu_report)
{
    ecu_report->rev = ECU_REPORT_REV;
    ecu_report->heat_on = 0;
    ecu_report->v5 = 0;
    ecu_report->v12 = 0;
    ecu_report->v56 = 0;
    ecu_report->gps_valid = 0;
    ecu_report->gps_lat = 0;
    ecu_report->gps_lon = 0;
    ecu_report->gps_alt = 0;
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