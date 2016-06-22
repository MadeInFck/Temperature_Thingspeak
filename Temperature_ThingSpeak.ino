#include <SoftwareSerial.h>
#include <SFE_BMP180.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
 
#define DEBUG true
#define ALTITUDE 56.0 //Altitude of my home
#define OLED_RESET 4

Adafruit_SSD1306 display(OLED_RESET);
SFE_BMP180 pressure;
SoftwareSerial esp8266(10,11); // make RX Arduino line is pin 10, make TX Arduino line is pin 11.
                             // This means that you need to connect the TX line from the esp to the Arduino's pin 10
                             // and the RX line from the esp to the Arduino's pin 11.

unsigned long previousTime=millis();
bool first=true;
bool firstSend=true;
unsigned long previouSend=millis();
void setup()
{
  Serial.begin(9600);
  display.println(F("Waiting for data..."));
  display.display(); 
  // Welcome screen
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64) else 0x3D
  waitFor(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(F("MadeInFck.com"));
  display.println(F("Init of ESP8266..."));
  display.display();
  
    //Init BMP180
  Serial.println(F("Init BMP180"));
  // Initialize the sensor (it is important to get calibration values stored on the device).
  display.clearDisplay();
  display.setCursor(0,0);
  if (pressure.begin()) {
    Serial.println(F("BMP180 init success"));
    display.println(F("BMP180 init : OK"));
    display.display();
  } else
  {
    // Oops, something went wrong, this is usually a connection problem,
    // see the comments at the top of this sketch for the proper connections.
    Serial.println(F("BMP180 init fail\n\n"));
    display.println(F("BMP180 issue during"));
    display.println(F("init!"));
    display.display();
  }
  
  //Init ESP8266.
  esp8266.begin(9600); // your esp's baud rate might be different
  Serial.println(F("Initialisation du module Wifi!"));
  sendData("AT+RST\r\n",10000,DEBUG); // reset module
  sendData("AT+CWMODE=1\r\n",2000,DEBUG); // configure as STA point
  sendData("AT+CIFSR\r\n",2000,DEBUG); // get ip address
  sendData("AT+CIPMUX=0\r\n",2000,DEBUG); // configure for single connection
  waitFor(2000);

  //End of init screen
  display.println(F("Module Wifi init : OK"));
  display.display();
  waitFor(2000);
  display.clearDisplay();
}
 
void loop()
{
  char status;
  double T,P,p0,a;
  
  if ((millis()-previousTime)>60000 || first) 
  {
      first=false;
      previousTime=millis();
      status=pressure.startTemperature();
      if (status!=0)
      {
        waitFor(status);
        status = pressure.getTemperature(T);
        if (status != 0)
        {
          // Print out the measurement:
          Serial.print(F("temperature: "));
          Serial.print(T,2);
          Serial.print(F(" deg C, "));
          
          // Start a pressure measurement:
          // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
          // If request is successful, the number of ms to wait is returned.
          // If request is unsuccessful, 0 is returned.
          status = pressure.startPressure(3);
          if (status != 0)
          {
            // Wait for the measurement to complete:
            waitFor(status);
    
            // Retrieve the completed pressure measurement:
            // Note that the measurement is stored in the variable P.
            // Note also that the function requires the previous temperature measurement (T).
            // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
            // Function returns 1 if successful, 0 if failure.
    
            status = pressure.getPressure(P,T);
            if (status != 0)
            {
              // Print out the measurement:
              Serial.print(F("absolute pressure: "));
              Serial.print(P,2);
              Serial.print(F(" mb, "));
    
              // The pressure sensor returns abolute pressure, which varies with altitude.
              // To remove the effects of altitude, use the sealevel function and your current altitude.
              // This number is commonly used in weather reports.
              // Parameters: P = absolute pressure in mb, ALTITUDE = current altitude in m.
              // Result: p0 = sea-level compensated pressure in mb
              p0 = pressure.sealevel(P,ALTITUDE); // we're at home altitude in meters
              Serial.print(F("relative (sea-level) pressure: "));
              Serial.print(p0,2);
              Serial.println(F(" mb, "));
            }
            else Serial.println(F("error retrieving pressure measurement\n"));
          }
          else Serial.println(F("error starting pressure measurement\n"));
        }
        else Serial.println(F("error retrieving temperature measurement\n"));
      }
      else Serial.println(F("error starting temperature measurement\n"));
      
      if ((millis()-previouSend )>1800000 || firstSend) {
          firstSend=false;
          previouSend=millis();
          sendData("AT+CIPSTART=\"TCP\",\"184.106.153.149\",80\r\n",2000,DEBUG); // turn on server on port 80
          String webPage="GET /update?api_key=URRDEEMCRKTWTKA0&field1=";
          webPage += T;
          webPage += "&field2=";
          webPage += p0;
          webPage += "\r\n";
          String cipSend="AT+CIPSEND=";
          cipSend+=webPage.length();
          cipSend+="\r\n";
          sendData(cipSend,2000,DEBUG);
          if (esp8266.find("ERROR")) {
            Serial.println(F("Erreur dans l'envoi du paquet!"));
          } else {
            sendData(webPage,2000,DEBUG);
          }
      }
      //Display T & p0
      affichage(T,p0);
      
  }
}
 
 
String sendData(String command, const int timeout, boolean debug)
{
    String response = "";
    
    esp8266.print(command); // send the read character to the esp8266
    
    long int time = millis();
    
    while( (time+timeout) > millis())
    {
      while(esp8266.available())
      {
        
        // The esp has data so display its output to the serial window 
        char c = esp8266.read(); // read the next character.
        response+=c;
      }  
    }
    
    if(debug)
    {
      Serial.print(response);
    }
    
    return response;
}

void waitFor(unsigned long time) {
  long currentTime=millis();
  while ((millis()-currentTime)<time) {
    // loop for time in milliseconds
  }
}

void affichage(double T, double P)
{
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(F("MadeInFck Wx Station"));
  display.println(F(" "));
  display.print(F("Temperature:"));
  display.print(T);
  display.println(F("oC"));
  display.print(F("Pression: "));
  display.print(P);
  display.print(F("mBar"));
  display.display();
}
 
