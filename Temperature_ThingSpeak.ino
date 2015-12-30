#include <SoftwareSerial.h>
 
#define DEBUG true
 
SoftwareSerial esp8266(10,11); // make RX Arduino line is pin 2, make TX Arduino line is pin 3.
                             // This means that you need to connect the TX line from the esp to the Arduino's pin 2
                             // and the RX line from the esp to the Arduino's pin 3
void setup()
{
  Serial.begin(9600);
  esp8266.begin(9600); // your esp's baud rate might be different
  Serial.println("Initialisation du module Wifi!");
  sendData("AT+RST\r\n",80000,DEBUG); // reset module
  sendData("AT+CWMODE=1\r\n",1000,DEBUG); // configure as STA point
  sendData("AT+CIFSR\r\n",1000,DEBUG); // get ip address
  sendData("AT+CIPMUX=0\r\n",1000,DEBUG); // configure for multiple connections
}
 
void loop()
{
  sendData("AT+CIPSTART=\"TCP\",\"184.106.153.149\",80\r\n",1000,DEBUG); // turn on server on port 80
  String webPage="GET /update?api_key=URRDEEMCRKTWTKA0&field1=40\r\n";
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
 
