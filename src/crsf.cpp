#include "crsf.h"

// CRSF uses CRC8 poly 0xD5 over type + payload
static uint8_t crsfCrc8(const uint8_t *data, size_t len)
{
    uint8_t crc = 0;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int b = 0; b < 8; b++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0xD5;
            } else {
                crc = crc << 1;
            }
        }
    }
    return crc;
}

static uint16_t rdBE16(const uint8_t *p)
{
    return ((uint16_t)p[0] << 8) | p[1];
}

static int32_t rdBE32(const uint8_t *p)
{
    return (int32_t)(((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
                     ((uint32_t)p[2] << 8) | (uint32_t)p[3]);
}

bool crsfParseGps(const uint8_t *buf, size_t len, GpsFix *fix)
{
    // Header: sync(1) frame_size(1) type(1); frame_size counts type+payload+crc
    if (len < 4) {
        return false;
    }
    if (buf[0] != CRSF_SYNC_BYTE) {
        return false;
    }

    uint8_t frameSize = buf[1];
    if ((size_t)(2 + frameSize) > len || frameSize < 2) {
        return false;
    }
    if (buf[2] != CRSF_FRAMETYPE_GPS) {
        return false;
    }

    // GPS payload is 15 bytes: lat(4) lon(4) speed(2) heading(2) alt(2) sats(1)
    uint8_t payloadLen = frameSize - 2;
    if (payloadLen != 15) {
        return false;
    }

    // CRC over type + payload
    if (crsfCrc8(&buf[2], 1 + payloadLen) != buf[2 + frameSize - 1]) {
        return false;
    }

    const uint8_t *p = &buf[3];
    fix->latE7         = rdBE32(p + 0);
    fix->lonE7         = rdBE32(p + 4);
    fix->speedKmhX10   = rdBE16(p + 8);
    fix->headingDegX10 = rdBE16(p + 10);
    fix->altitudeM     = (int32_t)rdBE16(p + 12) - 1000;
    fix->satcnt        = p[14];
    return true;
}
