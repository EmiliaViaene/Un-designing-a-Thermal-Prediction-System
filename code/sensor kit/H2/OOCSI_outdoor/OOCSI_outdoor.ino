#include <OOCSI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <HTTPClient.h>

// GPIO where the DS18B20 is connected to
const int oneWireBus = 4;     
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

//////////////////////////////////////////////////////////////////////// OOCSI 
const char* ssid = "Bastijn";
// Password of your Wifi network.
const char* password = "hetgroenehart!4";
// name for connecting with OOCSI (unique handle)
const char* OOCSIName = "HH2_outdoor";
// put the adress of your OOCSI server here, can be URL or IP address string
const char* hostserver = "oocsi.id.tue.nl";
// OOCSI reference for the entire sketch
// To connect to an OOCSI network, that is, a running OOCSI server, you first need to create an OOCSI reference:
OOCSI oocsi = OOCSI();


////////////////////////////////////////////////////////////// SLEEP TIMER
const uint32_t SLEEP_DURATION = 3600 * 1000000; // 1 hour = 60*60=3600(I want one reading per hour, taking into account delays etc.)
// ESP32 boot count
RTC_DATA_ATTR int bootCount = 0;
// Enter Deep Sleep with Timer Wake-up source
void deepSleep() {
    esp_sleep_enable_timer_wakeup(SLEEP_DURATION);
    esp_deep_sleep_start();
}



void setup() {
  Serial.begin(9600);
  delay(1000); // Wait a bit to make sure Serial starts

  sensors.begin();
  WiFi.begin(ssid, password);

  // Print boot counter every boot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  // output OOCSI activity on the built-in LED
  pinMode(LED_BUILTIN, OUTPUT);
  oocsi.setActivityLEDPin(LED_BUILTIN);
  oocsi.connect(OOCSIName, hostserver, ssid, password);
}

void loop() {
  ////////////////////////////////////////////////////////////////////////// SEND TEMPERATURE SENSOR DATA
  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);
  float temperatureF = sensors.getTempFByIndex(0);
  // Serial.print(temperatureC);
  // Serial.println("ºC");
  // make OOCSI message ready. Insert DF channel here
  oocsi.newMessage("HH_outdoor_temp");
  // replace DEVICE with your reference ID from a device
  oocsi.addString("device_id", "df2729158ae714476");
    // identify participant
  oocsi.addString("participant", "H2");


  // add outdoor temperature data to be stored in the dataset
  oocsi.addFloat("Temperature", temperatureC);

  oocsi.sendMessage();   // this command will send the message; don't forget to call this after creating a message
  oocsi.printSendMessage();   // prints out the raw message (how it is sent to the OOCSI server)
  oocsi.keepAlive();   // use the check() if you also need to process incoming messages/ use keepAlive() if you do NOT need to receive data from OOCSI

  Serial.println("Entering Deep Sleep mode..."); ////// SLEEP MODE (TO ADJUST SEE SLEEPTIMER ^)
  Serial.flush();
  deepSleep();
}
