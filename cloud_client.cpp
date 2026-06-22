#include "cloud_client.h"
#include "pump.h"
#include "valve.h"
#include "dht_sensor.h"
#include "soil_sensor.h"
#include "water_tank.h"

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
        Serial.println("\nCloud connected!");
    } else {
        Serial.println("\nCloud connect failed — running local only");
    }
}

void cloud_update() {
    static unsigned long lastSend = 0;
    if (millis() - lastSend < 2000) return;
    lastSend = millis();

    if (WiFi.status() != WL_CONNECTED) return;

    String json = "{";
    json += "\"pump\":\""  + String(pump_get_state() ? "ON" : "OFF") + "\",";
    json += "\"valve\":\"" + valve_get_state()                       + "\",";
    json += "\"temp\":"    + String(dht_get_temperature(), 1)        + ",";
    json += "\"hum\":"     + String(dht_get_humidity(),    1)        + ",";
    json += "\"soil1\":"   + String(soil_get_percent_1(),  1)        + ",";
    json += "\"soil2\":"   + String(soil_get_percent_2(),  1)        + ",";
    json += "\"fault1\":"  + String(soil_is_fault_1() ? "true" : "false") + ",";
    json += "\"fault2\":"  + String(soil_is_fault_2() ? "true" : "false") + ",";
    json += "\"percent\":" + String(tank_get_percent(),    1)        + ",";
    json += "\"volume\":"  + String(tank_get_volume_l(),  0);
    json += "}";

    HTTPClient http;
    http.begin(CLOUD_URL);
    http.addHeader("Content-Type", "application/json");
    int code = http.POST(json);

    if (code == 200) {
        String resp = http.getString();
        Serial.println("Cloud OK: " + resp);
        if (resp.indexOf("\"pump\":\"ON\"")     >= 0) pump_on();
        if (resp.indexOf("\"pump\":\"OFF\"")    >= 0) pump_off();
        if (resp.indexOf("\"valve\":\"OPEN\"")  >= 0) valve_open();
        if (resp.indexOf("\"valve\":\"CLOSE\"") >= 0) valve_close();
        if (resp.indexOf("\"valve\":\"STOP\"")  >= 0) valve_stop();
    } else {
        Serial.println("Cloud fail: " + String(code));
    }
    http.end();
}
