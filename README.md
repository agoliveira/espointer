# espointer

ESP32-S3 antenna tracker for long-range analog FPV. Receives aircraft GPS
telemetry from an ExpressLRS TX backpack over ESPNOW and drives a pan/tilt
antenna mount. Spiritual successor to the abandoned VirtualPilot Sentinel AAT.

## Status

Phase 1: tracking. Home auto-capture, az/el math, 180/180 servos with
over-the-top flip for the rear hemisphere, slew limiting, failsafe hold,
serial CLI, NVS-persisted settings.

Servo pins: pan GPIO4, tilt GPIO5. Type help on the serial monitor for the CLI.

Pan modes:
- geared360 (default): geared pan mechanism (Sentinel AAT lite gears are 48T
  servo to 24T shaft, ratio 2.0), full 360 antenna coverage from a 180 servo,
  tilt 0-90. Seam sits directly behind mountBearing; point it away from the
  flying area. panGearRatio configurable for other gear sets.
- flip180: direct-drive 180 pan, rear hemisphere reached by tilting past
  vertical.

deadbandDeg suppresses movements smaller than the set angle to stop printed
gear backlash from hunting.

Roadmap:
- Phase 2: web configuration

## Hardware

- ESP32-S3-WROOM devkit (esp32-s3-devkitc-1)
- No wiring required for Phase 0

## Build

1. Copy `include/config.h.example` to `include/config.h`
2. Set `BIND_PHRASE` to your ELRS binding phrase
3. `pio run --target upload` then `pio device monitor`

## Radio setup

On the ELRS Lua script: Backpack, set Telemetry to ESP NOW. Without this the
TX backpack broadcasts nothing.

## Protocol notes

- Group address (UID) = first 6 bytes of MD5 of `-DMY_BINDING_PHRASE="<phrase>"`,
  multicast bit of byte 0 cleared. The tracker sets its STA MAC to this UID and
  the TX backpack unicasts to it.
- Payload is MSPv2 (`$X<`, flags, function u16 LE, size u16 LE, payload,
  crc8_dvb_s2). Function 0x11 (MSP_ELRS_BACKPACK_CRSF_TLM) wraps a raw CRSF
  frame. Frame type 0x02 is GPS: lat/lon int32 BE (deg * 1e7), speed u16 BE
  (km/h * 10), heading u16 BE (deg * 10), altitude u16 BE (m + 1000), satcnt u8.
