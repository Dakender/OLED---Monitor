#include <Wire.h>
#include <ArduinoJson.h> // V6!
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

/* Uncomment the initialize the I2C address , uncomment only one, If you get a totally blank screen try the other*/
#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
//#define i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

// INFO TO CHANGE HERE

const char *ssid = "YOUR_WiFi_SSID_HERE"; // Change this to your WiFi SSID
const char *password = "YOUR_WiFi_PASSWORD_HERE"; // Change this to your WiFi password
const String ducoUser = "YOUR_DUINO-COIN_USER_NAME_HERE"; // Change this to your Duino-Coin username

const String ducoReportJsonUrl = "https://server.duinocoin.com/v2/users/" + ducoUser + "?limit=1";
const int run_in_ms = 5000;
float lastAverageHash = 0.0;
float lastTotalHash = 0.0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  display.begin(i2c_Address, true); // Address 0x3C default
  //display.setContrast (0); // dim display

  display.display();
  delay(2000);

  // Clear the buffer.
  display.clearDisplay();

  // draw a single pixel
  display.drawPixel(10, 10, SH110X_WHITE);
  display.display();
  delay(2000);
  display.clearDisplay();

}

void loop() {
  // put your main code here, to run repeatedly:
  if (runEvery(run_in_ms)) {

    float totalHashrate = 0.0;
    float avgHashrate = 0.0;
    int total_miner = 0;

    String input = httpGetString(ducoReportJsonUrl);

    DynamicJsonDocument doc (5000);
    DeserializationError error = deserializeJson(doc, input);

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }

    JsonObject result = doc["result"];
    JsonObject result_balance = result["balance"];
    double result_balance_balance = result_balance["balance"];
    const char *result_balance_created = result_balance["created"];
    const char *result_balance_username = result_balance["username"];
    const char *result_balance_verified = result_balance["verified"];

    for (JsonObject result_miner : result["miners"].as<JsonArray>()) {
      float result_miner_hashrate = result_miner["hashrate"];
      totalHashrate = totalHashrate + result_miner_hashrate;
      total_miner++;

    }

    avgHashrate = totalHashrate / long(total_miner);
    long run_span = run_in_ms / 1000;


    /*
       BEGIN DISPLAY

       refreshed every 5000 ms as in 'run_in_ms' variable

       I STRONGLY RECOMMEND PUT REFRESH RATE ABOVE 5000 MS,
       IT WILL LIGHTEN CURRENT DUINO SERVER AND YOUR BOARD MEMORY BUFFER, REALLY.

    */

    Serial.println("result_balance_username : " + String(result_balance_username));
    Serial.println("result_balance_verified : " + String(result_balance_verified));
    Serial.println("result_balance_balance : " + String(result_balance_balance));
    Serial.println("totalHashrate : " + String(totalHashrate));
    Serial.println("avgHashrate H/s : " + String(avgHashrate));
    Serial.println("total_miner : " + String(total_miner));

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.println("Ð * DUCO Monitor * Ð");
    display.println(" ");
    display.println("User     : " + String(result_balance_username));
    display.println("Verified : " + String(result_balance_verified));
    display.println("Miners   : " + String(total_miner));
    display.println("Balance  : " + String(result_balance_balance));
    display.println("H/s ("+ String(run_span) + "s) : " + String(totalHashrate));
    display.display();
  }
}

String httpGetString(String URL) {

  String payload = "";
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  if (http.begin(client, URL)) {
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      payload = http.getString();
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  return payload;
}


boolean runEvery(unsigned long interval) {
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}
