const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Arm Control</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background: #111;
      color: #eee;
      text-align: center;
      margin: 0;
      padding: 10px;
    }

    .top-slider {
      width: 90%;
      margin: 20px auto;
    }

    .container {
      display: flex;
      justify-content: space-around;
      align-items: center;
      height: 60vh;
    }

    .v-slider {
      writing-mode: bt-lr;
      -webkit-appearance: slider-vertical;
      width: 40px;
      height: 80%;
    }

    button {
      width: 90%;
      height: 60px;
      font-size: 22px;
      margin-top: 20px;
      background: #2ecc71;
      border: none;
      border-radius: 10px;
    }
  </style>
</head>

<body>
  <h2>ESP32 Robotic Arm</h2>

  <div class="container">
    <!-- Forward / Backward -->
    <input id="fb" class="v-slider" type="range" min="0" max="180" value="90">

    <!-- Up / Down -->
    <input id="ud" class="v-slider" type="range" min="0" max="180" value="90">
  </div>

  <!-- Left / Right -->
  <input id="lr" class="top-slider" type="range" min="0" max="180" value="90">

  <!-- Open / Close -->
  <button id="grip">OPEN</button>

  <!-- Reset -->
  <button id="reset">Reset</button>

<script>
  let lastSend = 0;
  const SEND_INTERVAL = 50; // ms, ~20 Hz

  function sendData() {
    const now = Date.now();
    if (now - lastSend < SEND_INTERVAL) return;
    lastSend = now;

    const lr = document.getElementById("lr").value;
    const fb = document.getElementById("fb").value;
    const ud = document.getElementById("ud").value;
    const grip = document.getElementById("grip").dataset.state || 0;

    fetch(`/servo?lr=${lr}&fb=${fb}&ud=${ud}&grip=${grip}`);
  }

  document.querySelectorAll("input[type=range]").forEach(slider => {
    slider.addEventListener("input", sendData);
  });

  document.getElementById("grip").addEventListener("click", () => {
    const btn = document.getElementById("grip");
    const state = btn.dataset.state === "1" ? 0 : 1;
    btn.dataset.state = state;
    btn.textContent = state ? "Close" : "Open";
    sendData();
  });

  document.getElementById("reset").addEventListener("click", () => {
  // reset sliders visually
  document.getElementById("lr").value = 90;
  document.getElementById("fb").value = 90;
  document.getElementById("ud").value = 90;

  // reset gripper button state
  const gripBtn = document.getElementById("grip");
  gripBtn.dataset.state = 1;   // closed
  gripBtn.textContent = "Close";

  sendData();
  });
</script>
</body>
</html>
)rawliteral";