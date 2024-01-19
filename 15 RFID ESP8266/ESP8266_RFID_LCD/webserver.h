const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP8266 Access Portal</title>
  <style>
    html {font-family: Times New Roman; display: inline-block; text-align: center;}
    h2 {font-size: 2.3rem; color: #E9eeee;}
    p {font-size: 1.5rem; color: #E9eeee;}
    body {max-width: 400px; margin:0px auto; padding-bottom: 25px; background: #2c3233;}
    .slider { -webkit-appearance: none; margin: 5px; width: 360px; height: 25px; background: #8300CE;
      outline: none; -webkit-transition: .2s; transition: opacity .2s;}
    .slider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 35px; height: 35px; background:#01070a; cursor: pointer;}
    .slider::-moz-range-thumb { width: 35px; height: 35px; background: #01070a; cursor: pointer; } 
    .button { background-color: #195B6A; border: none; color: white; padding: 16px 20px;
    text-decoration: none; font-size: 20px; margin: 2px; cursor: pointer;}
    .button2 {background-color: #77878A;}
  </style>
</head>
<body>
  <h2>Tech Talkies Access management</h2>
  <p>Red: %PERSON1% times</p>
  <p>Blue: %PERSON2% times</p>
  <p>Green: %PERSON3% times</p>
  <p>Unauthorized: %UNKNOWN% times</p>
<script>

</script>
</body>
</html>
)rawliteral";