#include <DHT.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define DHTPIN 5
#define DHTTYPE DHT11

const char* ssid = "HUAWEI-HNXR-2G";
const char* password = "SdTNPdbN";

WiFiClient wifiClient;
HTTPClient httpClient;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
    
String serverUrl = "http://192.168.100.68:5199/api/SensorData";

DHT dht(DHTPIN, DHTTYPE);

const long sensorInterval = 10000;
long previousMillis = 0;

void setup() {
  Serial.begin(9600);
  dht.begin();

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  timeClient.begin();
  timeClient.setTimeOffset(3600);
}

void loop() {
  long currentMillis = millis();
  if(currentMillis - previousMillis >= sensorInterval) {
    previousMillis = currentMillis;

    timeClient.update(); 

    float temperature = dht.readTemperature(); 
    if (isnan(temperature)) {
      Serial.println("Failed to read temperature from DHT sensor!");
    }
    else {
      Serial.println("Temperature: " + String(temperature));
    }

    float humidity = dht.readHumidity(); 
    if (isnan(humidity)) {
      Serial.println("Failed to read humidity from DHT sensor!");
    }
    else {
      Serial.println("Humidity: " + String(humidity));
    }

    String data = "{\"temperature\": " + String(temperature) + ", \"humidity\": " + String(humidity) + ", \"date\": " + "\"" + getFullFormattedTime() + "\"" +"}";
    Serial.println(data);
    httpClient.begin(wifiClient, serverUrl);
    httpClient.addHeader("Content-Type", "application/json");
    int responseCode = httpClient.POST(data);
    String response = httpClient.getString();
    httpClient.end();

    Serial.println("Server response code: " + String(responseCode));
    Serial.println("Server response: " + response);

    Serial.println("-------------------------");
  }
}

String getFullFormattedTime() {
   time_t rawtime = timeClient.getEpochTime();
   struct tm * ti;
   ti = localtime(&rawtime);

   uint16_t year = ti->tm_year + 1900;
   String yearStr = String(year);

   uint8_t month = ti->tm_mon + 1;
   String monthStr = month < 10 ? "0" + String(month) : String(month);

   uint8_t day = ti->tm_mday;
   String dayStr = day < 10 ? "0" + String(day) : String(day);

   uint8_t hours = ti->tm_hour;
   String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

   uint8_t minutes = ti->tm_min;
   String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

   uint8_t seconds = ti->tm_sec;
   String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

   return yearStr + "-" + monthStr + "-" + dayStr + "T" +
          hoursStr + ":" + minuteStr + ":" + secondStr;
} 
