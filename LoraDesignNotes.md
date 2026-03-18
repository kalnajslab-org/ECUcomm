# LoRa Design Notes — ECU Telemetry System

## Radio Hardware

- **Module:** RFM95W (SX1276 silicon)
- **Configuration:** SF9 / BW250 / CR 4/5 (assumed) / CRC on
- **Band:** 869.4–869.65 MHz (ETSI EN 300 220 sub-band g1)
- **Center frequency:** 869.525 MHz (exact center — at BW250 signal edges land precisely at 869.400 and 869.650 MHz)
- **Duty cycle limit:** 10% (sub-band g1 allows 10%, versus 1% on main 868 MHz channels)
- **Max power:** 500mW ERP on this sub-band

---

## Payload

- **Format:** ECU_REPORT_DATA — 348 bits = **44 bytes**
- **Hardware limit:** 255 bytes (SX127x single-byte FIFO length field)
- **Practical constraint:** Time on air and duty cycle, not a byte floor

---

## Time on Air

At SF9 / BW250 with a 44-byte payload:

| Parameter | Value |
|---|---|
| Symbol duration | 2.048 ms |
| Preamble | ~25.1 ms |
| Payload | ~118.8 ms |
| **Total ToA** | **~144 ms** |

Compare to SF12/BW125 (~2.5 s) — SF9/BW250 is approximately **17× faster on air** for the same payload.

---

## Duty Cycle Budget

- **Report rate:** 0.5 Hz (one packet every 2 seconds)
- **Actual duty cycle:** 144 ms / 2000 ms = **7.2%**
- **Headroom:** ~2.8% before hitting the 10% limit
- **Room to grow:** Payload could roughly double before breaching 10% at 0.5 Hz

---

## Regulatory Notes

- Sub-band 869.4–869.65 MHz is only 250 kHz wide — BW250 fills it exactly
- Crystal/TCXO frequency accuracy matters: LoRa at BW250 has ~±15 kHz capture range, leaving very little margin at band edges
- RFM95W modules with TCXO are significantly more stable than bare crystal designs
- US operation (FCC Part 15): no explicit duty cycle limit, 255-byte hardware limit applies directly

---

## SF/BW Matching Requirement

SF and BW **must match exactly** between transmitter and receiver. LoRa chirp spread spectrum requires the receiver correlator to be tuned to the same chirp duration and sweep width. These parameters are not transmitted in the packet — they must be configured identically on both ends.

### Register Verification

Read back modem config registers after init on both devices to confirm settings:

| Register | Address | Key Fields |
|---|---|---|
| `REG_MODEM_CONFIG_1` | `0x1D` | BW [7:4], CodingRate [3:1], ImplicitHeader [0] |
| `REG_MODEM_CONFIG_2` | `0x1E` | SF [7:4], RxPayloadCrcOn [2] |
| `REG_MODEM_CONFIG_3` | `0x26` | LowDataRateOptimize [3], AgcAutoOn [2] |

**BW encoding (bits [7:4] of 0x1D):**

| Value | Bandwidth |
|---|---|
| `0x7` | 125 kHz |
| `0x8` | 250 kHz |
| `0x9` | 500 kHz |

**SF encoding:** SF value written directly to bits [7:4] of `0x1E` (e.g. SF9 → `0x9`).

```cpp
void log_modem_config() {
    uint8_t cfg1 = readRegister(0x1D);
    uint8_t cfg2 = readRegister(0x1E);
    uint8_t cfg3 = readRegister(0x26);

    uint8_t bw  = (cfg1 >> 4) & 0x0F;
    uint8_t cr  = (cfg1 >> 1) & 0x07;
    uint8_t sf  = (cfg2 >> 4) & 0x0F;
    uint8_t crc = (cfg2 >> 2) & 0x01;
    uint8_t ldr = (cfg3 >> 3) & 0x01;

    Serial.printf("BW=%d CR=4/%d SF=%d CRC=%d LDR=%d\n",
                  bw, cr + 4, sf, crc, ldr);
}
```

### LowDataRateOptimize (LDR) Warning

Some libraries (e.g. Sandeep Mistry's Arduino `LoRa`) automatically set or clear LDR in `REG_MODEM_CONFIG_3` based on SF:

- SF11 and SF12 → LDR **enabled** automatically
- SF9 and below → LDR **cleared** automatically

A mismatch in LDR between TX and RX can cause framing issues even when SF and BW are correctly matched. Always verify `0x26` on both ends.

---

## Wire Format

The ETL `bit_stream_writer/reader` (big-endian) is used for portable, bit-exact packing. Direct `memcpy` of the C bitfield struct to the TX buffer is **not safe** — C bitfield layout is implementation-defined and non-portable.

See `ecu_telemetry.h` / `ecu_telemetry.cpp` for the full encode/decode implementation.
