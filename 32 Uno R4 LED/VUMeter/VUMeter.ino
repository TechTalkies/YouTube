/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
Include a link if re-using.
https://www.youtube.com/@techtalkies1

Thanks to @paytech60 from the Arduino Forums.
https://forum.arduino.cc/t/interesting-video/1146883/7
-------------------------------------------------*/

#include <arduinoFFT.h>
#include <Arduino_LED_Matrix.h>

#define SAMPLES         256          // Must be a power of 2
#define SAMPLING_FREQ   20000         // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.
#define AUDIO_IN_PIN    0             // Signal in on this pin (same as A0 or Analog 0)
#define NUM_BANDS       12            // To change this, you will need to change the bunch of if statements describing the mapping from bins to bands
#define NOISE           100           // Used as a crude noise filter, values below this are ignored


ArduinoLEDMatrix matrix;
uint8_t frame[8][12] = {0};

// Sampling and FFT stuff
unsigned int sampling_period_us;
byte peak[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};              // The length of these arrays must be >= NUM_BANDS
int bandValues[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
double vReal[SAMPLES];
double vImag[SAMPLES];
unsigned long newTime;
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES, SAMPLING_FREQ); //This was changed from the orig 
// Was: arduinoFFT FFT = arduinoFFT(vReal, vImag, SAMPLES, SAMPLING_FREQ);
// Recomended use case is: ArduinoFFT<float> FFT = ArduinoFFT<float>(vReal, vImag, SAMPLES, SAMPLING_FREQ);

double NN;
int HH = 128;

void setup() {
  Serial.begin(115200);
  matrix.begin();
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQ));
}


void loop() {

  // Reset bandValues[]
  for (int i = 0; i<NUM_BANDS; i++){
    bandValues[i] = 0;
  }

  // Sample the audio pin
  for (int i = 0; i < SAMPLES; i++) {
    newTime = micros();
    vReal[i] = analogRead(AUDIO_IN_PIN); // A conversion takes about 9.7uS on an ESP32
    vImag[i] = 0;
    while ((micros() - newTime) < sampling_period_us) { /* chill */ }
  }

  // Compute FFT
  FFT.dcRemoval(); // Was: FFT.DCRemoval(); 
  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD); // Was: FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD); // Was: FFT.Compute(FFT_FORWARD);
  FFT.complexToMagnitude(); // Was: FFT.ComplexToMagnitude();

  
  // Analyse FFT results
  for (int i = 2; i < (SAMPLES/2); i++){       // Don't use sample 0 and only first SAMPLES/2 are usable. Each array element represents a frequency bin and its value the amplitude.
    if (vReal[i] > NOISE) {                    // Add a crude noise filter

    
      if (i<=2 )           bandValues[0]   += (int)vReal[i]; NN = (int)((bandValues[0]/2)/HH)+1;  if (NN > 8) NN = 8; for (int x=0;x<NN;x++){frame[x][11] = true;} for (int x=NN;x<8;x++){frame[x][11] = false;}
      if (i>2   && i<=3  ) bandValues[1]   += (int)vReal[i]; NN = (int)((bandValues[1]/2)/HH)+1;  if (NN > 8) NN = 8; for (int x=0;x<NN;x++){frame[x][10] = true;} for (int x=NN;x<8;x++){frame[x][10] = false;}
      if (i>3   && i<=4  ) bandValues[2]   += (int)vReal[i]; NN = (int)((bandValues[2]/2)/HH)+1;  if (NN > 8) NN = 8; for (int x=0;x<NN;x++){frame[x][9] = true;} for (int x=NN;x<8;x++){frame[x][9] = false;}
      if (i>4   && i<=6  ) bandValues[3]   += (int)vReal[i]; NN = (int)((bandValues[3]/2)/HH)+1;  if (NN > 8) NN = 8; for (int x=0;x<NN;x++){frame[x][8] = true;} for (int x=NN;x<8;x++){frame[x][8] = false;}
      if (i>6   && i<=9  ) bandValues[4]   += (int)vReal[i]; NN = (int)((bandValues[4]/3)/HH)+1;  if (NN > 8) NN = 8; for (int x=0;x<NN;x++){frame[x][7] = true;} for (int x=NN;x<8;x++){frame[x][7] = false;}
      if (i>9   && i<=13  ) bandValues[5]   += (int)vReal[i]; NN = (int)((bandValues[5]/4)/HH)+1;  if (NN > 8) NN = 8; for (int x=0;x<NN;x++){frame[x][6] = true;} for (int x=NN;x<8;x++){frame[x][6] = false;}
      if (i>13  && i<=20 ) bandValues[6]   += (int)vReal[i]; NN = (int)((bandValues[6]/4)/HH)+1;  if (NN > 8) NN = 8; for (int x=0;x<NN;x++){frame[x][5] = true;} for (int x=NN;x<8;x++){frame[x][5] = false;}
      if (i>20  && i<=30 ) bandValues[7]   += (int)vReal[i]; NN = (int)((bandValues[7]/4)/HH)+1;  if (NN > 8) NN = 8; for (int x=0;x<NN;x++){frame[x][4] = true;} for (int x=NN;x<8;x++){frame[x][4] = false;}
      if (i>30  && i<=46 ) bandValues[8]   += (int)vReal[i]; NN = (int)((bandValues[8]/4)/HH)+1;  if (NN > 8) NN = 8; for (int x=0;x<NN;x++){frame[x][3] = true;} for (int x=NN;x<8;x++){frame[x][3] = false;}
      if (i>46  && i<=70 ) bandValues[9]   += (int)vReal[i]; NN = (int)((bandValues[9]/4)/HH)+1;  if (NN > 8) NN = 8; for (int x=0;x<NN;x++){frame[x][2] = true;} for (int x=NN;x<8;x++){frame[x][2] = false;}
      if (i>70  && i<=106 ) bandValues[10]  += (int)vReal[i]; NN = (int)((bandValues[10]/4)/HH)+1; if (NN > 8) NN = 8; for (int x=0;x<NN;x++){frame[x][1] = true;} for (int x=NN;x<8;x++){frame[x][1] = false;}
      if (i>106           ) bandValues[11]  += (int)vReal[i]; NN = (int)((bandValues[11]/4)/HH)+1; if (NN > 8) NN = 8; for (int x=0;x<NN;x++){frame[x][0] = true;} for (int x=NN;x<8;x++){frame[x][0] = false;}

      
      matrix.renderBitmap(frame, 8, 12);
      

    }
  }
}