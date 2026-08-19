#include "OOCSI.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"
#include <OneWire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSansBoldOblique24pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

////////////////////////////////////////////////////////////////////////// DHT22
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// //////////////////////////////////////////////////////////////////////// API
const char* ssid = "VFNL-DF7B08";
//Password of your Wifi network.
const char* password = "4W8DNSU424SP4";

// const char* ssid = "Proximus-Home-92F0";
// // Password of your Wifi network.
// const char* password = "w9bdkm3hjhm6p";

// Your Domain name with URL path or IP address with path
String openWeatherMapApiKey = "273a7bdc5f26d184400ce98d1bc8e957";
// lat en lon voor: H1
String lat = "51.3856734";
String lon = "5.466115689689311";
// For a final application, check the API call limits per hour/minute to avoid getting blocked/banned
unsigned long lastTime = 0;
// Timer set to 1 minutes (60000)
unsigned long timerDelay = 10000;
String jsonBuffer;
String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  String payload = "{}"; 
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
  return payload;
}

//////////////////////////////////////////////////////////////////////// SET UP OOCSI
// name for connecting with OOCSI (unique handle)
const char* OOCSIName = "HH1_indoor_data_V2.2";
// put the adress of your OOCSI server here, can be URL or IP address string
const char* hostserver = "oocsi.id.tue.nl";
// OOCSI reference for the entire sketch
OOCSI oocsi = OOCSI();


//////////////////////////////////////////////////////////////////////// OOCSI RECEIVE OUTDOOR TEMP DATA
float Temperature_sensor; // declare the float and rounded int to call the sensorvalues
int Temperature_sensor_round;
// function which OOCSI calls when an OOCSI message is received
void processOOCSI() {
  float Temperature_HH1 = oocsi.getFloat("Temperature", -200.0);
  String Participant_HH1 = oocsi.getString("participant", "");

  if (Temperature_HH1 > -200.0 && Participant_HH1 == "H1") { // only return the temperature if its value is not -200.0 & specify which household I want the data from
    Temperature_sensor = oocsi.getFloat("Temperature", -200.0);
    Temperature_sensor_round = round(Temperature_sensor);
  } 
}


void setup() {
  Serial.begin(9600);
  ////////////////////////////////////////////////////////////////////////// check bme280 sensor & start wifi
  dht.begin();
  WiFi.begin(ssid, password);

  ////////////////////////////////////////////////////////////////////////// setting up OOCSI
  // setting up OOCSI. processOOCSI is the name of the function to call when receiving messages, can be a random function name
  oocsi.connect(OOCSIName, hostserver, ssid, password, processOOCSI);
  // subscribe to a channel (for receiving messages)
  oocsi.subscribe("HH_outdoor_temp");

  ////////////////////////////////////////////////////////////////////////// setting up the OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);
}


void loop() { 
  display.clearDisplay(); //clear the display so values are not printed on top of each other

  ////////////////////////////////////////////////////////////////////////// GET, DISPLAY, AND SEND TEMPERATURE SENSOR DATA
  float temperature = dht.readTemperature();
  float temperature_corrected = temperature - 3.2;
  int temperature_round = round(temperature);

  // send the value to the OLED
  display.setFont(&FreeSansBold18pt7b);
  display.setCursor(0, 45);
  // Display static text
  display.println(temperature_corrected, 1);

  // Send the value to OOCSI (Datafoundry dataset)
  oocsi.newMessage("HH_INDOOR_receiver");
  // replace DEVICE with your reference ID from a device
  oocsi.addString("device_id", "da6b9f7cb5a3b4e15");
  // identify participant
  oocsi.addString("participant", "H1");

  // add outdoor temperature data to be stored in the dataset
  oocsi.addFloat("Temperature", temperature_corrected);

  // oocsi.sendMessage();   // this command will send the message; don't forget to call this after creating a message
  oocsi.printSendMessage();   // prints out the raw message (how it is sent to the OOCSI server)


  ////////////////////////////////////////////////////////////////////////// GET API DATA
  String serverPath = "http://api.openweathermap.org/data/2.5/weather?lat=" + lat + "&lon=" + lon + "&appid=" + openWeatherMapApiKey + "&units=metric";
  jsonBuffer = httpGETRequest(serverPath.c_str());
  Serial.println(jsonBuffer);
  JSONVar myObject = JSON.parse(jsonBuffer);
  // ARDUINOJSON_USE_DOUBLE;
  // JSON.typeof(jsonVar) can be used to get the type of the var
  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  }

  // make OOCSI message ready. Insert DF channel here
  oocsi.newMessage("HH_API_data");
  // replace DEVICE with your reference ID from a device
  oocsi.addString("device_id", "d377054c1ed6e496f");
  // identify participant
  oocsi.addString("participant", "H1");

  oocsi.addFloat("Temperature_API", (double) myObject["main"]["temp"]);
  oocsi.addFloat("Temperature_API_MIN", (double) myObject["main"]["temp_min"]);
  oocsi.addFloat("Temperature_API_MAX", (double) myObject["main"]["temp_max"]);
  oocsi.addString("weather_main", myObject["weather"][0]["main"]);
  oocsi.addString("weather_description", myObject["weather"][0]["description"]);

  oocsi.sendMessage();   // this command will send the message; don't forget to call this after creating a message
  oocsi.printSendMessage();   // prints out the raw message (how it is sent to the OOCSI server)
  oocsi.check();   // use the check() if you also need to process incoming messages/ use keepAlive() if you do NOT need to receive data from OOCSI

  // get a mean api temperature value to display on the OLED
  float API_temperature = (double) myObject["main"]["temp"];
  int API_temperature_round = round(API_temperature);


  ////////////////////////////////////////////////////////////////////////// RECEIVE OOCSI OUTDOOR TEMP DATA & CALCULATE MEAN OUTDOOR TEMP (WITH API VALUE)
  processOOCSI(); // get the oocsi sensor values from the function declared above
  int Temperature_mean = (API_temperature_round + Temperature_sensor_round)/2; // calculate the mean of the api and oocsi value


  //////////////////////////////////////////////////////////////////////////  CALCULATE & DISPLAY MEAN OUTDOOR TEMP (WITH API VALUE)
  Serial.print("Temperature_API: ");
  Serial.println(API_temperature_round);
  Serial.print("Temperature_OOCSI: ");
  Serial.println(Temperature_sensor_round);
  Serial.print("Temperature_mean: ");
  Serial.println(Temperature_mean);

  // DISPLAY THE MEAN TEMP VALUE ON THE OLED
  display.setFont(&FreeSans9pt7b);
  display.setCursor(88, 38);
  // Display static text
  if   (Temperature_sensor > 0 | Temperature_sensor < 0) {
    display.println(Temperature_mean, 1);
  }
  else {
    display.println(API_temperature, 1);
  }


  ////////////////////////////////////////////////////////////////////////// FIXED DISPLAY ELEMENTS
  display.drawCircle(71, 22, 2, WHITE);
  // display.setFont(&FreeSans9pt7b);
  // display.setCursor(60, 48);
  // display.println("in");
  display.drawLine(80, 0, 80, 64, WHITE);
  display.drawCircle(125, 27.5, 1, WHITE);

  display.display();
  delay(60000);  // wait 60 seconds before looping again
}

