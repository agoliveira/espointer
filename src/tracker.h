#pragma once
#include <Arduino.h>
#include "crsf.h"

enum TrackerState : uint8_t {
    TRK_WAIT_HOME,   // collecting fixes for home capture
    TRK_TRACKING,    // home set, following fixes
    TRK_FAILSAFE,    // no fix within timeout, holding last position
    TRK_MANUAL,      // CLI aim override
};

struct TrackerStatus {
    TrackerState state;
    bool     homeSet;
    double   homeLat, homeLon;
    int32_t  homeAlt;
    float    azimuth;    // deg true, 0 = north
    float    elevation;  // deg above horizon
    float    distanceM;
    uint8_t  homeProgress; // fixes accumulated toward capture
};

void trackerInit();
void trackerOnFix(const GpsFix *fix, uint32_t nowMs);
// Call regularly; drives servo targets and failsafe transitions
void trackerLoop(uint32_t nowMs);
void trackerResetHome();
void trackerManualAim(float az, float el); // enter manual mode
void trackerManualOff();
const TrackerStatus *trackerStatus();

// Exposed for host-side testing
void trackerAzElToServos(float azDeg, float elDeg, int16_t mountBearing,
                         uint8_t panMode, float gearRatio,
                         float *panDeg, float *tiltDeg);
