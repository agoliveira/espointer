#include "msp.h"

uint8_t crc8_dvb_s2(uint8_t crc, uint8_t data)
{
    crc ^= data;
    for (int i = 0; i < 8; i++) {
        if (crc & 0x80) {
            crc = (crc << 1) ^ 0xD5;
        } else {
            crc = crc << 1;
        }
    }
    return crc;
}

bool mspParse(const uint8_t *buf, size_t len, MspFrame *out)
{
    // Minimum frame: 3 header + 5 (flags+func+size) + 0 payload + 1 crc
    if (len < 9) {
        return false;
    }
    if (buf[0] != '$' || buf[1] != 'X') {
        return false;
    }
    if (buf[2] != '<' && buf[2] != '>') {
        return false;
    }

    uint8_t  flags       = buf[3];
    uint16_t function    = (uint16_t)buf[4] | ((uint16_t)buf[5] << 8);
    uint16_t payloadSize = (uint16_t)buf[6] | ((uint16_t)buf[7] << 8);

    if ((size_t)(8 + payloadSize + 1) > len) {
        return false;
    }

    uint8_t crc = 0;
    for (size_t i = 3; i < (size_t)(8 + payloadSize); i++) {
        crc = crc8_dvb_s2(crc, buf[i]);
    }
    if (crc != buf[8 + payloadSize]) {
        return false;
    }

    out->type        = buf[2];
    out->flags       = flags;
    out->function    = function;
    out->payloadSize = payloadSize;
    out->payload     = &buf[8];
    return true;
}
