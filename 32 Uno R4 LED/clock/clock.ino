/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1

Original code from Mario's Ideas
https://www.youtube.com/watch?v=2Ip8FWd6w60
-------------------------------------------------*/

#include "RTC.h"
#include "Arduino_LED_Matrix.h"
ArduinoLEDMatrix matrix;

byte Time[8][12] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

byte  Digits  [5][30]{                                                                 
{ 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
{ 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1 },
{ 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1 },
{ 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1 },
{ 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1 },
};                                   


int currentSecond;
boolean secondsON_OFF = 1;
int hours, minutes, seconds, year, dayofMon;
String dayofWeek, month;

void displayDigit(int d, int s_x, int s_y){
  for (int i=0;i<3;i++)
    for (int j=0;j<5;j++)
      Time[i+s_x][11-j-s_y] = Digits[j][i+d*3];   
    
  matrix.renderBitmap(Time, 8, 12);
}


DayOfWeek convertDOW(String dow){
  if (dow == String("Mon")) return DayOfWeek::MONDAY;
  if (dow == String("Tue")) return DayOfWeek::TUESDAY;
  if (dow == String("Wed")) return DayOfWeek::WEDNESDAY;
  if (dow == String("Thu")) return DayOfWeek::THURSDAY;
  if (dow == String("Fri")) return DayOfWeek::FRIDAY;
  if (dow == String("Sat")) return DayOfWeek::SATURDAY;
  if (dow == String("Sun")) return DayOfWeek::SUNDAY;
}

Month convertMonth(String m){
 if (m == String("Jan")) return Month::JANUARY;
  if (m == String("Feb")) return Month::FEBRUARY;
  if (m == String("Mar")) return Month::MARCH;
  if (m == String("Apr")) return Month::APRIL;
  if (m == String("May")) return Month::MAY;
  if (m == String("Jun")) return Month::JUNE;
  if (m == String("Jul")) return Month::JULY;
  if (m == String("Aug")) return Month::AUGUST;
  if (m == String("Sep")) return Month::SEPTEMBER;
  if (m == String("Oct")) return Month::OCTOBER;
  if (m == String("Nov")) return Month::NOVEMBER;
  if (m == String("Dec")) return Month::DECEMBER;
}

void getCurTime(String timeSTR,String* d_w,int* d_mn, String* mn,int* h,int* m,int* s,int* y){
  
  *d_w = timeSTR.substring(0,3);
  *mn = timeSTR.substring(4,7);
  *d_mn = timeSTR.substring(8,11).toInt();
  *h = timeSTR.substring(11,13).toInt();
  *m = timeSTR.substring(14,16).toInt();
  *s = timeSTR.substring(17,19).toInt();
  *y = timeSTR.substring(20,24).toInt();

}


void setup() {  
  Serial.begin(9600);  
  matrix.begin();
  RTC.begin();    
  String timeStamp = __TIMESTAMP__;
  getCurTime(timeStamp,&dayofWeek,&dayofMon,&month,&hours,&minutes,&seconds,&year);
  RTCTime startTime(dayofMon, convertMonth(month) , year, hours, minutes, seconds, 
                    convertDOW(dayofWeek), SaveLight::SAVING_TIME_ACTIVE); 
  RTC.setTime(startTime);
}

void loop(){
  RTCTime currentTime;
  RTC.getTime(currentTime);
  if (currentTime.getSeconds()!=currentSecond){
    secondsON_OFF ?  secondsON_OFF = 0 : secondsON_OFF = 1;
    displayDigit((int)(currentTime.getHour()/10),0,0 );
    displayDigit(currentTime.getHour()%10,4,0 );
    displayDigit((int)(currentTime.getMinutes()/10),1,6 );
    displayDigit(currentTime.getMinutes()%10,5,6 );
    Time[0][2]=secondsON_OFF;
    Time[0][4]=secondsON_OFF;
    currentSecond=currentTime.getSeconds();
    matrix.renderBitmap(Time, 8, 12);
    Serial.println(secondsON_OFF);
  }
}