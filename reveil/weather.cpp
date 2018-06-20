
#include <time.h>
#include "weather.h"
#include <Arduino.h>
#include "OpenWeatherMapCurrent.h"

String OPEN_WEATHER_MAP_APP_ID = "c4d6bd9d1729c413ad56eb1c26c08056";
String OPEN_WEATHER_MAP_LOCATION = "Grenoble,FR";
String OPEN_WEATHER_MAP_LANGUAGE = "en";
boolean IS_METRIC = true;

// initiate the client
OpenWeatherMapCurrent client;
OpenWeatherMapCurrentData data;

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

int weather_temperature_get(void)
{
  return (int)data.temp;
}


char *weather_main_get(void)
{
  char main_weather[sizeof(data.main.c_str())] = {};
  strcpy(main_weather,data.main.c_str());
  return main_weather;
}
