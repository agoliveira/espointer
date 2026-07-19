#include "settings.h"
#include <Preferences.h>

Settings settings;

static const Settings kDefaults = {
    .panMode      = PAN_GEARED360,
    .panGearRatio = 2.0f,
    .deadbandDeg  = 1.0f,
    .panMinUs     = 1000,
    .panMaxUs     = 2000,
    .panInv       = false,
    .tiltMinUs    = 1000,
    .tiltMaxUs    = 2000,
    .tiltInv      = false,
    .mountBearing = 0,
    .slewDegS     = 90,
    .fsTimeoutS   = 5,
    .homeSats     = 6,
    .homeFixes    = 5,
    .homeRadiusM  = 20,
};

static Preferences prefs;

void settingsLoad()
{
    settings = kDefaults;
    prefs.begin("espointer", true);
    if (prefs.isKey("panMinUs")) {
        settings.panMode      = prefs.getUChar("panMode", PAN_GEARED360);
        settings.panGearRatio = prefs.getFloat("panGearRatio", 2.0f);
        settings.deadbandDeg  = prefs.getFloat("deadbandDeg", 1.0f);
        settings.panMinUs     = prefs.getUShort("panMinUs");
        settings.panMaxUs     = prefs.getUShort("panMaxUs");
        settings.panInv       = prefs.getBool("panInv");
        settings.tiltMinUs    = prefs.getUShort("tiltMinUs");
        settings.tiltMaxUs    = prefs.getUShort("tiltMaxUs");
        settings.tiltInv      = prefs.getBool("tiltInv");
        settings.mountBearing = prefs.getShort("mountBearing");
        settings.slewDegS     = prefs.getUShort("slewDegS");
        settings.fsTimeoutS   = prefs.getUShort("fsTimeoutS");
        settings.homeSats     = prefs.getUChar("homeSats");
        settings.homeFixes    = prefs.getUChar("homeFixes");
        settings.homeRadiusM  = prefs.getUShort("homeRadiusM");
    }
    prefs.end();
}

void settingsSave()
{
    prefs.begin("espointer", false);
    prefs.putUChar("panMode", settings.panMode);
    prefs.putFloat("panGearRatio", settings.panGearRatio);
    prefs.putFloat("deadbandDeg", settings.deadbandDeg);
    prefs.putUShort("panMinUs", settings.panMinUs);
    prefs.putUShort("panMaxUs", settings.panMaxUs);
    prefs.putBool("panInv", settings.panInv);
    prefs.putUShort("tiltMinUs", settings.tiltMinUs);
    prefs.putUShort("tiltMaxUs", settings.tiltMaxUs);
    prefs.putBool("tiltInv", settings.tiltInv);
    prefs.putShort("mountBearing", settings.mountBearing);
    prefs.putUShort("slewDegS", settings.slewDegS);
    prefs.putUShort("fsTimeoutS", settings.fsTimeoutS);
    prefs.putUChar("homeSats", settings.homeSats);
    prefs.putUChar("homeFixes", settings.homeFixes);
    prefs.putUShort("homeRadiusM", settings.homeRadiusM);
    prefs.end();
}

void settingsPrint()
{
    Serial.printf("panMode=%s panGearRatio=%.2f deadbandDeg=%.1f\n",
                  settings.panMode == PAN_GEARED360 ? "geared360" : "flip180",
                  settings.panGearRatio, settings.deadbandDeg);
    Serial.printf("panMinUs=%u panMaxUs=%u panInv=%d\n",
                  settings.panMinUs, settings.panMaxUs, settings.panInv);
    Serial.printf("tiltMinUs=%u tiltMaxUs=%u tiltInv=%d\n",
                  settings.tiltMinUs, settings.tiltMaxUs, settings.tiltInv);
    Serial.printf("mountBearing=%d slewDegS=%u fsTimeoutS=%u\n",
                  settings.mountBearing, settings.slewDegS, settings.fsTimeoutS);
    Serial.printf("homeSats=%u homeFixes=%u homeRadiusM=%u\n",
                  settings.homeSats, settings.homeFixes, settings.homeRadiusM);
}

bool settingsSet(const char *key, const char *value)
{
    long v = atol(value);
    if (!strcmp(key, "panMode")) {
        if (!strcmp(value, "geared360") || !strcmp(value, "0")) { settings.panMode = PAN_GEARED360; return true; }
        if (!strcmp(value, "flip180") || !strcmp(value, "1"))   { settings.panMode = PAN_FLIP180; return true; }
        return false;
    }
    if (!strcmp(key, "panGearRatio")) {
        float f = atof(value);
        if (f < 0.5f || f > 10.0f) return false;
        settings.panGearRatio = f;
        return true;
    }
    if (!strcmp(key, "deadbandDeg")) {
        float f = atof(value);
        if (f < 0.0f || f > 20.0f) return false;
        settings.deadbandDeg = f;
        return true;
    }
    if (!strcmp(key, "panMinUs"))     { settings.panMinUs = v; return true; }
    if (!strcmp(key, "panMaxUs"))     { settings.panMaxUs = v; return true; }
    if (!strcmp(key, "panInv"))       { settings.panInv = v != 0; return true; }
    if (!strcmp(key, "tiltMinUs"))    { settings.tiltMinUs = v; return true; }
    if (!strcmp(key, "tiltMaxUs"))    { settings.tiltMaxUs = v; return true; }
    if (!strcmp(key, "tiltInv"))      { settings.tiltInv = v != 0; return true; }
    if (!strcmp(key, "mountBearing")) { settings.mountBearing = ((v % 360) + 360) % 360; return true; }
    if (!strcmp(key, "slewDegS"))     { settings.slewDegS = constrain(v, 5, 720); return true; }
    if (!strcmp(key, "fsTimeoutS"))   { settings.fsTimeoutS = constrain(v, 1, 600); return true; }
    if (!strcmp(key, "homeSats"))     { settings.homeSats = constrain(v, 4, 20); return true; }
    if (!strcmp(key, "homeFixes"))    { settings.homeFixes = constrain(v, 1, 20); return true; }
    if (!strcmp(key, "homeRadiusM"))  { settings.homeRadiusM = constrain(v, 5, 200); return true; }
    return false;
}
