#pragma once
#include <Arduino.h>

// LEDC-driven pan/tilt servos with slew limiting.
// Angles: pan 0-180 (0 = left end of travel), tilt 0-180 (0 = horizon front,
// 90 = zenith, 180 = horizon rear for over-the-top flip).

#define PIN_SERVO_PAN  4
#define PIN_SERVO_TILT 5

void servosInit();
void servosSetTarget(float panDeg, float tiltDeg);
// Call at ~50Hz; moves outputs toward target respecting settings.slewDegS
void servosTick(uint32_t dtMs);
void servosGet(float *panDeg, float *tiltDeg, float *panTgt, float *tiltTgt);
