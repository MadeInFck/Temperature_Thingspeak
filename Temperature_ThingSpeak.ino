#include <SoftwareSerial.h>
#include <SFE_BMP180.h>
#include <Wire.h>
 
#define DEBUG true
#define ALTITUDE 56.0 //Altitude of my home

SFE_BMP180 pressure;
SoftwareSerial esp8266(10,11); // make RX Arduino line is pin 10, make TX Arduino line is pin 10.
                             // This means that you need to connect the TX line from the esp to the Arduino's pin 10
                             // and the RX line from the esp to the Arduino's pin 11.
void setup()
{
  Serial.begin(9600);
  //Init ESP8266.
  esp8266.begin(9600); // your esp's baud rate might be different
  Serial.println("Initialisation du module Wifi!");
  sendData("AT+RST\r\n",80000,DEBUG); // reset module
  sendData("AT+CWMODE=1\r\n",1000,DEBUG); // configure as STA point
  sendData("AT+CIFSR\r\n",1000,DEBUG); // get ip address
  sendData("AT+CIPMUX=0\r\n",1000,DEBUG); // configure for multiple connections

  //Init BMP180
  Serial.println("REBOOT BMP180");
  // Initialize the sensor (it is important to get calibration values stored on the device).
  if (pressure.begin())
    Serial.println("BMP180 init success");
  else
  {
    // Oops, something went wrong, this is usually a connection problem,
    // see the comments at the top of this sketch for the proper connections.
    Serial.println("BMP180 init fail\n\n");
    while(1); // Pause forever.
  }
}
 
void loop()
{
  char status;
  double T,P,p0,a;
  status=pressure.startTemperature();
  if (status!=0)
  {
    waitFor(status);
    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Print out the measurement:
      Serial.print("temperature: ");
      Serial.print(T,2);
      Serial.print(" deg C, ");
      
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
          Serial.print("absolute pressure: ");
          Serial.print(P,2);
          Serial.print(" mb, ");

          // The pressure sensor returns abolute pressure, which varies with altitude.
          // To remove the effects of altitude, use the sealevel function and your current altitude.
          // This number is commonly used in weather reports.
          // Parameters: P = absolute pressure in mb, ALTITUDE = current altitude in m.
          // Result: p0 = sea-level compensated pressure in mb
          p0 = pressure.sealevel(P,ALTITUDE); // we're at home altitude in meters
          Serial.print("relative (sea-level) pressure: ");
          Serial.print(p0,2);
          Serial.print(" mb, ");
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
  waitFor(5000);
  
  sendData("AT+CIPSTART=\"TCP\",\"184.106.153.149\",80\r\n",1000,DEBUG); // turn on server on port 80
  String webPage="GET /update?api_key=URRDEEMCRKTWTKA0&field1=";
  webPage += T;
  webPage += "&field2=";
  webPage += p0;
  webPage += "\r\n";
  String cipSend="AT+CIPSEND=";
  cipSend+=webPage.length();
  cipSend+="\r\n";
  sendData(cipSend,1000,DEBUG);
  if (esp8266.find("ERROR")) {
    Serial.println("Erreur dans l'envoi du paquet!");
  } else {
    sendData(webPage,1000,DEBUG);
  }

  unsigned long temps=millis();
  while ((millis()-temps)<1800000) {
    
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
 
