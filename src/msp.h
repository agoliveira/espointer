#pragma once
#include <Arduino.h>

// Minimal MSPv2 parser for complete frames as delivered in one ESPNOW payload.
// Wire format (ExpressLRS Backpack lib/MSP):
//   '$' 'X' ('<'|'>') flags(1) function(2 LE) payloadSize(2 LE) payload crc8_dvb_s2
// CRC covers flags..payload.

#define MSP_ELRS_BACKPACK_CRSF_TLM 0x11

struct MspFrame {
    uint8_t  type;        // '<' command or '>' response
    uint8_t  flags;
    uint16_t function;
    uint16_t payloadSize;
    const uint8_t *payload; // points into the source buffer
};

uint8_t crc8_dvb_s2(uint8_t crc, uint8_t data);

// Returns true and fills out if buf contains a valid MSPv2 frame.
bool mspParse(const uint8_t *buf, size_t len, MspFrame *out);
