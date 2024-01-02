const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Tech Talkies</title>
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 2rem;}
    p {font-size: 1.9rem;}
    body {max-width: 400px; margin:0px auto; padding-bottom: 25px;}
    .slider { -webkit-appearance: none; margin: 14px; width: 360px; height: 25px; 
    background: linear-gradient(to right, red,orange,yellow,green,blue,indigo,violet, red);
      outline: none; -webkit-transition: .2s; transition: opacity .2s;}
	   .button { background-color: #195B6A; border: none; color: white; padding: 16px 20px;
    text-decoration: none; font-size: 20px; margin: 2px; cursor: pointer;}
    .textarea {width:90%;}
  </style>
</head>

<body>
  <h2>Tech Talkies Web Logger</h2>
  <form>
		<label for="logText">Enter log data to send:</label><br>

		<textarea class="textarea" id="logText" name="logText" rows="4"></textarea>
		<button class="button" type="submit" onclick="sendLogData()">Send</button> 
    </form><br>
	
	<p><button class="button"onclick="getLogData();">Read Log</button></p>
	
  <textarea class="textarea" id="logHere" name="logHere" rows="10" readonly></textarea>
	
<script>
function sendLogData(element) {
  var textValue = document.getElementById("logText").value;
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/update?value="+textValue, true);
  xhr.send();
}
function getLogData(element) {
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
       // Typical action to be performed when the document is ready:
       document.getElementById("logHere").innerHTML = xhr.responseText;
    }
};

  xhr.open("GET", "/getlog", true);
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";