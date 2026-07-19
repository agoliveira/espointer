#include "cli.h"
#include "settings.h"
#include "tracker.h"
#include "servos.h"

static char s_line[96];
static uint8_t s_len = 0;

static const char *stateName(TrackerState s)
{
    switch (s) {
        case TRK_WAIT_HOME: return "WAIT_HOME";
        case TRK_TRACKING:  return "TRACKING";
        case TRK_FAILSAFE:  return "FAILSAFE";
        case TRK_MANUAL:    return "MANUAL";
    }
    return "?";
}

static void printStatus()
{
    const TrackerStatus *st = trackerStatus();
    float pan, tilt, panT, tiltT;
    servosGet(&pan, &tilt, &panT, &tiltT);
    Serial.printf("state=%s home=%d", stateName(st->state), st->homeSet);
    if (st->homeSet) {
        Serial.printf(" (%.7f %.7f alt=%dm)", st->homeLat, st->homeLon, st->homeAlt);
    } else {
        Serial.printf(" progress=%u/%u", st->homeProgress, settings.homeFixes);
    }
    Serial.printf("\naz=%.1f el=%.1f dist=%.0fm pan=%.1f->%.1f tilt=%.1f->%.1f\n",
                  st->azimuth, st->elevation, st->distanceM, pan, panT, tilt, tiltT);
}

static void execute(char *line)
{
    char *cmd = strtok(line, " ");
    if (!cmd) {
        return;
    }

    if (!strcmp(cmd, "help")) {
        Serial.println("commands:");
        Serial.println("  show               print settings");
        Serial.println("  set <key> <value>  change a setting (RAM)");
        Serial.println("  save               persist settings to NVS");
        Serial.println("  home               clear home, recapture from next fixes");
        Serial.println("  north <deg>        set mountBearing (shortcut)");
        Serial.println("  aim <az> <el>      manual aim, true bearing + elevation");
        Serial.println("  track              leave manual mode");
        Serial.println("  status             tracker state");
    } else if (!strcmp(cmd, "show")) {
        settingsPrint();
    } else if (!strcmp(cmd, "set")) {
        char *key = strtok(NULL, " ");
        char *val = strtok(NULL, " ");
        if (key && val && settingsSet(key, val)) {
            Serial.printf("%s set (RAM only, use save to persist)\n", key);
        } else {
            Serial.println("usage: set <key> <value>, see show for keys");
        }
    } else if (!strcmp(cmd, "save")) {
        settingsSave();
        Serial.println("saved");
    } else if (!strcmp(cmd, "home")) {
        trackerResetHome();
    } else if (!strcmp(cmd, "north")) {
        char *val = strtok(NULL, " ");
        if (val && settingsSet("mountBearing", val)) {
            Serial.printf("mountBearing=%d (RAM only, use save to persist)\n",
                          settings.mountBearing);
        } else {
            Serial.println("usage: north <deg>");
        }
    } else if (!strcmp(cmd, "aim")) {
        char *az = strtok(NULL, " ");
        char *el = strtok(NULL, " ");
        if (az && el) {
            trackerManualAim(atof(az), atof(el));
        } else {
            Serial.println("usage: aim <az> <el>");
        }
    } else if (!strcmp(cmd, "track")) {
        trackerManualOff();
    } else if (!strcmp(cmd, "status")) {
        printStatus();
    } else {
        Serial.printf("unknown: %s (try help)\n", cmd);
    }
}

void cliPoll()
{
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\r') {
            continue;
        }
        if (c == '\n') {
            s_line[s_len] = 0;
            if (s_len > 0) {
                execute(s_line);
            }
            s_len = 0;
        } else if (s_len < sizeof(s_line) - 1) {
            s_line[s_len++] = c;
        }
    }
}
