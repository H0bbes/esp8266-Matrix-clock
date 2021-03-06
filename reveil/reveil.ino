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

//intensity
#define LOW_INTENSITY       20
#define MEDIUM_INTENSITY    50
#define MAX_INTENSITY       100



// initiate the client
OpenWeatherMapCurrent client;

String OPEN_WEATHER_MAP_APP_ID = "c4d6bd9d1729c413ad56eb1c26c08056";
String OPEN_WEATHER_MAP_LOCATION = "Grenoble,FR";
String OPEN_WEATHER_MAP_LANGUAGE = "en";
boolean IS_METRIC = true;

OpenWeatherMapCurrentData data;

  
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

char offset_x = 0;
char count = 0;
uint32_t seconde = 0;

void loop() {
  ArduinoOTA.handle();
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

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

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

  
  //get_weather();
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
        pixels.setPixelColor(get_map_num_led({0+offset_x,i-8*x}), pixels.Color(intensity,intensity,intensity));
     if((font[i] & 0x40) >> 6 == 1)
        pixels.setPixelColor(get_map_num_led({1+offset_x,i-8*x}), pixels.Color(intensity,intensity,intensity));
     if((font[i] & 0x20)>>5 == 1)
        pixels.setPixelColor(get_map_num_led({2+offset_x,i-8*x}), pixels.Color(intensity,intensity,intensity));
     if((font[i] & 0x10)>>4 == 1)
        pixels.setPixelColor(get_map_num_led({3+offset_x,i-8*x}), pixels.Color(intensity,intensity,intensity)); 
    //pixels.show();
   }
  offset_x = 5;
  x=hour % 10;
  Serial.print("hour =");
  Serial.println(x);
  for (int i=0+8*x;i<8+8*x;i++)
  { 
     if((font[i] & 0x01<<7) >> 7 == 1)
        pixels.setPixelColor(get_map_num_led({0+offset_x,i-8*x}), pixels.Color(intensity,intensity,intensity));
     if((font[i] & 0x40) >> 6 == 1)
        pixels.setPixelColor(get_map_num_led({1+offset_x,i-8*x}), pixels.Color(intensity,intensity,intensity));
     if((font[i] & 0x20)>>5 == 1)
        pixels.setPixelColor(get_map_num_led({2+offset_x,i-8*x}), pixels.Color(intensity,intensity,intensity));
     if((font[i] & 0x10)>>4 == 1)
        pixels.setPixelColor(get_map_num_led({3+offset_x,i-8*x}), pixels.Color(intensity,intensity,intensity)); 
  //pixels.show();
  }
  offset_x = 10;

  //print double point
  pixels.setPixelColor(get_map_num_led({0+offset_x,3}), pixels.Color(intensity,intensity,intensity));
  pixels.setPixelColor(get_map_num_led({0+offset_x,5}), pixels.Color(intensity,intensity,intensity));
  //pixels.show();
  offset_x = 12;
  //print dizaine
  x=minute / 10;
  Serial.print("minutes =");
  Serial.println(x);
  for (char i=0+x*8;i<8+x*8;i++)
  { 
     if((font[i] & 0x01<<7) >> 7 == 1)
        pixels.setPixelColor(get_map_num_led({0+offset_x,i-8*x}), pixels.Color(intensity,intensity,intensity));
     if((font[i] & 0x40) >> 6 == 1)
        pixels.setPixelColor(get_map_num_led({1+offset_x,i-8*x}), pixels.Color(intensity,intensity,intensity));
     if((font[i] & 0x20)>>5 == 1)
        pixels.setPixelColor(get_map_num_led({2+offset_x,i-8*x}), pixels.Color(intensity,intensity,intensity));
     if((font[i] & 0x10)>>4 == 1)
        pixels.setPixelColor(get_map_num_led({3+offset_x,i-8*x}), pixels.Color(intensity,intensity,intensity)); 
    pixels.show();
   }
  offset_x = 17;
  x=minute % 10;
  Serial.print("minutes =");
  Serial.println(x);
    
  for (int i=0+x*8;i<8+x*8;i++)
  { 
     if((font[i] & 0x01<<7) >> 7 == 1)
        pixels.setPixelColor(get_map_num_led({0+offset_x,i-8*x}), pixels.Color(intensity,intensity,intensity));
     if((font[i] & 0x40) >> 6 == 1)
        pixels.setPixelColor(get_map_num_led({1+offset_x,i-8*x}), pixels.Color(intensity,intensity,intensity));
     if((font[i] & 0x20)>>5 == 1)
        pixels.setPixelColor(get_map_num_led({2+offset_x,i-8*x}), pixels.Color(intensity,intensity,intensity));
     if((font[i] & 0x10)>>4 == 1)
        pixels.setPixelColor(get_map_num_led({3+offset_x,i-8*x}), pixels.Color(intensity,intensity,intensity)); 
   }
   
    offset_x = 24;
    Serial.printf("main: %s\n", data.main.c_str());
    if(strcmp(data.main.c_str(),"Clouds") == 0)
    {
       x=13;
       color.red=20;
       color.green=20;
       color.blue=20;
      print_cara(x,color);
    }
     if(strcmp(data.main.c_str(),"Clear") == 0)
     {
        color.red=50;
        color.green=50;
        color.blue=0;
        x=11;
        if(hour < 20 && hour > 7)
          print_cara(11,color);//print sun
        else
        {
          color.red=15;
          color.green=20;
          color.blue=20;
          print_cara(10,color);//print moon
        }
     }
     if(strcmp(data.main.c_str(),"Snow") == 0)
     {
        x=45;
        color.red=0;
        color.green=50;
        color.blue=50;
        print_cara(x,color);
     }
     if(strcmp(data.main.c_str(),"Rain") == 0)
     {
        color.red=0;
        color.green=38;
        color.blue=70;
        x=44;
        print_cara(x,color);
     }
     if(strcmp(data.main.c_str(),"Thunderstorm") == 0)
     {
        x=43;
        print_cara(x,color);
     }
   pixels.show();
   //delay(500);
   
}

uint8_t temperature = 0;
void print_weather(unsigned long epoch)
{
    pixels.clear();
    pixels.show();
    temperature = (int)data.temp;
    uint8_t hour = (epoch  % 86400L) / 3600+2;
    uint8_t x = 0;
    offset_x = 0;
    struct pixel_color color = {};
    char intensity = 0;
    if (temperature > 20){
      color.red=51;
      color.green=20;
      color.blue=0;
    }
    else if(temperature > 10){
      color.red=30;
      color.green=30;
      color.blue=0;
    }
    else if(temperature > 0){
      color.red=0;
      color.green=20;
      color.blue=40;
    }
    else if(temperature < 0){
      color.red=0;
      color.green=10;
      color.blue=40;
    }
    x=temperature / 10;
    for (int i=0+8*x;i<8+8*x;i++)
    { 
     if((font[i] & 0x01<<7) >> 7 == 1)
        pixels.setPixelColor(get_map_num_led({0+offset_x,i-8*x}), pixels.Color(color.green,color.red,color.blue));
     if((font[i] & 0x40) >> 6 == 1)
        pixels.setPixelColor(get_map_num_led({1+offset_x,i-8*x}), pixels.Color(color.green,color.red,color.blue));
     if((font[i] & 0x20)>>5 == 1)
        pixels.setPixelColor(get_map_num_led({2+offset_x,i-8*x}), pixels.Color(color.green,color.red,color.blue));
     if((font[i] & 0x10)>>4 == 1)
        pixels.setPixelColor(get_map_num_led({3+offset_x,i-8*x}), pixels.Color(color.green,color.red,color.blue));       
    }
    offset_x = 5;
    x = temperature % 10;
    for (int i=0+8*x;i<8+8*x;i++)
    { 
     if((font[i] & 0x01<<7) >> 7 == 1)
        pixels.setPixelColor(get_map_num_led({0+offset_x,i-8*x}), pixels.Color(color.green,color.red,color.blue));
     if((font[i] & 0x40) >> 6 == 1)
        pixels.setPixelColor(get_map_num_led({1+offset_x,i-8*x}), pixels.Color(color.green,color.red,color.blue));
     if((font[i] & 0x20)>>5 == 1)
        pixels.setPixelColor(get_map_num_led({2+offset_x,i-8*x}), pixels.Color(color.green,color.red,color.blue));
     if((font[i] & 0x10)>>4 == 1)
        pixels.setPixelColor(get_map_num_led({3+offset_x,i-8*x}), pixels.Color(color.green,color.red,color.blue)); 
    }
    offset_x = 9;
    //print °C
    x=12;
    print_cara(x,color);

    offset_x = 19;
    Serial.printf("main: %s\n", data.main.c_str());
    if(strcmp(data.main.c_str(),"Clouds") == 0)
    {
       x=13;
       color.red=20;
       color.green=20;
       color.blue=20;
      print_cara(x,color);
    }
     if(strcmp(data.main.c_str(),"Clear") == 0)
     {
        color.red=50;
        color.green=50;
        color.blue=0;
        x=11;
        if(hour < 20 && hour > 7)
          print_cara(11,color);//print sun
        else
        {
          color.red=15;
          color.green=20;
          color.blue=20;
          print_cara(10,color);//print moon
        }
     }
     if(strcmp(data.main.c_str(),"Snow") == 0)
     {
        x=45;
        color.red=0;
        color.green=50;
        color.blue=50;
        print_cara(x,color);
     }
     if(strcmp(data.main.c_str(),"Rain") == 0)
     {
        color.red=0;
        color.green=38;
        color.blue=70;
        x=44;
        print_cara(x,color);
     }
     if(strcmp(data.main.c_str(),"Thunderstorm") == 0)
     {
        x=43;
        print_cara(x,color);
     }
      
    pixels.show();
}

void print_cara(uint8_t x, pixel_color color)
{
  for (int i=0+8*x;i<8+8*x;i++)
  { 
    if((font[i] & 0x01<<7) >> 7 == 1)
      pixels.setPixelColor(get_map_num_led({0+offset_x,i-8*x}), pixels.Color(color.green,color.red,color.blue));
    if((font[i] & 0x40) >> 6 == 1)
      pixels.setPixelColor(get_map_num_led({1+offset_x,i-8*x}), pixels.Color(color.green,color.red,color.blue));
    if((font[i] & 0x20)>>5 == 1)
      pixels.setPixelColor(get_map_num_led({2+offset_x,i-8*x}), pixels.Color(color.green,color.red,color.blue));
    if((font[i] & 0x10)>>4 == 1)
      pixels.setPixelColor(get_map_num_led({3+offset_x,i-8*x}), pixels.Color(color.green,color.red,color.blue)); 
    if((font[i] & 0x08) >> 3 == 1)
      pixels.setPixelColor(get_map_num_led({4+offset_x,i-8*x}), pixels.Color(color.green,color.red,color.blue));
    if((font[i] & 0x04) >> 2 == 1)
      pixels.setPixelColor(get_map_num_led({5+offset_x,i-8*x}), pixels.Color(color.green,color.red,color.blue));
    if((font[i] & 0x02)>>1 == 1)
      pixels.setPixelColor(get_map_num_led({6+offset_x,i-8*x}), pixels.Color(color.green,color.red,color.blue));
    if((font[i] & 0x01)>>0 == 1)
      pixels.setPixelColor(get_map_num_led({7+offset_x,i-8*x}), pixels.Color(color.green,color.red,color.blue)); 
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


void get_weather()
{
  
  Serial.println();
  Serial.println("\n\nNext Loop-Step: " + String(millis()) + ":");


  client.setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
  client.setMetric(IS_METRIC);
  client.updateCurrent(&data, OPEN_WEATHER_MAP_APP_ID, OPEN_WEATHER_MAP_LOCATION);

  Serial.println("------------------------------------");

  // "lon": 8.54, float lon;
  Serial.printf("lon: %f\n", data.lon);
  // "lat": 47.37 float lat;
  Serial.printf("lat: %f\n", data.lat);
  // "id": 521, weatherId weatherId;
  Serial.printf("weatherId: %d\n", data.weatherId);
  // "main": "Rain", String main;
  Serial.printf("main: %s\n", data.main.c_str());
  // "description": "shower rain", String description;
  Serial.printf("description: %s\n", data.description.c_str());
  // "icon": "09d" String icon; String iconMeteoCon;
  Serial.printf("icon: %s\n", data.icon.c_str());
  Serial.printf("iconMeteoCon: %s\n", data.iconMeteoCon.c_str());
  // "temp": 290.56, float temp;
  Serial.printf("temp: %d\n", (int)data.temp);
  // "pressure": 1013, uint16_t pressure;
  Serial.printf("pressure: %d\n", data.pressure);
  // "humidity": 87, uint8_t humidity;
  Serial.printf("humidity: %d\n", data.humidity);
  // "temp_min": 289.15, float tempMin;
  Serial.printf("tempMin: %d\n",(int) data.tempMin);
  // "temp_max": 292.15 float tempMax;
  Serial.printf("tempMax: %d\n", (int)data.tempMax);
  // "wind": {"speed": 1.5}, float windSpeed;
  Serial.printf("windSpeed: %d\n",(int) data.windSpeed);
  // "wind": {"deg": 1.5}, float windDeg;
  Serial.printf("windDeg: %d\n", (int)data.windDeg);
  // "clouds": {"all": 90}, uint8_t clouds;
  Serial.printf("clouds: %d\n", data.clouds);
  // "dt": 1527015000, uint64_t observationTime;
  time_t time = data.observationTime;
  Serial.printf("observationTime: %d, full date: %s", data.observationTime, ctime(&time));
  // "country": "CH", String country;
  Serial.printf("country: %s\n", data.country.c_str());
  // "sunrise": 1526960448, uint32_t sunrise;
  time = data.sunrise;
  Serial.printf("sunrise: %d, full date: %s", data.sunrise, ctime(&time));
  // "sunset": 1527015901 uint32_t sunset;
  time = data.sunset;
  Serial.printf("sunset: %d, full date: %s", data.sunset, ctime(&time));

  // "name": "Paris", String cityName;
  Serial.printf("cityName: %s\n", data.cityName.c_str());
  Serial.println();
  Serial.println("---------------------------------------------------/\n");
  
}

void clear_pixel()
{  
  for(int i=0;i<NUMPIXELS+1;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0,0,0)); // Moderately bright green color.
  }
  pixels.show(); // This sends the updated pixel color to the hardware.
}


