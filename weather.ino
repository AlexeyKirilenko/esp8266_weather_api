#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Adafruit_GFX.h>       //Libraries required to use the Display
#include <Adafruit_ST7735.h>  ////Libraries required to use the Display
#include <SPI.h>
#include <NTPClient.h>        // include NTPClient library
#include <TimeLib.h>          // Include Arduino time library
#include <ArduinoJson.h>



WiFiClient wifiClient;

//pins

// -----------  Display pins  --------------------------------------
#define TFT_CS         15   //
#define TFT_RST        0   //
#define TFT_DC         2   // my pins - you shoud use own

//wi-fi

const char* ssid = "%networkname_here%";              //Your router SSID and password                             
const char* password =  "password_here";    //Your router SSID and password
//get api data
const String server  = "api.openweathermap.org";
const String lat     = "59.836819";
const String lon     = "30.311276";
const String appid   = "your_API_KEY here";
const String tail = "&units=metric&lang=en"; // you can choose metrics and lang,read more about that here: https://openweathermap.org/current
const String url     = "http://" + server + "/data/2.5/weather?lat=" + lat + "&lon=" + lon + "&units=metric&appid=" + appid + tail;
const int port = 443;  // HTTPS port
const int httpPortRead = 443; 
const String host = "api.openweathermap.org";

int To_remove;      //There are some irrelevant data on the string and here's how I keep the index
                    //of those characters 
                    //display:
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST); //Those things are for the display
float p = 3.1415926;



unsigned long lastConnectionTime = 0;
unsigned long postingInterval = 0;

String httpData;

struct weather_structure {
  unsigned int id;
  const char* main;
  const char* icon;
  const char* descript;
  float temp;
  float pressure;
  byte  humidity;
  float speed;
  float deg;
};
weather_structure weather;



void setup() {
  Serial.begin(9600);
  tft.initR(INITR_GREENTAB);  
  WiFi.disconnect();             //Disconnect and reconnect to the Wifi you set                                                 
  delay(5000);   
  Serial.print("\nConnecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (millis() < lastConnectionTime) lastConnectionTime = 0;
    if (millis() - lastConnectionTime > postingInterval or lastConnectionTime == 0) {
      if (httpRequest() and parseData()) {
      //  Serial.println("\nWeather");
      //  Serial.printf("id: %d\n", weather.id);
      //  Serial.printf("main: %s\n", weather.main);
        Serial.printf("description: %s\n", weather.descript);
      //  Serial.printf("icon: %s\n", weather.icon);
        Serial.printf("temp: %d c\n", round(weather.temp));
        Serial.printf("humidity: %d %\n", round(weather.humidity));
       // Serial.printf("pressure: %d hPa or %d mmHg\n", round(weather.pressure), round(weather.pressure * 0.75));
       // Serial.printf("wind's speed: %d\n", round(weather.speed));
       // Serial.printf("wind's direction: %d\n", round(weather.deg));
        Serial.println();
        //Print_TFT(); //Display to the TFT LCD, this function will vary depending on your display
        void Print_TFT();
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextWrap(false);
  tft.setCursor(20, 10);    //Horiz/Vertic
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_GREEN);
  tft.print("weather.main"); 

  tft.setCursor(15, 30);    //Horiz/Vertic
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("weather.descript");

  tft.setCursor(0, 55);    //Horiz/Vertic
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_BLUE);
  tft.print("temp:");
  tft.setCursor(0, 80);
  tft.print(weather.temp);  

  tft.setCursor(0, 115);    //Horiz/Vertic
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_RED);
  tft.print("humidity:");
  tft.setCursor(5, 115);
  tft.setTextColor(ST77XX_RED);
  tft.print(weather.humidity); //
 delay(5000);        
                  }
}
  }
}
      
   
  


      
bool httpRequest() {
  HTTPClient client;
  bool find = false;
  //client.setTimeout(1800000);
  Serial.print("Connecting ");
  client.begin(wifiClient,url);
  //client,host,httpPortRead
  int httpCode = client.GET();

  if (httpCode > 0) {
    Serial.printf("successfully, code: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK) {
      httpData = client.getString();
      if (httpData.indexOf(F("\"main\":{\"temp\":")) > -1) {
        lastConnectionTime = millis();
        find = true;
      }
      else Serial.println("Failed, json string is not found");
    }
  }
  else Serial.printf("failed, error: %s\n", client.errorToString(httpCode).c_str());

  postingInterval = find ? 600L * 1000L : 60L * 1000L;
  client.end();

  return find;
}

bool parseData() {
  Serial.println(httpData);

DynamicJsonBuffer jsonBuffer;
JsonObject& root = jsonBuffer.parseObject(httpData);

  if (!root.success()) {
    Serial.println("Json parsing failed!");
    return false;
  }

  weather.id       = root["weather"][0]["id"];
  weather.main     = root["weather"][0]["main"];
  weather.descript = root["weather"][0]["description"];
  weather.icon     = root["weather"][0]["icon"];
  weather.temp     = root["main"]["temp"];
  weather.humidity = root["main"]["humidity"];
  weather.pressure = root["main"]["pressure"];
  weather.speed    = root["wind"]["speed"];
  weather.deg      = root["wind"]["deg"];

  httpData = "";
  return true;
}