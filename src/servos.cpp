#include "servos.h"
#include "settings.h"

#define LEDC_CH_PAN   0
#define LEDC_CH_TILT  1
#define LEDC_FREQ_HZ  50
#define LEDC_RES_BITS 16
#define LEDC_PERIOD_US 20000

static float s_pan = 90, s_tilt = 0;       // current output angle
static float s_panTgt = 90, s_tiltTgt = 0; // target angle

static uint32_t usToDuty(uint32_t us)
{
    return (uint32_t)(((uint64_t)us << LEDC_RES_BITS) / LEDC_PERIOD_US);
}

static void writeServo(uint8_t channel, float deg, uint16_t minUs, uint16_t maxUs, bool inv)
{
    deg = constrain(deg, 0.0f, 180.0f);
    if (inv) {
        deg = 180.0f - deg;
    }
    uint32_t us = minUs + (uint32_t)((maxUs - minUs) * (deg / 180.0f));
    ledcWrite(channel, usToDuty(us));
}

void servosInit()
{
    ledcSetup(LEDC_CH_PAN, LEDC_FREQ_HZ, LEDC_RES_BITS);
    ledcSetup(LEDC_CH_TILT, LEDC_FREQ_HZ, LEDC_RES_BITS);
    ledcAttachPin(PIN_SERVO_PAN, LEDC_CH_PAN);
    ledcAttachPin(PIN_SERVO_TILT, LEDC_CH_TILT);
    writeServo(LEDC_CH_PAN, s_pan, settings.panMinUs, settings.panMaxUs, settings.panInv);
    writeServo(LEDC_CH_TILT, s_tilt, settings.tiltMinUs, settings.tiltMaxUs, settings.tiltInv);
}

void servosSetTarget(float panDeg, float tiltDeg)
{
    s_panTgt = constrain(panDeg, 0.0f, 180.0f);
    s_tiltTgt = constrain(tiltDeg, 0.0f, 180.0f);
}

static float slewToward(float current, float target, float maxStep)
{
    float d = target - current;
    if (d > maxStep) {
        d = maxStep;
    } else if (d < -maxStep) {
        d = -maxStep;
    }
    return current + d;
}

void servosTick(uint32_t dtMs)
{
    float maxStep = settings.slewDegS * (dtMs / 1000.0f);
    s_pan = slewToward(s_pan, s_panTgt, maxStep);
    s_tilt = slewToward(s_tilt, s_tiltTgt, maxStep);
    writeServo(LEDC_CH_PAN, s_pan, settings.panMinUs, settings.panMaxUs, settings.panInv);
    writeServo(LEDC_CH_TILT, s_tilt, settings.tiltMinUs, settings.tiltMaxUs, settings.tiltInv);
}

void servosGet(float *panDeg, float *tiltDeg, float *panTgt, float *tiltTgt)
{
    if (panDeg)  *panDeg = s_pan;
    if (tiltDeg) *tiltDeg = s_tilt;
    if (panTgt)  *panTgt = s_panTgt;
    if (tiltTgt) *tiltTgt = s_tiltTgt;
}
