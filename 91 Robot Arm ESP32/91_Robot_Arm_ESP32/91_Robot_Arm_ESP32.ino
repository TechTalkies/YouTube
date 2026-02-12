/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

#include <WiFi.h>
#include <WiFiAP.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include "web.h"

const char* ssid = "ESP32 Robot";
const char* password = "12345678";

WebServer server(80);

static const int servoPinLR = D2;
static const int servoPinFB = D3;
static const int servoPinUD = D4;
static const int servoPinGrip = D5;

Servo servoLR, servoFB, servoUD, servoGrip;

int lr_target = 90;
int fb_target = 90;
int ud_target = 90;
int grip_target = 0;

int lr_current = 90;
int fb_current = 90;
int ud_current = 90;
int grip_current = 0;

const int GRIP_OPEN = 180;
const int GRIP_CLOSE = 60;

const int STEP_ARM = 2;   // smooth arm motion
const int STEP_GRIP = 5;  // faster gripper

void handleServo() {

  if (server.hasArg("lr"))
    lr_target = constrain(server.arg("lr").toInt(), 0, 180);

  if (server.hasArg("fb"))
    fb_target = constrain(server.arg("fb").toInt(), 0, 180);

  if (server.hasArg("ud"))
    ud_target = constrain(server.arg("ud").toInt(), 0, 180);

  if (server.hasArg("grip")) {
    int g = server.arg("grip").toInt();
    grip_target = (g == 1) ? GRIP_CLOSE : GRIP_OPEN;
  }

  Serial.printf("T -> lr:%d fb:%d ud:%d grip:%d\n",
                lr_target, fb_target, ud_target, grip_target);

  // Quick ACK
  server.send(200, "text/plain", "ok");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  delay(1000);
  IPAddress IP = IPAddress(10, 10, 10, 1);
  IPAddress NMask = IPAddress(255, 255, 255, 0);
  WiFi.softAPConfig(IP, IP, NMask);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  /* SETUP YOR WEB OWN ENTRY POINTS */
  server.on("/servo", handleServo);

  // Route for root / web page
  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", index_html);
  });

  server.begin();
  Serial.println("HTTP server started");

  Serial.begin(115200);
  servoLR.attach(servoPinLR);
  servoFB.attach(servoPinFB);
  servoUD.attach(servoPinUD);
  servoGrip.attach(servoPinGrip);
}

void loop() {
  server.handleClient();
  delay(2);  //allow the cpu to switch to other tasks

  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate >= 20) {
    lastUpdate = millis();

    // Left / Right
    if (lr_current < lr_target) lr_current += STEP_ARM;
    else if (lr_current > lr_target) lr_current -= STEP_ARM;
    if (abs(lr_current - lr_target) < STEP_ARM) lr_current = lr_target;

    // Forward / Backward
    if (fb_current < fb_target) fb_current += STEP_ARM;
    else if (fb_current > fb_target) fb_current -= STEP_ARM;
    if (abs(fb_current - fb_target) < STEP_ARM) fb_current = fb_target;

    // Up / Down
    if (ud_current < ud_target) ud_current += STEP_ARM;
    else if (ud_current > ud_target) ud_current -= STEP_ARM;
    if (abs(ud_current - ud_target) < STEP_ARM) ud_current = ud_target;

    // Gripper (faster, or even snap if you want)
    if (grip_current < grip_target) grip_current += STEP_GRIP;
    else if (grip_current > grip_target) grip_current -= STEP_GRIP;
    if (abs(grip_current - grip_target) < STEP_GRIP) grip_current = grip_target;

    // ---- ACTUAL SERVO OUTPUTS HERE ----
    servoLR.write(lr_current);
    servoFB.write(fb_current);
    servoUD.write(ud_current);
    servoGrip.write(grip_current);
  }
}