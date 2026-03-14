/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVOMIN 150
#define SERVOMAX 600

const char* ssid = "ESP32_SERVO";
const char* password = "12345678";

WiFiServer server(80);

bool lockMode = false;

int servoPos[5] = { 0, 0, 0, 0, 0 };

String webpage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>Servo Control</title>
<style>
body{font-family:Arial;text-align:center;}
.slider{width:300px;}
button{padding:10px;margin:10px;font-size:16px;}
</style>
</head>
<body>

<h2>Servo Controller</h2>

<div>
Servo 1<br>
<input type="range" min="0" max="180" value="0" class="slider" id="s0" oninput="send(0,this.value)">
</div>

<div>
Servo 2<br>
<input type="range" min="0" max="180" value="0" class="slider" id="s1" oninput="send(1,this.value)">
</div>

<div>
Servo 3<br>
<input type="range" min="0" max="180" value="0" class="slider" id="s2" oninput="send(2,this.value)">
</div>

<div>
Servo 4<br>
<input type="range" min="0" max="180" value="0" class="slider" id="s3" oninput="send(3,this.value)">
</div>

<div>
Servo 5<br>
<input type="range" min="0" max="180" value="0" class="slider" id="s4" oninput="send(4,this.value)">
</div>

<br>

<button onclick="lock()">Lock</button>
<button onclick="reset()">Reset</button>

<script>

function send(id,val)
{
 fetch("/servo?id="+id+"&val="+val);
}

function lock()
{
 fetch("/lock");
}

function reset()
{
 fetch("/reset");

 for(let i=0;i<5;i++)
 document.getElementById("s"+i).value=0;
}

</script>

</body>
</html>
)rawliteral";

void moveServo(int id, int angle) {
  int pwmVal = map(angle, 0, 180, SERVOMIN, SERVOMAX);
  pwm.setPWM(id, 0, pwmVal);
}

void setup() {
  Serial.begin(115200);

  pwm.begin();
  pwm.setPWMFreq(50);

  WiFi.softAP(ssid, password);

  Serial.println(WiFi.softAPIP());

  server.begin();

  for (int i = 0; i < 5; i++)
    moveServo(i, 0);
}

void loop() {
  WiFiClient client = server.available();

  if (!client) return;

  String req = client.readStringUntil('\r');
  client.flush();

  if (req.indexOf("GET / ") >= 0) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type:text/html\n");
    client.println(webpage);
  }

  if (req.indexOf("/servo") >= 0) {
    int id = req.substring(req.indexOf("id=") + 3).toInt();
    int val = req.substring(req.indexOf("val=") + 4).toInt();

    servoPos[id] = val;

    if (lockMode) {
      for (int i = 0; i < 5; i++) {
        servoPos[i] = val;
        moveServo(i, val);
      }
    } else {
      moveServo(id, val);
    }
  }

  if (req.indexOf("/lock") >= 0) {
    lockMode = !lockMode;

    for (int i = 0; i < 5; i++) {
      servoPos[i] = 0;
      moveServo(i, 0);
    }
  }

  if (req.indexOf("/reset") >= 0) {
    for (int i = 0; i < 5; i++) {
      servoPos[i] = 0;
      moveServo(i, 0);
    }
  }

  client.stop();
}