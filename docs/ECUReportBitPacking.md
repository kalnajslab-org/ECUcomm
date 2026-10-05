# ECU Report Bit Packing

This document specifies the on-the-wire bit layout of the ECU report as produced by
`ecu_report_serialize()` and consumed by `ecu_report_deserialize()` in
[src/ECUReport.cpp](../src/ECUReport.cpp).

**Report revision documented here: `ECU_REPORT_REV` = 8.**

## Ground rules

- `ECUReport_t` in [src/ECUReport.h](../src/ECUReport.h) defines the *fields and their widths only*.
  An in-memory instance of that struct is **not** bit-packed — C bitfield allocation is
  compiler-dependent. The authoritative packing is the sequence of
  `writer.write_unchecked(field, width)` calls in `ecu_report_serialize()`.
- Packing is done with `etl::bit_stream_writer` / `etl::bit_stream_reader` constructed with
  `etl::endian::big`.
- Fields are written in declaration order, **MSB-first**, with no padding or alignment between
  fields. A field may straddle byte boundaries.
- Bit 0 of the stream is the **most significant bit of byte 0**. In the tables below,
  "byte.bit" gives the containing byte index and the bit position within that byte counted
  from the MSB (so `2.3` = byte 2, 4th-most-significant bit).
- Signed fields (`gps_lat`, `gps_lon`) are written as full-width two's-complement `int32_t`.
- Any trailing bits in the final byte are undefined padding.

## Message framing

Every message starts with the same 16-bit header, which is what allows a receiver to decide
how to interpret the rest:

| Bit offset | Width | Field | Notes |
|---|---|---|---|
| 0 | 4 | `rev` | Report structure revision (`ECU_REPORT_REV`) |
| 4 | 4 | `msg_type` | `0` = `ECU_REPORT_DATA`, `1` = `ECU_REPORT_RAW` |
| 8 | 8 | `ecu_id` | ECU identifier |

Because these three fields land on clean byte boundaries, they can be extracted without a
bit-stream reader — this is exactly what `ecu_report_deserialize_rev_msg_type_id()` does:

```c
rev      = (data[0] >> 4) & 0x0F;
msg_type =  data[0]       & 0x0F;
ecu_id   =  data[1];
```

The remainder of the message is selected by `msg_type` (a union in `ECUReport_t`).

## `msg_type = 0` — `ECU_REPORT_DATA`

Total payload: **371 bits = 47 bytes** (`ECU_DATA_REPORT_SIZE_BITS` /
`ECU_DATA_REPORT_SIZE_BYTES`), with 5 bits of padding in byte 46.

| Bit offset | Width | Last bit | byte.bit | Field | Encoding / range |
|---|---|---|---|---|---|
| 0 | 4 | 3 | 0.0 | `rev` | Report revision |
| 4 | 4 | 7 | 0.4 | `msg_type` | 0 = DATA |
| 8 | 8 | 15 | 1.0 | `ecu_id` | ECU ID |
| 16 | 1 | 16 | 2.0 | `heat_on` | bool |
| 17 | 1 | 17 | 2.1 | `rs41_en` | bool |
| 18 | 1 | 18 | 2.2 | `tsen_power` | bool |
| 19 | 9 | 27 | 2.3 | `v5` | V×100, 0–511 → 0.00–5.11 V |
| 28 | 11 | 38 | 3.4 | `v12` | V×100, 0–2047 → 0.00–20.47 V |
| 39 | 13 | 51 | 4.7 | `v56` | V×100, 0–8191 → 0.00–81.91 V |
| 52 | 11 | 62 | 6.4 | `board_t` | (°C+100)×10, 0–2047 → −100.0–104.7 °C |
| 63 | 8 | 70 | 7.7 | `temp_setpoint` | °C+100, 0–255 → −100–155 °C |
| 71 | 8 | 78 | 8.7 | `switch_mA` | mA, 0–255 (clamped) |
| 79 | 1 | 79 | 9.7 | `gps_valid` | bool |
| 80 | 32 | 111 | 10.0 | `gps_lat` | signed, degrees×1e6 |
| 112 | 32 | 143 | 14.0 | `gps_lon` | signed, degrees×1e6 |
| 144 | 16 | 159 | 18.0 | `gps_alt` | meters, 0–65535 |
| 160 | 5 | 164 | 20.0 | `gps_sats` | count, 0–31 (clamped) |
| 165 | 19 | 183 | 20.5 | `gps_date` | DDMMYY (year = 20YY) |
| 184 | 25 | 208 | 23.0 | `gps_time` | HHMMSSSS (seconds in 100ths) |
| 209 | 8 | 216 | 26.1 | `gps_age_secs` | seconds, 0–255; 255 = >254 |
| 217 | 1 | 217 | 27.1 | `rs41_valid` | bool |
| 218 | 1 | 218 | 27.2 | `rs41_regen` | bool |
| 219 | 16 | 234 | 27.3 | `rs41_airt` | (°C+100)×436.9067, 0–65535 → −100–50 °C |
| 235 | 16 | 250 | 29.3 | `rs41_hum` | (%RH+20)×543.1333, 0–65535 → −20–100 %RH |
| 251 | 16 | 266 | 31.3 | `rs41_hst` | (°C+100)×436.9067, 0–65535 → −100–50 °C |
| 267 | 16 | 282 | 33.3 | `rs41_pres` | (ln(hPa)−3.4012)×18533.04, 0–65535 → ~30–1030 hPa |
| 283 | 8 | 290 | 35.3 | `rs41_hdg` | heading×255/360, 0–255 → 0–360° |
| 291 | 8 | 298 | 36.3 | `rs41_status` | flag bits, see `ECU_RS41_*` (below) |
| 299 | 1 | 299 | 37.3 | `rs41_pcb_h` | bool |
| 300 | 12 | 311 | 37.4 | `tsen_airt` | raw counts; `0xFFF` = error |
| 312 | 24 | 335 | 39.0 | `tsen_ptemp` | raw counts; `0xFFFFFF` = error |
| 336 | 24 | 359 | 42.0 | `tsen_pres` | raw counts; `0xFFFFFF` = error |
| 360 | 11 | 370 | 45.0 | `cpu_temp` | (°C+100)×10, 0–2047 → −100.0–104.7 °C |
| 371 | 5 | 375 | 46.3 | *(padding)* | undefined |

### Scaling round-trip

Encode/decode pairs, as implemented in the `add_*()` helpers and `ecu_report_print()`:

| Field | Encode | Decode |
|---|---|---|
| `v5`, `v12`, `v56` | `raw = volts * 100` | `volts = raw / 100.0` |
| `board_t`, `cpu_temp` | `raw = (degC + 100) * 10` | `degC = raw / 10.0 - 100.0` |
| `temp_setpoint` | `raw = degC + 100` | `degC = raw - 100` |
| `gps_lat`, `gps_lon` | `raw = deg * 1e6` | `deg = raw / 1e6` |
| `rs41_airt`, `rs41_hst` | `raw = (degC + 100) * 436.9067` | `degC = raw / 436.9067 - 100.0` |
| `rs41_hum` | `raw = (pct + 20) * 543.1333` | `pct = raw / 543.1333 - 20.0` |
| `rs41_pres` | `raw = (ln(hPa) - 3.4012) * 18533.04` | `hPa = exp(raw / 18533.04 + 3.4012)` |
| `rs41_hdg` | `raw = (deg / 360.0) * 255.0` | `deg = (raw / 255.0) * 360.0` |

`add_rs41()` clamps its inputs to the encodable range rather than wrapping; `add_tsen()`
substitutes an all-ones error value if a raw reading exceeds the field width.

`rs41_pres` is log-encoded to give roughly constant relative resolution (~0.005 %) across the
range; `0` also means "no valid pressure" (`pres <= 0`).

### `rs41_status` bits

| Bit | Constant | Meaning |
|---|---|---|
| 0 | `ECU_RS41_HIGH_INTERNAL_TEMP` | high internal temperature |
| 1 | `ECU_RS41_REGEN_TEMP_LOW` | regen temperature low |
| 2 | `ECU_RS41_PTU_FAILURE` | PTU failure |
| 3 | `ECU_RS41_FLASH_FAILURE` | flash failure |
| 4 | `ECU_RS41_LOW_INPUT_VOLTAGE` | low input voltage |
| 5 | `ECU_RS41_NOT_CALIBRATED` | not calibrated |
| 6 | `ECU_RS41_NO_PRESSURE_MODULE` | no pressure module |
| 7 | `ECU_RS41_DISCONNECTED_BOOM` | disconnected boom |

## `msg_type = 1` — `ECU_REPORT_RAW`

A generic byte-carrier message. `ECU_MAX_RAW_BYTES` (200) bytes are *always* transmitted;
only the first `n_bytes` are meaningful.

| Bit offset | Width | byte.bit | Field | Notes |
|---|---|---|---|---|
| 0 | 4 | 0.0 | `rev` | Report revision |
| 4 | 4 | 0.4 | `msg_type` | 1 = RAW |
| 8 | 8 | 1.0 | `ecu_id` | ECU ID |
| 16 | 8 | 2.0 | `n_bytes` | Valid byte count in `raw[]`, 0–200 |
| 24 | 8×200 | 3.0 | `raw[0..199]` | Payload bytes, in order |

Total: **203 bytes**, byte-aligned throughout (`2 + ECU_MAX_RAW_BYTES` as returned by
`ecu_report_serialized_size()`; the leading 2 covers the rev/msg_type/ecu_id header, and
`n_bytes` is counted with the raw array).

An unrecognized `msg_type` is treated as RAW by both `ecu_report_serialize()` and
`ecu_report_serialized_size()`.

## Buffer sizes

| Symbol | Value | Meaning |
|---|---|---|
| `ECU_DATA_REPORT_SIZE_BITS` | 371 | Sum of DATA field widths |
| `ECU_DATA_REPORT_SIZE_BYTES` | 47 | Above, rounded up |
| `ECU_MAX_RAW_BYTES` | 200 | Size of `raw[]` in a RAW message |
| `ECU_REPORT_MAX_SIZE_BYTES` | 256 | Size of `ECUReportBytes_t`; lets receivers preallocate |

`ecu_report_serialize()` always returns a full 256-byte `ECUReportBytes_t`; use
`ecu_report_serialized_size()` to find how many of those bytes to actually transmit.

## Changing the report

Six places must be kept in exact agreement — widths *and* declared types:

1. `ECUReport_t` in [src/ECUReport.h](../src/ECUReport.h)
2. `ECU_DATA_REPORT_SIZE_BITS` in the same header
3. `ecu_report_init()`
4. `ecu_report_serialize()`
5. `ecu_report_deserialize()`
6. `ecu_report_print()`

Bump `ECU_REPORT_REV` on any layout change, and update this document. A mismatch between
writer and reader produces silently misaligned fields for everything downstream of the change,
with no checksum to catch it.
