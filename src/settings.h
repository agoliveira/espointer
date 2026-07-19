#pragma once
#include <Arduino.h>

struct Settings {
    uint16_t panMinUs;
    uint16_t panMaxUs;
    bool     panInv;
    uint16_t tiltMinUs;
    uint16_t tiltMaxUs;
    bool     tiltInv;
    int16_t  mountBearing;  // compass bearing the pan center points at, deg 0-359
    uint16_t slewDegS;      // max servo slew, deg/s
    uint16_t fsTimeoutS;    // seconds without fix before failsafe hold
    uint8_t  homeSats;      // min sats for home capture
    uint8_t  homeFixes;     // consecutive fixes required
    uint16_t homeRadiusM;   // max spread between consecutive fixes
};

extern Settings settings;

void settingsLoad();   // NVS -> settings (defaults if absent)
void settingsSave();   // settings -> NVS
void settingsPrint();
// Returns true if key existed and value applied
bool settingsSet(const char *key, const char *value);
