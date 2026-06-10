#pragma once
// ======================================================
// cloud_client.h
// Pushes sensor data to Render and receives commands back
// ======================================================
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino.h>

// ── Change this to your actual Render URL ─────────────
#define CLOUD_URL  "https://esp32-pump-dashboard.onrender.com/update"

// ── Home WiFi credentials (to reach the internet) ─────
#define HOME_SSID  "YOUR_HOME_WIFI_NAME"
#define HOME_PASS  "YOUR_HOME_WIFI_PASSWORD"
