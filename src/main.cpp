// espointer Phase 0: receive ELRS TX backpack telemetry over ESPNOW,
// decode CRSF GPS frames, stream to USB serial.

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#include "config.h"
#include "uid.h"
#include "msp.h"
#include "crsf.h"

static uint8_t g_uid[6];

// Written from the ESPNOW callback (WiFi task), read from loop()
static volatile uint32_t g_pktTotal = 0;   // ESPNOW packets received
static volatile uint32_t g_pktTlm = 0;     // valid CRSF_TLM MSP frames
static volatile uint32_t g_pktGps = 0;     // valid GPS fixes
static portMUX_TYPE g_fixMux = portMUX_INITIALIZER_UNLOCKED;
static GpsFix g_fix;
static bool g_fixValid = false;
static uint32_t g_fixMillis = 0;

static void onEspnowRecv(const uint8_t *mac, const uint8_t *data, int len)
{
    g_pktTotal = g_pktTotal + 1;

    MspFrame msp;
    if (!mspParse(data, (size_t)len, &msp)) {
        return;
    }
    if (msp.function != MSP_ELRS_BACKPACK_CRSF_TLM) {
        return;
    }
    g_pktTlm = g_pktTlm + 1;

    GpsFix fix;
    if (!crsfParseGps(msp.payload, msp.payloadSize, &fix)) {
        return;
    }
    g_pktGps = g_pktGps + 1;

    portENTER_CRITICAL(&g_fixMux);
    g_fix = fix;
    g_fixValid = true;
    g_fixMillis = millis();
    portEXIT_CRITICAL(&g_fixMux);
}

void setup()
{
    Serial.begin(115200);
    delay(2000); // allow USB CDC to enumerate

    Serial.println("espointer phase 0");

    uidFromBindPhrase(BIND_PHRASE, g_uid);
    Serial.printf("UID from bind phrase: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  g_uid[0], g_uid[1], g_uid[2], g_uid[3], g_uid[4], g_uid[5]);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Become the address the TX backpack transmits to
    esp_err_t err = esp_wifi_set_mac(WIFI_IF_STA, g_uid);
    Serial.printf("esp_wifi_set_mac: %s\n", esp_err_to_name(err));

    err = esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);
    Serial.printf("esp_wifi_set_channel(%d): %s\n", ESPNOW_CHANNEL, esp_err_to_name(err));

    if (esp_now_init() != ESP_OK) {
        Serial.println("esp_now_init FAILED, halting");
        while (true) {
            delay(1000);
        }
    }
    esp_now_register_recv_cb(onEspnowRecv);

    Serial.println("Listening. Enable Backpack telemetry = ESP NOW in the ELRS Lua menu.");
}

void loop()
{
    static uint32_t lastStatus = 0;
    static uint32_t lastGpsCount = 0;

    uint32_t now = millis();

    // Print each new fix as it arrives
    if (g_pktGps != lastGpsCount) {
        lastGpsCount = g_pktGps;

        portENTER_CRITICAL(&g_fixMux);
        GpsFix fix = g_fix;
        portEXIT_CRITICAL(&g_fixMux);

        Serial.printf("GPS lat=%.7f lon=%.7f alt=%dm spd=%.1fkm/h hdg=%.1f sats=%u\n",
                      fix.latE7 / 1e7, fix.lonE7 / 1e7, fix.altitudeM,
                      fix.speedKmhX10 / 10.0f, fix.headingDegX10 / 10.0f,
                      fix.satcnt);
        lastStatus = now;
    }

#if STATUS_INTERVAL_MS > 0
    if (now - lastStatus >= STATUS_INTERVAL_MS) {
        lastStatus = now;
        uint32_t age = g_fixValid ? (now - g_fixMillis) : 0;
        Serial.printf("status: espnow=%lu tlm=%lu gps=%lu lastFixAge=%lums\n",
                      (unsigned long)g_pktTotal, (unsigned long)g_pktTlm,
                      (unsigned long)g_pktGps,
                      g_fixValid ? (unsigned long)age : 0UL);
    }
#endif

    delay(10);
}
