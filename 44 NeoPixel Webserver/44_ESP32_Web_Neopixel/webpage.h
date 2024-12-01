const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Tech Talkies LED Ring</title>
  <style>
    :root {
      --circle-size: 80vw; /* Circle size as a percentage of viewport width */
      --active-color: #4CAF50; /* Color for selected buttons */
      --inactive-color: #ddd;  /* Color for unselected buttons */
      --text-color: #333;      /* Text color */
    }

    body {
      display: flex;
      justify-content: center;
      align-items: center;
      min-height: 100vh;
      margin: 0;
      background-color: #f5f5f5;
      font-family: Arial, sans-serif;
    }

    .circle-container {
      position: relative;
      width: var(--circle-size);
      height: var(--circle-size);
      max-width: 500px;
      max-height: 500px;
      border: 2px dashed var(--inactive-color); /* Visual guide for the circle */
      border-radius: 50%%;
    }

    .circle-container label {
	  position: absolute;
	  top: 50%%;
	  left: 50%%;
	  width: 15%%;
	  height: 15%%;
	  background-color: var(--inactive-color);
	  border-radius: 50%%;
	  text-align: center;
	  display: flex;
	  justify-content: center;
	  align-items: center;
	  font-size: 1rem;
	  color: var(--text-color);
	  cursor: pointer;
	  transform: translate(-50%%, -50%%);
	  transition: background-color 0.3s, color 0.3s;
	  flex-direction: column; /* Align checkbox and radio button vertically */
	  gap: 2px; /* Space between the checkbox and the radio button */
	}


    .circle-container input[type="checkbox"] {
      display: none;
    }

    .circle-container input[type="checkbox"]:checked + label {
      background-color: var(--active-color);
      color: white;
    }

    /* Dynamically arrange buttons in a perfect circle */
    .circle-container label:nth-of-type(1)  { transform: translate(-50%%, -50%%) rotate(0deg)    translate(calc(var(--circle-size) / 2.2)) rotate(-0deg); }
    .circle-container label:nth-of-type(2)  { transform: translate(-50%%, -50%%) rotate(22.5deg) translate(calc(var(--circle-size) / 2.2)) rotate(-22.5deg); }
    .circle-container label:nth-of-type(3)  { transform: translate(-50%%, -50%%) rotate(45deg)   translate(calc(var(--circle-size) / 2.2)) rotate(-45deg); }
    .circle-container label:nth-of-type(4)  { transform: translate(-50%%, -50%%) rotate(67.5deg) translate(calc(var(--circle-size) / 2.2)) rotate(-67.5deg); }
    .circle-container label:nth-of-type(5)  { transform: translate(-50%%, -50%%) rotate(90deg)   translate(calc(var(--circle-size) / 2.2)) rotate(-90deg); }
    .circle-container label:nth-of-type(6)  { transform: translate(-50%%, -50%%) rotate(112.5deg) translate(calc(var(--circle-size) / 2.2)) rotate(-112.5deg); }
    .circle-container label:nth-of-type(7)  { transform: translate(-50%%, -50%%) rotate(135deg)  translate(calc(var(--circle-size) / 2.2)) rotate(-135deg); }
    .circle-container label:nth-of-type(8)  { transform: translate(-50%%, -50%%) rotate(157.5deg) translate(calc(var(--circle-size) / 2.2)) rotate(-157.5deg); }
    .circle-container label:nth-of-type(9)  { transform: translate(-50%%, -50%%) rotate(180deg)  translate(calc(var(--circle-size) / 2.2)) rotate(-180deg); }
    .circle-container label:nth-of-type(10) { transform: translate(-50%%, -50%%) rotate(202.5deg) translate(calc(var(--circle-size) / 2.2)) rotate(-202.5deg); }
    .circle-container label:nth-of-type(11) { transform: translate(-50%%, -50%%) rotate(225deg)  translate(calc(var(--circle-size) / 2.2)) rotate(-225deg); }
    .circle-container label:nth-of-type(12) { transform: translate(-50%%, -50%%) rotate(247.5deg) translate(calc(var(--circle-size) / 2.2)) rotate(-247.5deg); }
    .circle-container label:nth-of-type(13) { transform: translate(-50%%, -50%%) rotate(270deg)  translate(calc(var(--circle-size) / 2.2)) rotate(-270deg); }
    .circle-container label:nth-of-type(14) { transform: translate(-50%%, -50%%) rotate(292.5deg) translate(calc(var(--circle-size) / 2.2)) rotate(-292.5deg); }
    .circle-container label:nth-of-type(15) { transform: translate(-50%%, -50%%) rotate(315deg)  translate(calc(var(--circle-size) / 2.2)) rotate(-315deg); }
    .circle-container label:nth-of-type(16) { transform: translate(-50%%, -50%%) rotate(337.5deg) translate(calc(var(--circle-size) / 2.2)) rotate(-337.5deg); }
  
  
   /* Center Controls */
    .center-controls {
      position: absolute;
      top: 50%%;
      left: 50%%;
      transform: translate(-50%%, -50%%);
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 1rem;
      text-align: center;
    }

    .center-controls input[type="color"] {
      width: 50px;
      height: 50px;
      border: none;
      border-radius: 50%%;
      cursor: pointer;
    }
	</style>
</head>
<body>
<div class="circle-container">
  <input type="checkbox" id="cb1" onchange="toggleLED(0)"><label for="cb1">1 <input type="radio" name="circle-group" value="1" onclick="selectSingle(0)"></label>
  <input type="checkbox" id="cb2" onchange="toggleLED(1)"><label for="cb2">2 <input type="radio" name="circle-group" value="2" onclick="selectSingle(1)"></label>
  <input type="checkbox" id="cb3" onchange="toggleLED(2)"><label for="cb3">3 <input type="radio" name="circle-group" value="3" onclick="selectSingle(2)"></label>
  <input type="checkbox" id="cb4" onchange="toggleLED(3)"><label for="cb4">4 <input type="radio" name="circle-group" value="4" onclick="selectSingle(3)"></label>
  <input type="checkbox" id="cb5" onchange="toggleLED(4)"><label for="cb5">5 <input type="radio" name="circle-group" value="5" onclick="selectSingle(4)"></label>
  <input type="checkbox" id="cb6" onchange="toggleLED(5)"><label for="cb6">6 <input type="radio" name="circle-group" value="6" onclick="selectSingle(5)"></label>
  <input type="checkbox" id="cb7" onchange="toggleLED(6)"><label for="cb7">7 <input type="radio" name="circle-group" value="7" onclick="selectSingle(6)"></label>
  <input type="checkbox" id="cb8" onchange="toggleLED(7)"><label for="cb8">8 <input type="radio" name="circle-group" value="8" onclick="selectSingle(7)"></label>
  <input type="checkbox" id="cb9" onchange="toggleLED(8)"><label for="cb9">9 <input type="radio" name="circle-group" value="9" onclick="selectSingle(8)"></label>
  <input type="checkbox" id="cb10" onchange="toggleLED(9)"><label for="cb10">10 <input type="radio" name="circle-group" value="10" onclick="selectSingle(9)"></label>
  <input type="checkbox" id="cb11" onchange="toggleLED(10)"><label for="cb11">11 <input type="radio" name="circle-group" value="11" onclick="selectSingle(10)"></label>
  <input type="checkbox" id="cb12" onchange="toggleLED(11)"><label for="cb12">12 <input type="radio" name="circle-group" value="12" onclick="selectSingle(11)"></label>
  <input type="checkbox" id="cb13" onchange="toggleLED(12)"><label for="cb13">13 <input type="radio" name="circle-group" value="13" onclick="selectSingle(12)"></label>
  <input type="checkbox" id="cb14" onchange="toggleLED(13)"><label for="cb14">14 <input type="radio" name="circle-group" value="14" onclick="selectSingle(13)"></label>
  <input type="checkbox" id="cb15" onchange="toggleLED(14)"><label for="cb15">15 <input type="radio" name="circle-group" value="15" onclick="selectSingle(14)"></label>
  <input type="checkbox" id="cb16" onchange="toggleLED(15)"><label for="cb16">16 <input type="radio" name="circle-group" value="16" onclick="selectSingle(15)"></label>
</div>
  
  <!-- Center Controls -->
    <div class="center-controls">
      <input type="color" id="color-picker" value="%STARTCOLOR%" title="Pick a color">
      <div class="radio-group">
        <label><input type="radio" name="options" value="option1" onclick="sendMessage('select')">Group</label><br>
		    <label><input type="radio" name="options" value="option2" onclick="sendMessage('single')">Single</label><br>
        <label><input type="radio" name="options" value="option3" onclick="sendMessage('all')">All</label>
      </div>


<script>
  function toggleLED(i) {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/LED?value="+i, true);
    xhr.send();
  }

  function sendMessage(s) {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/setting?value="+s, true);
    xhr.send();
  }
  
  function selectSingle(i) {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/single?value="+i, true);
    xhr.send();
  }

  var colorInput = document.getElementById("color-picker");

  colorInput.addEventListener('change', function(e) {
    var result = this.value.slice(1);
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/color?value=" + result, true);
    xhr.send();
  });

</script>
</body>
</html>
)rawliteral";