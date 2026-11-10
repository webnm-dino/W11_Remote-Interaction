/*
  DM3014 Interactive Devices, Ashley Hi 2026
  Week 11 – Remote Interaction
  You don't need to edit this file. It does three jobs:
  1. Joins Wi-Fi, and rejoins if the connection drops.
  2. Reads the ultrasonic sensor: three quick pings, keeps the middle
     one, so a single bad echo doesn't make the page jump.
  3. Sends readings to Firebase's Realtime Database over HTTPS:
       PATCH <DATABASE_URL><DATA_PATH>.json
       { "distance": 42.5, "near": true, "updatedAt": <server time> }
     It sends twice a second (the page uses that as a heartbeat) and
     immediately whenever "near" changes.
*/

#pragma once
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// Defined in your .ino file.
extern const char* WIFI_NAME;
extern const char* WIFI_PASSWORD;
extern const char* DATABASE_URL;
extern const char* DATA_PATH;
extern const int TRIG_PIN;
extern const int ECHO_PIN;

const unsigned long SEND_EVERY_MS = 500;

static WiFiClientSecure fbClient;
static HTTPClient fbHttp;
static unsigned long fbLastSend = 0;
static int fbLastNear = -1;
static unsigned long wifiLastTry = 0;

// ── Wi-Fi ────────────────────────────────────────────────────────

static void joinWifi() {
  Serial.printf("[presence] joining Wi-Fi '%s'", WIFI_NAME);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);   // SuperMini boards are steadier at lower power
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(400);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("[presence] Wi-Fi connected, board address %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("[presence] couldn't join Wi-Fi. Check the name and password, and that it's 2.4 GHz.");
  }
  wifiLastTry = millis();
}

// ── sensor ───────────────────────────────────────────────────────

static float pingOnceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(4);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long us = pulseIn(ECHO_PIN, HIGH, 25000);   // 25 ms ≈ 4 m and back
  if (us == 0) return -1;
  return us / 58.0;
}

float readDistanceCm() {
  float a = pingOnceCm(); delay(10);
  float b = pingOnceCm(); delay(10);
  float c = pingOnceCm();
  // middle of three
  if ((a <= b && b <= c) || (c <= b && b <= a)) return b;
  if ((b <= a && a <= c) || (c <= a && a <= b)) return a;
  return c;
}

// ── Firebase ─────────────────────────────────────────────────────

static String firebaseUrl() {
  String url = DATABASE_URL;
  while (url.endsWith("/")) url.remove(url.length() - 1);
  String path = DATA_PATH;
  if (!path.startsWith("/")) path = "/" + path;
  return url + path + ".json";
}

static void explain(int code) {
  if (code == 401 || code == 403) Serial.println("[presence]   permission denied: publish the database rules from the guide");
  else if (code == 404) Serial.println("[presence]   not found: check DATABASE_URL");
  else if (code < 0) Serial.printf("[presence]   couldn't reach Firebase (%s)\n", fbHttp.errorToString(code).c_str());
}

void sendToFirebase(float cm, bool near) {
  unsigned long now = millis();
  bool changed = (int)near != fbLastNear;
  if (!changed && now - fbLastSend < SEND_EVERY_MS) return;

  if (WiFi.status() != WL_CONNECTED) {
    if (now - wifiLastTry > 10000) joinWifi();
    return;
  }

  String body = String("{\"distance\":") + String(cm, 1) +
                ",\"near\":" + (near ? "true" : "false") +
                ",\"updatedAt\":{\".sv\":\"timestamp\"}}";

  fbHttp.begin(fbClient, firebaseUrl());
  fbHttp.addHeader("Content-Type", "application/json");
  int code = fbHttp.PATCH(body);
  fbHttp.end();

  fbLastSend = now;
  if (code == 200) {
    fbLastNear = near;
    if (changed) Serial.printf("[presence] %s (%.0f cm)\n", near ? "someone is near" : "nobody near", cm);
  } else {
    Serial.printf("[presence] Firebase replied %d\n", code);
    explain(code);
  }
}

// ── called from your sketch ──────────────────────────────────────

void plumbingBegin() {
  Serial.begin(115200);
  delay(500);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  fbClient.setInsecure();   // classroom shortcut: skips checking Google's certificate
  fbHttp.setReuse(true);    // keep the connection open between sends
  fbHttp.setTimeout(4000);

  Serial.println("\n[presence] starting");
  joinWifi();
  Serial.printf("[presence] sending to %s\n", firebaseUrl().c_str());
}
