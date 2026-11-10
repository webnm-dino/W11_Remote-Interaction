/*
  DM3014 Interactive Devices, Ashley Hi 2026
  Week 11 – Remote Interaction
  An ultrasonic sensor measures how far away someone is. The board
  sends that distance to Firebase, and a web page anywhere in the
  world reacts to it.

  This is the file you edit. The Wi-Fi and Firebase code live in the
  firebase_plumbing.h tab; you don't need to open it.

  No libraries to install.
*/

#include "firebase_plumbing.h"

// ── 1. Wi-Fi ────────────────────────────────────────────────────────
// A 2.4 GHz network with a plain password. A phone hotspot works well.
// School or university networks that ask you to log in will not work.
const char* WIFI_NAME     = "wifi-name";
const char* WIFI_PASSWORD = "wifi-password";

// ── 2. Firebase ─────────────────────────────────────────────────────
// Copy the database URL from the Firebase console.
const char* DATABASE_URL = //;

// Where in the database the readings go. Your page reads the same path.
const char* DATA_PATH = "/sensor";

// ── 3. Sensor pins ──────────────────────────────────────────────────
const int TRIG_PIN = 3;
const int ECHO_PIN = 4; // sensor powered from 3V3, so ECHO goes straight in

// ── 4. Interaction ──────────────────────────────────────────────────
// Closer than this counts as "someone is here".
const float NEAR_CM = 60;

void setup() {
  plumbingBegin();
}

void loop() {
  float cm = readDistanceCm(); // -1 means nothing in range
  bool near = (cm > 0 && cm < NEAR_CM);

  // sends twice a second, and straight away whenever "near" changes
  sendToFirebase(cm, near);
  delay(60);
}
