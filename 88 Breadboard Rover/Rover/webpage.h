const char* htmlHomePage PROGMEM = R"HTML(
<!DOCTYPE html>
<html>
<head>
  <title>Breadboard Rover</title>
  <title>by Tech Talkies</title>
  <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">

  <style>
    body {
      position: fixed;
      font-family: 'Gill Sans','Gill Sans MT',Calibri,'Trebuchet MS',sans-serif;
      color: rgb(128,128,128);
      text-align: center;
      background: white;
      touch-action: none;
    }

    h1 {
      margin: 10px 0;
    }

    canvas {
      touch-action: none;
    }
  </style>
</head>

<body>
  <h1>Breadboard Rover</h1>

  <!-- MJPEG STREAM -->
  <img src="/stream" width="320" height="240">

  <p>
    L: <span id="x_coordinate">0</span>
    R: <span id="y_coordinate">0</span>
    Speed: <span id="speed">0</span> %
    Angle: <span id="angle">0</span>
  </p>

  <canvas id="canvas"></canvas>

<script>
  // =========================
  // WEBSOCKET (CONTROL ONLY)
  // =========================
  let websocket;

  function initWebSocket() {
    websocket = new WebSocket("ws://" + window.location.host + "/CarInput");

    websocket.onopen = () => console.log("Control WS connected");
    websocket.onclose = () => setTimeout(initWebSocket, 2000);
  }

  window.addEventListener("load", initWebSocket);

  function send(left, right) {
    if (!websocket || websocket.readyState !== 1) return;
    websocket.send("MoveCar," + left + "," + right);
  }
</script>

<script>
  // =========================
  // JOYSTICK CODE (UNCHANGED)
  // =========================
  var canvas, ctx;
  var width, height, radius, x_orig, y_orig;
  let coord = { x: 0, y: 0 };
  let paint = false;

  window.addEventListener('load', () => {
    canvas = document.getElementById('canvas');
    ctx = canvas.getContext('2d');
    resize();

    document.addEventListener('mousedown', startDrawing);
    document.addEventListener('mouseup', stopDrawing);
    document.addEventListener('mousemove', Draw);

    document.addEventListener('touchstart', startDrawing, { passive: true });
    document.addEventListener('touchend', stopDrawing, { passive: true });
    document.addEventListener('touchcancel', stopDrawing, { passive: true });
    document.addEventListener('touchmove', Draw, { passive: true });

    window.addEventListener('resize', resize);
  });

  function resize() {
  const MAX_RADIUS = 140;   // 🔥 cap size here
  const MIN_RADIUS = 90;

  width = window.innerWidth;
  height = window.innerHeight;

  // scale radius based on screen, but cap it
  radius = Math.min(MAX_RADIUS, Math.max(MIN_RADIUS, width * 0.25));

  canvas.width = width;
  canvas.height = height;

  x_orig = width / 2;
  y_orig = height * 0.6;    // lower half of screen

  ctx.clearRect(0, 0, canvas.width, canvas.height);
  background();
  joystick(x_orig, y_orig);
}

  function background() {
    x_orig = width / 2;
    y_orig = height / 3;
    ctx.beginPath();
    ctx.arc(x_orig, y_orig, radius + 20, 0, Math.PI * 2);
    ctx.fillStyle = '#ECE5E5';
    ctx.fill();
  }

  function joystick(x, y) {
    ctx.beginPath();
    ctx.arc(x, y, radius, 0, Math.PI * 2);
    ctx.fillStyle = '#F08080';
    ctx.fill();
    ctx.lineWidth = 8;
    ctx.strokeStyle = '#F6ABAB';
    ctx.stroke();
  }

  function getPosition(event) {
    let e = event.touches ? event.touches[0] : event;
    coord.x = e.clientX - canvas.offsetLeft;
    coord.y = e.clientY - canvas.offsetTop;
  }

  function is_it_in_the_circle() {
    return Math.hypot(coord.x - x_orig, coord.y - y_orig) <= radius;
  }

  function startDrawing(event) {
    paint = true;
    getPosition(event);
    Draw(event);
  }

  function stopDrawing() {
    paint = false;
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    background();
    joystick(width / 2, height / 3);
    send(0, 0);
  }

  function getfuerza(nJoyX, nJoyY) {
    let fPivYLimit = 32.0;
    let nMotPremixL, nMotPremixR;

    if (nJoyY >= 0) {
      nMotPremixL = (nJoyX >= 0 ? 100 : 100 + nJoyX);
      nMotPremixR = (nJoyX >= 0 ? 100 - nJoyX : 100);
    } else {
      nMotPremixL = (nJoyX >= 0 ? 100 - nJoyX : 100);
      nMotPremixR = (nJoyX >= 0 ? 100 : 100 + nJoyX);
    }

    nMotPremixL *= nJoyY / 100.0;
    nMotPremixR *= nJoyY / 100.0;

    let nPivSpeed = nJoyX;
    let fPivScale = (Math.abs(nJoyY) > fPivYLimit) ? 0.0 : (1.0 - Math.abs(nJoyY) / fPivYLimit);

    let left  = (1.0 - fPivScale) * nMotPremixL + fPivScale * nPivSpeed;
    let right = (1.0 - fPivScale) * nMotPremixR - fPivScale * nPivSpeed;

    left  = Math.round(left  * 2.55);
    right = Math.round(right * 2.55);

    send(left, right);
  }

  function Draw(event) {
    if (!paint) return;

    ctx.clearRect(0, 0, canvas.width, canvas.height);
    background();
    getPosition(event);

    let angle = Math.atan2(coord.y - y_orig, coord.x - x_orig);
    let dist = Math.min(radius, Math.hypot(coord.x - x_orig, coord.y - y_orig));

    let x = x_orig + dist * Math.cos(angle);
    let y = y_orig + dist * Math.sin(angle);

    joystick(x, y);

    let x_rel = Math.round(x - x_orig);
    let y_rel = Math.round(y - y_orig);
    let speed = Math.round(100 * dist / radius);

    document.getElementById("x_coordinate").innerText = x_rel;
    document.getElementById("y_coordinate").innerText = y_rel;
    document.getElementById("speed").innerText = speed;
    document.getElementById("angle").innerText = Math.round(angle * 180 / Math.PI);

    getfuerza(x_rel, y_rel);
  }
</script>

</body>
</html>
)HTML";
