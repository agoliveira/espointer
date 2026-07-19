#include "tracker.h"
#include "settings.h"
#include "servos.h"
#include <math.h>

static TrackerStatus s_st;
static uint32_t s_lastFixMs = 0;

// Home capture accumulator
static uint8_t s_accCount = 0;
static double  s_accLat = 0, s_accLon = 0;
static int64_t s_accAlt = 0;
static double  s_firstLat = 0, s_firstLon = 0;

static const double DEG_TO_M_LAT = 111320.0;

static double groundDistanceM(double lat1, double lon1, double lat2, double lon2)
{
    double latMid = (lat1 + lat2) * 0.5 * M_PI / 180.0;
    double dx = (lon2 - lon1) * DEG_TO_M_LAT * cos(latMid);
    double dy = (lat2 - lat1) * DEG_TO_M_LAT;
    return sqrt(dx * dx + dy * dy);
}

static double bearingDeg(double lat1, double lon1, double lat2, double lon2)
{
    double latMid = (lat1 + lat2) * 0.5 * M_PI / 180.0;
    double dx = (lon2 - lon1) * cos(latMid); // east
    double dy = (lat2 - lat1);               // north
    double b = atan2(dx, dy) * 180.0 / M_PI;
    if (b < 0) {
        b += 360.0;
    }
    return b;
}

static float wrap180(float deg)
{
    while (deg > 180.0f)  deg -= 360.0f;
    while (deg < -180.0f) deg += 360.0f;
    return deg;
}

// Map true azimuth/elevation to pan/tilt angles for 180/180 servos with
// over-the-top flip. Pan 0-180 sweeps mountBearing-90 .. mountBearing+90.
// Rear hemisphere: mirror pan, tilt goes past zenith (180 - el).
void trackerAzElToServos(float azDeg, float elDeg, int16_t mountBearing,
                         float *panDeg, float *tiltDeg)
{
    float rel = wrap180(azDeg - (float)mountBearing);
    if (rel >= -90.0f && rel <= 90.0f) {
        *panDeg = rel + 90.0f;
        *tiltDeg = constrain(elDeg, 0.0f, 90.0f);
    } else {
        *panDeg = wrap180(rel - 180.0f) + 90.0f;
        *tiltDeg = 180.0f - constrain(elDeg, 0.0f, 90.0f);
    }
}

void trackerInit()
{
    memset(&s_st, 0, sizeof(s_st));
    s_st.state = TRK_WAIT_HOME;
}

void trackerResetHome()
{
    s_st.homeSet = false;
    s_st.state = TRK_WAIT_HOME;
    s_accCount = 0;
    s_st.homeProgress = 0;
    Serial.println("home cleared, waiting for fixes");
}

static void tryCaptureHome(const GpsFix *fix)
{
    if (fix->satcnt < settings.homeSats) {
        s_accCount = 0;
        s_st.homeProgress = 0;
        return;
    }
    double lat = fix->latE7 / 1e7;
    double lon = fix->lonE7 / 1e7;

    if (s_accCount == 0) {
        s_firstLat = lat;
        s_firstLon = lon;
        s_accLat = 0;
        s_accLon = 0;
        s_accAlt = 0;
    } else if (groundDistanceM(s_firstLat, s_firstLon, lat, lon) > settings.homeRadiusM) {
        // moved too far, restart accumulation from this fix
        s_accCount = 0;
        s_firstLat = lat;
        s_firstLon = lon;
        s_accLat = 0;
        s_accLon = 0;
        s_accAlt = 0;
    }

    s_accLat += lat;
    s_accLon += lon;
    s_accAlt += fix->altitudeM;
    s_accCount++;
    s_st.homeProgress = s_accCount;

    if (s_accCount >= settings.homeFixes) {
        s_st.homeLat = s_accLat / s_accCount;
        s_st.homeLon = s_accLon / s_accCount;
        s_st.homeAlt = (int32_t)(s_accAlt / s_accCount);
        s_st.homeSet = true;
        s_st.state = TRK_TRACKING;
        Serial.printf("HOME set: %.7f %.7f alt=%dm (%u fixes)\n",
                      s_st.homeLat, s_st.homeLon, s_st.homeAlt, s_accCount);
    }
}

void trackerOnFix(const GpsFix *fix, uint32_t nowMs)
{
    s_lastFixMs = nowMs;

    if (!s_st.homeSet) {
        tryCaptureHome(fix);
        return;
    }

    if (s_st.state == TRK_FAILSAFE) {
        s_st.state = TRK_TRACKING;
        Serial.println("fix reacquired, tracking");
    }
    if (s_st.state != TRK_TRACKING) {
        return; // manual mode ignores fixes
    }

    double lat = fix->latE7 / 1e7;
    double lon = fix->lonE7 / 1e7;
    double dist = groundDistanceM(s_st.homeLat, s_st.homeLon, lat, lon);
    s_st.distanceM = (float)dist;

    // Too close: bearing is noise, hold azimuth, look up proportionally
    if (dist < 10.0) {
        s_st.elevation = 0;
        return;
    }

    s_st.azimuth = (float)bearingDeg(s_st.homeLat, s_st.homeLon, lat, lon);
    double dAlt = (double)(fix->altitudeM - s_st.homeAlt);
    s_st.elevation = (float)(atan2(dAlt, dist) * 180.0 / M_PI);
    if (s_st.elevation < 0) {
        s_st.elevation = 0;
    }

    float pan, tilt;
    trackerAzElToServos(s_st.azimuth, s_st.elevation, settings.mountBearing, &pan, &tilt);
    servosSetTarget(pan, tilt);
}

void trackerLoop(uint32_t nowMs)
{
    if (s_st.state == TRK_TRACKING && s_lastFixMs != 0 &&
        (nowMs - s_lastFixMs) > (uint32_t)settings.fsTimeoutS * 1000UL) {
        s_st.state = TRK_FAILSAFE; // hold last position
        Serial.println("FAILSAFE: no fix, holding position");
    }
}

void trackerManualAim(float az, float el)
{
    s_st.state = TRK_MANUAL;
    float pan, tilt;
    trackerAzElToServos(az, el, settings.mountBearing, &pan, &tilt);
    servosSetTarget(pan, tilt);
    Serial.printf("manual aim az=%.1f el=%.1f -> pan=%.1f tilt=%.1f\n", az, el, pan, tilt);
}

void trackerManualOff()
{
    s_st.state = s_st.homeSet ? TRK_TRACKING : TRK_WAIT_HOME;
    Serial.println("manual off");
}

const TrackerStatus *trackerStatus()
{
    return &s_st;
}
