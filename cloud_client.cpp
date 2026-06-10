#include "cloud_client.h"
#include "pump.h"
#include "valve.h"
#include "dht_sensor.h"
#include "soil_sensor.h"
#include "water_tank.h"
#include <HTTPClient.h>
#include <Arduino.h>

static bool cloudConnected = false;

// ──────────────────────────────────────────────────────
// Connect to home WiFi (internet) on a second network
// The ESP32 AP (for local dashboard) stays active too
// ──────────────────────────────────────────────────────
void cloud_init() {
    WiFi.begin(HOME_SSID, HOME_PASS);
    Serial.print("Connecting to home WiFi");
    int tries = 0;
    while (WiFi.status() != WL_CONNECTED && tries < 20) {
        delay(500);
        Serial.print(".");
        tries++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        cloudConnected = true;
        Serial.println("\nCloud connected: " + WiFi.localIP().toString());
    } else {
        Serial.println("\nCloud connect failed — running local only");
    }
}

// ──────────────────────────────────────────────────────
// Build JSON from current sensor state and POST to Render
// Render replies with any pending commands e.g. {"pump":"ON"}
// ──────────────────────────────────────────────────────
void cloud_update() {
    static unsigned long lastSend = 0;
    if (millis() - lastSend < 2000) return;   // push every 2 s
    lastSend = millis();

    if (WiFi.status() != WL_CONNECTED) return;

    // Build JSON payload
    String json = "{";
    json += "\"pump\":\""   + String(pump_get_state() ? "ON" : "OFF") + "\",";
    json += "\"valve\":\""  + valve_get_state()                       + "\",";
    json += "\"temp\":"     + String(dht_get_temperature(), 1)        + ",";
    json += "\"hum\":"      + String(dht_get_humidity(),    1)        + ",";
    json += "\"soil1\":"    + String(soil_get_percent_1(),  1)        + ",";
    json += "\"soil2\":"    + String(soil_get_percent_2(),  1)        + ",";
    json += "\"fault1\":"   + String(soil_is_fault_1() ? "true" : "false") + ",";
    json += "\"fault2\":"   + String(soil_is_fault_2() ? "true" : "false") + ",";
    json += "\"percent\":"  + String(tank_get_percent(),    1)        + ",";
    json += "\"volume\":"   + String(tank_get_volume_ml(),  0);
    json += "}";

    HTTPClient http;
    http.begin(CLOUD_URL);
    http.addHeader("Content-Type", "application/json");
    int code = http.POST(json);

    if (code == 200) {
        String response = http.getString();
        Serial.println("Cloud response: " + response);

        // Parse commands from response
        // Simple manual parse — no JSON library needed
        if (response.indexOf("\"pump\":\"ON\"") >= 0)    pump_on();
        if (response.indexOf("\"pump\":\"OFF\"") >= 0)   pump_off();
        if (response.indexOf("\"valve\":\"OPEN\"") >= 0) valve_open();
        if (response.indexOf("\"valve\":\"CLOSE\"") >= 0)valve_close();
        if (response.indexOf("\"valve\":\"STOP\"") >= 0) valve_stop();
    } else {
        Serial.println("Cloud POST failed: " + String(code));
    }

    http.end();
}
