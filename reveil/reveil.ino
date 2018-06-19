// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <time.h>
#include "OpenWeatherMapCurrent.h"
#include <JsonListener.h>

#include "mappage_led.h"
#include "font.h"
#include "weather.h"

//intensity
#define LOW_INTENSITY       20
#define MEDIUM_INTENSITY    50
#define MAX_INTENSITY       100

// Which pin  is connected to the LED
#define PIN            D6

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      257

char ssid[] = "";  //  your network SSID (name)
char pass[] = "";       // your network password

unsigned int localPort = 2390;      // local port to listen for UDP packets
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "fr.pool.ntp.org";
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);
void print_cara(Adafruit_NeoPixel *pixels ,uint8_t x, pixel_color color);
int delayval = 500; // delay for half a second

void clear_pixel();
unsigned long sendNTPpacket(IPAddress& address);

bool toggle_hour_weather = false;

void setup() {
  ESP.wdtDisable();
  ESP.wdtEnable(WDTO_8S);

  Serial.begin(115200);
  Serial.println();
  Serial.println();

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());


  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\n", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  pixels.begin(); // This initializes the NeoPixel library..

}

int16_t offset_x = 0;
uint32_t seconde = 0;

void loop() {
  ArduinoOTA.handle();//OTA through Ardyuino IDE
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP);

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
  const unsigned long seventyYears = 2208988800UL;
  // subtract seventy years:
  unsigned long epoch = 0;

  int cb = udp.parsePacket();
  if (!cb) {
    Serial.println("no packet yet");
  }
  else {
    Serial.print("packet received, length=");
    Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = " );
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // subtract seventy years:
    epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);

    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    Serial.print(':');
    if ( ((epoch % 3600) / 60) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ( (epoch % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch % 60); // print the second
  }
  // wait ten seconds before asking for the time again
  ESP.wdtFeed();

  delay(25000);
  if(seconde % 100 == 0)
  {
    //every 1/2 hour
    //update wheather
    get_weather();
  }
  if(toggle_hour_weather)
  {
    clear_pixel();
    print_time(epoch, LOW_INTENSITY);
  }
  else
  {
     //print weather
     Serial.println("Show weather");
     clear_pixel();
     print_weather( epoch);
  }
  toggle_hour_weather = !toggle_hour_weather;
  seconde = seconde + 100;
}

void print_time(unsigned long epoch, char intensity)
{
  //get_time
  //clear_pixel();
  pixel_position pos = {};
  offset_x = 0;
  pixels.clear();
  pixels.show();

  struct pixel_color color = {};
  int x=0;
  int hour = 0;
  int minute = 0;
  hour = (epoch  % 86400L) / 3600+2;
  minute = ((epoch % 3600) / 60);


  offset_x = 0;
  //print dizaine
  x=hour / 10;
  for (int i=0+8*x;i<8+8*x;i++)
  {
     if((font[i] & 0x01<<7) >> 7 == 1)
        pixels.setPixelColor(get_map_num_led({(uint16_t)(0+offset_x),(uint16_t)(i-8*x)}), pixels.Color(intensity,intensity,intensity));
     if((font[i] & 0x40) >> 6 == 1)
        pixels.setPixelColor(get_map_num_led({(uint16_t)(1+offset_x),(uint16_t)(i-8*x)}), pixels.Color(intensity,intensity,intensity));
     if((font[i] & 0x20)>>5 == 1)
        pixels.setPixelColor(get_map_num_led({(uint16_t)(2+offset_x),(uint16_t)(i-8*x)}), pixels.Color(intensity,intensity,intensity));
     if((font[i] & 0x10)>>4 == 1)
        pixels.setPixelColor(get_map_num_led({(uint16_t)(3+offset_x),(uint16_t)(i-8*x)}), pixels.Color(intensity,intensity,intensity));
    //pixels.show();
   }
  offset_x = 5;
  x=hour % 10;
  Serial.print("hour =");
  Serial.println(x);
  for (int i=0+8*x;i<8+8*x;i++)
  {
     if((font[i] & 0x01<<7) >> 7 == 1)
        pixels.setPixelColor(get_map_num_led({(uint16_t)(0+offset_x),(uint16_t)(i-8*x)}), pixels.Color(intensity,intensity,intensity));
     if((font[i] & 0x40) >> 6 == 1)
        pixels.setPixelColor(get_map_num_led({(uint16_t)(1+offset_x),(uint16_t)(i-8*x)}), pixels.Color(intensity,intensity,intensity));
     if((font[i] & 0x20)>>5 == 1)
        pixels.setPixelColor(get_map_num_led({(uint16_t)(2+offset_x),(uint16_t)(i-8*x)}), pixels.Color(intensity,intensity,intensity));
     if((font[i] & 0x10)>>4 == 1)
        pixels.setPixelColor(get_map_num_led({(uint16_t)(3+offset_x),(uint16_t)(i-8*x)}), pixels.Color(intensity,intensity,intensity));
  //pixels.show();
  }
  offset_x = 10;

  //print double point
  pixels.setPixelColor(get_map_num_led({(uint16_t)(0+offset_x),(uint16_t)3}), pixels.Color(intensity,intensity,intensity));
  pixels.setPixelColor(get_map_num_led({(uint16_t)(0+offset_x),(uint16_t)5}), pixels.Color(intensity,intensity,intensity));
  //pixels.show();
  offset_x = 12;
  //print dizaine
  x=minute / 10;
  Serial.print("minutes =");
  Serial.println(x);
  for (char i=0+x*8;i<8+x*8;i++)
  {
     if((font[i] & 0x01<<7) >> 7 == 1)
        pixels.setPixelColor(get_map_num_led({(uint16_t)(0+offset_x),(uint16_t)(i-8*x)}), pixels.Color(intensity,intensity,intensity));
     if((font[i] & 0x40) >> 6 == 1)
        pixels.setPixelColor(get_map_num_led({(uint16_t)(1+offset_x),(uint16_t)(i-8*x)}), pixels.Color(intensity,intensity,intensity));
     if((font[i] & 0x20)>>5 == 1)
        pixels.setPixelColor(get_map_num_led({(uint16_t)(2+offset_x),(uint16_t)(i-8*x)}), pixels.Color(intensity,intensity,intensity));
     if((font[i] & 0x10)>>4 == 1)
        pixels.setPixelColor(get_map_num_led({(uint16_t)(3+offset_x),(uint16_t)(i-8*x)}), pixels.Color(intensity,intensity,intensity));
    pixels.show();
   }
  offset_x = 17;
  x=minute % 10;
  Serial.print("minutes =");
  Serial.println(x);

  for (int i=0+x*8;i<8+x*8;i++)
  {
     if((font[i] & 0x01<<7) >> 7 == 1)
        pixels.setPixelColor(get_map_num_led({(uint16_t)(0+offset_x),(uint16_t)(i-8*x)}), pixels.Color(intensity,intensity,intensity));
     if((font[i] & 0x40) >> 6 == 1)
        pixels.setPixelColor(get_map_num_led({(uint16_t)(1+offset_x),(uint16_t)(i-8*x)}), pixels.Color(intensity,intensity,intensity));
     if((font[i] & 0x20)>>5 == 1)
        pixels.setPixelColor(get_map_num_led({(uint16_t)(2+offset_x),(uint16_t)(i-8*x)}), pixels.Color(intensity,intensity,intensity));
     if((font[i] & 0x10)>>4 == 1)
        pixels.setPixelColor(get_map_num_led({(uint16_t)(3+offset_x),(uint16_t)(i-8*x)}), pixels.Color(intensity,intensity,intensity));
   }

    offset_x = 24;
    Serial.printf("main: %s\n",weather_main_get());
    if(strcmp(weather_main_get(),"Clouds") == 0)
    {
       x=13;
       color.red=20;color.green=20;color.blue=20;
      print_cara(&pixels,x,color);
    }
     if(strcmp(weather_main_get(),"Clear") == 0)
     {
        color.red=50;color.green=50;color.blue=0;
        x=11;
        if(hour < 21 && hour > 7)
          print_cara(&pixels,11,color);//print sun
        else
        {
          color.red=15;color.green=20;color.blue=20;
          print_cara(&pixels,10,color);//print moon
        }
     }
     if(strcmp(weather_main_get(),"Snow") == 0)
     {
        x=45;
        color.red=0;color.green=50;color.blue=50;
        print_cara(&pixels,x,color);
     }
     if(strcmp(weather_main_get(),"Rain") == 0)
     {
        x=44;
        color.red=0;color.green=38;color.blue=70;
        print_cara(&pixels,x,color);
     }
     if(strcmp(weather_main_get(),"Thunderstorm") == 0)
     {
        x=43;
        print_cara(&pixels,x,color);
     }
   pixels.show();
   //delay(500);

}


void print_weather(unsigned long epoch)
{
    pixels.clear();
    pixels.show();
    uint8_t temperature =weather_temperature_get();
    uint8_t hour = (epoch  % 86400L) / 3600+2;
    uint8_t x = 0;
    offset_x = 0;
    struct pixel_color color = {};
    char intensity = 0;
    if (temperature > 20){
      color.red=51;color.green=20;color.blue=0;
    }
    else if(temperature > 10){
      color.red=30;color.green=30;color.blue=0;
    }
    else if(temperature > 0){
      color.red=0;color.green=20;color.blue=40;
    }
    else if(temperature < 0){
      color.red=0; color.green=10;color.blue=40;
    }
    x=temperature / 10;
    for (int i=0+8*x;i<8+8*x;i++)
    {
     if((font[i] & 0x01<<7) >> 7 == 1)
        pixels.setPixelColor(get_map_num_led({(uint16_t)(0+offset_x),(uint16_t)(i-8*x)}), pixels.Color(color.green,color.red,color.blue));
     if((font[i] & 0x40) >> 6 == 1)
        pixels.setPixelColor(get_map_num_led({(uint16_t)(1+offset_x),(uint16_t)(i-8*x)}), pixels.Color(color.green,color.red,color.blue));
     if((font[i] & 0x20)>>5 == 1)
        pixels.setPixelColor(get_map_num_led({(uint16_t)(2+offset_x),(uint16_t)(i-8*x)}), pixels.Color(color.green,color.red,color.blue));
     if((font[i] & 0x10)>>4 == 1)
        pixels.setPixelColor(get_map_num_led({(uint16_t)(3+offset_x),(uint16_t)(i-8*x)}), pixels.Color(color.green,color.red,color.blue));
    }
    offset_x = 5;
    x = temperature % 10;
    for (int i=0+8*x;i<8+8*x;i++)
    {
     if((font[i] & 0x01<<7) >> 7 == 1)
        pixels.setPixelColor(get_map_num_led({(uint16_t)(0+offset_x),(uint16_t)(i-8*x)}), pixels.Color(color.green,color.red,color.blue));
     if((font[i] & 0x40) >> 6 == 1)
        pixels.setPixelColor(get_map_num_led({(uint16_t)(1+offset_x),(uint16_t)(i-8*x)}), pixels.Color(color.green,color.red,color.blue));
     if((font[i] & 0x20)>>5 == 1)
        pixels.setPixelColor(get_map_num_led({(uint16_t)(2+offset_x),(uint16_t)(i-8*x)}), pixels.Color(color.green,color.red,color.blue));
     if((font[i] & 0x10)>>4 == 1)
        pixels.setPixelColor(get_map_num_led({(uint16_t)(3+offset_x),(uint16_t)(i-8*x)}), pixels.Color(color.green,color.red,color.blue));
    }
    offset_x = 9;
    //print °C
    x=12;
    print_cara(&pixels,x,color);

    offset_x = 19;
    Serial.printf("main: %s\n", weather_main_get());
    if(strcmp(weather_main_get(),"Clouds") == 0)
    {
       x=13;
       color.red=20;
       color.green=20;
       color.blue=20;
      print_cara(&pixels,x,color);
    }
     if(strcmp(weather_main_get(),"Clear") == 0)
     {
        color.red=50;
        color.green=50;
        color.blue=0;
        x=11;
        if(hour < 20 && hour > 7)
          print_cara(&pixels,x,color);//print sun
        else
        {
          x=10;
          color.red=15;
          color.green=20;
          color.blue=20;
          print_cara(&pixels,x,color);//print moon
        }
     }
     if(strcmp(weather_main_get(),"Snow") == 0)
     {
        x=45;
        color.red=0;
        color.green=50;
        color.blue=50;
        print_cara(&pixels,x,color);
     }
     if(strcmp(weather_main_get(),"Rain") == 0)
     {
        color.red=0;
        color.green=38;
        color.blue=70;
        x=44;
        print_cara(&pixels,x,color);
     }
     if(strcmp(weather_main_get(),"Thunderstorm") == 0)
     {
        x=43;
        print_cara(&pixels,x,color);
     }

    pixels.show();
}

void print_cara(Adafruit_NeoPixel *pixels ,uint8_t x, pixel_color color)
{
  for (int i=0+8*x;i<8+8*x;i++)
  {
    if((font[i] & 0x01<<7) >> 7 == 1)
      pixels->setPixelColor(get_map_num_led({(uint16_t)(0+offset_x),(uint16_t)(i-8*x)}), pixels->Color(color.green,color.red,color.blue));
    if((font[i] & 0x40) >> 6 == 1)
      pixels->setPixelColor(get_map_num_led({(uint16_t)(1+offset_x),(uint16_t)(i-8*x)}), pixels->Color(color.green,color.red,color.blue));
    if((font[i] & 0x20)>>5 == 1)
      pixels->setPixelColor(get_map_num_led({(uint16_t)(2+offset_x),(uint16_t)(i-8*x)}), pixels->Color(color.green,color.red,color.blue));
    if((font[i] & 0x10)>>4 == 1)
      pixels->setPixelColor(get_map_num_led({(uint16_t)(3+offset_x),(uint16_t)(i-8*x)}), pixels->Color(color.green,color.red,color.blue));
    if((font[i] & 0x08) >> 3 == 1)
      pixels->setPixelColor(get_map_num_led({(uint16_t)(4+offset_x),(uint16_t)(i-8*x)}), pixels->Color(color.green,color.red,color.blue));
    if((font[i] & 0x04) >> 2 == 1)
      pixels->setPixelColor(get_map_num_led({(uint16_t)(5+offset_x),(uint16_t)(i-8*x)}), pixels->Color(color.green,color.red,color.blue));
    if((font[i] & 0x02)>>1 == 1)
      pixels->setPixelColor(get_map_num_led({(uint16_t)(6+offset_x),(uint16_t)(i-8*x)}), pixels->Color(color.green,color.red,color.blue));
    if((font[i] & 0x01)>>0 == 1)
      pixels->setPixelColor(get_map_num_led({(uint16_t)(7+offset_x),(uint16_t)(i-8*x)}), pixels->Color(color.green,color.red,color.blue));
   }

}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}


void clear_pixel()
{
  for(int i=0;i<NUMPIXELS+1;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0,0,0)); // Moderately bright green color.
  }
  pixels.show(); // This sends the updated pixel color to the hardware.
}
