/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/
const char* HTML_CONTENT = R""""(
<!DOCTYPE html>

<html>
<head>
  <meta charset="UTF-8"/>
  <meta name="viewport" content="width=device-width, initial-scale=1" >
  <title>Tech Talkies Web Server</title>
  
    <style>
    html {font-family: Times New Roman; display: inline-block; text-align: center;}
    h2 {font-size: 1.8rem; color: white;}
    p {font-size: 1.5rem; color: #E9eeee;}
    body {max-width: 400px; margin:0px auto; padding-bottom: 25px; background: linear-gradient(to right, #9966ff 47%, #0099ff 100%);

    .slider {position: top;
        top: 50%;
        left: 50%;
        width: 270px;
        height: 60px;
        padding: 30px;
        padding-left: 40px;
        margin: 20px;
        background: #fcfcfc;
        border-radius: 20px;

        align-items: center;
        box-shadow: 0px 15px 40px #7E6D5766;}
        
    .slider p {font-size: 26px;
          font-weight: 600;
          font-family: Open Sans;
          padding-left: 30px;
          color: black;}
          
    .slider input[type="range"] {-webkit-appearance:none !important;
                              width: 200px;
                              height: 2px;
                              background: black;
                              border: none;
                              outline: none;}
                              
    .slider input[type="range"]::-webkit-slider-thumb {-webkit-appearance: none !important;
                                                    width: 30px;
                                                    height:30px;
                                                    background: black;
                                                    border: 2px solid black;
                                                    border-radius: 50%;
                                                    cursor: pointer;}

    .slider input[type="range"]::-webkit-slider-thumb:hover {background: black;}

    .button { background-color: white; border: none; color: black; padding: 16px 20px; border-radius: 15px;
    text-decoration: none; font-size: 20px; margin: 2px; cursor: pointer;}
    .button2 {background-color: #77878A;}
  </style>
</head>

<body>
<h2><a href="https://www.youtube.com/@techtalkies1">Tech Talkies Web Server</a></h2>

  <div class="slider">
    <input id="range1" type="range" min="0" max="99" step="5" value="0" oninput="rangeUpdate(this.value)" onchange="rangeUpdate(this.value)">
    <p id="rangeValue">0</p>
    
    <br>
    <h2>LED</h2>

    <button class="button" onclick="buttonclick('LEDON');">ON</button></a>
	  <button class="button" onclick="buttonclick('LEDOFF');">OFF</button></a>

  </div> 

<script>
   function refresh(value) {
    document.getElementById("rangeValue").innerText = value;
  }

  function rangeUpdate(value) {
    var url = "/" + value + "range";
    console.log(url);
    refresh(value);
   
    fetch(url)
    .catch(error => {
        console.log(error);
      })       
  }

  function buttonclick(element){
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/read?value=" + element, true);
    xhr.send();
  }
    </script>
</body> 
</html>
)"""";

const uint32_t frame_slider[12][4] = {
  {
		0x80080,
		0xffffff8,
		0x800000,
		66
	},
	{
		0xc00c0,
		0xffffffc,
		0xc00000,
		66
	},
	{
		0x60060,
		0xffffff6,
		0x600000,
		66
	},
	{
		0x30030,
		0xffffff3,
		0x300000,
		66
	},
	{
		0x18018,
		0xffffff1,
		0x80180000,
		66
	},
	{
		0xc00c,
		0xffffff0,
		0xc00c0000,
		66
	},
	{
		0x6006,
		0xffffff0,
		0x60060000,
		66
	},
	{
		0x3003,
		0xffffff0,
		0x30030000,
		66
	},
	{
		0x1801,
		0x8ffffff0,
		0x18018000,
		66
	},
	{
		0xc00,
		0xcffffff0,
		0xc00c000,
		66
	},
	{
		0x600,
		0x6ffffff0,
		0x6006000,
		66
	},
	{
		0x300,
		0x3ffffff0,
		0x3003000,
		66
	}
};

const uint32_t leds[2][4] = {
	{
		0xeeea8,
		0x8acca88e,
		0x88000000,
		66
	},
	{
		0xec8aa,
		0x8aa8a98e,
		0x88000000,
		66
	}
};