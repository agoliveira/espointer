#pragma once
#include <Arduino.h>

// CRSF frame as carried inside MSP_ELRS_BACKPACK_CRSF_TLM.
// Verified against ExpressLRS/Backpack lib/CrsfProtocol/crsf_protocol.h.

#define CRSF_SYNC_BYTE     0xC8
#define CRSF_FRAMETYPE_GPS 0x02

struct GpsFix {
    int32_t  latE7;      // degrees * 1e7
    int32_t  lonE7;      // degrees * 1e7
    uint16_t speedKmhX10;
    uint16_t headingDegX10;
    int32_t  altitudeM;  // meters, offset already removed
    uint8_t  satcnt;
};

// buf points at a CRSF frame (sync byte first), len is bytes available.
// Returns true and fills fix if it is a CRC-valid GPS frame.
bool crsfParseGps(const uint8_t *buf, size_t len, GpsFix *fix);
