# Temperature_Thingspeak
Monitor home temperature using ESP8266 Wifi module

#30/12/15:
After struggling a while to use my ESP8266, I finally managed to connect it as a simple TCP link.
Discovered about http://Thingspeak.com and created a public channel to monitor my home temperature.
So far, only the arduino board pushed only a raw value as it is a simple test.

#01/01/16 :
Designed a new circuit to get temperature and pressure to be followed via this public channel.
OLED screen 128x64 added to monitor Temperature and pressure @ home (refresh every minute).
Thingspeak data is being refreshed every 30 minutes.

Check my blog : http://www.madeinfck.com

#13/06/16 :  
Got rid off the breadboard and soldered this scheme.
Don't forget to configure your ESP8266 with your network.
Next step : packaging using 3D printer after designing a fitting case (with *Blender* probably).
