/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/
#include "WiFiS3.h"
#include <Arduino_LED_Matrix.h>
#include "webpage.h"

//Change the below variables to your need
#define SECRET_SSID "WiFi name"
#define SECRET_PASS "WiFi password"
#define LED 2

int status = WL_IDLE_STATUS;
WiFiServer server(80);
ArduinoLEDMatrix matrix;

void setup() {
  Serial.begin(115200);  // initialize serial communication
  pinMode(LED, OUTPUT);  // set the LED pin mode

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true)
      ;
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(SECRET_SSID);  // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(SECRET_SSID, SECRET_PASS);
    // wait 10 seconds for connection:
    delay(2300);
  }
  server.begin();     // start the web server on port 80
  printWifiStatus();  // you're connected now, so print out the status

  matrix.begin();
  print_rangeSlider(0);
}


void loop() {
  WiFiClient client = server.available();  // listen for incoming clients

  if (client) {                    // if you get a client,
    Serial.println("new client");  // print a message out the serial port
    String currentLine = "";       // make a String to hold incoming data from the client
    while (client.connected()) {   // loop while the client's connected
      if (client.available()) {    // if there's bytes to read from the client,
        char c = client.read();    // read a byte, then
        //Serial.write(c);           // print it out to the serial monitor
        if (c == '\n') {  // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            String html = String(HTML_CONTENT);
            client.println(html);
            client.flush();

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {  // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        //Receive data from the webserver
        if (currentLine.endsWith("range")) {
          Serial.println("Range");
          String value = currentLine.substring(currentLine.length() - 7, currentLine.length() - 5);
          Serial.println(value);
          print_rangeSlider(value.toInt());
        }
        if (currentLine.endsWith("LEDON")) {
          Serial.println("LED ON");
          matrix.loadFrame(leds[1]);
          digitalWrite(LED, HIGH);
        }
        if (currentLine.endsWith("LEDOFF")) {
          Serial.println("LED OFF");
          matrix.loadFrame(leds[0]);
          digitalWrite(LED, LOW);
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

void print_rangeSlider(int value) {
  value = map(value, 0, 100, 0, 11);
  matrix.loadFrame(frame_slider[value]);
  //delay(1);
}