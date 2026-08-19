#include <OOCSI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

//////////////////////////////////////////////////////////////  FOR OOCSI CONNECTION: 
// use this if you want the OOCSI-ESP library to manage the connection to the Wifi
// SSID of your Wifi network, the library currently does not support WPA2 Enterprise networks
// const char* ssid = "Proximus-Home-92F0";
// // Password of your Wifi network.
// const char* password = "w9bdkm3hjhm6p";

const char* ssid = "Telenet0515971_Guest";
// Password of your Wifi network.
const char* password = "&YZAFWl3$i33";

// name for connecting with OOCSI (unique handle)
const char* OOCSIName = "HH3_window_1";
// put the adress of your OOCSI server here, can be URL or IP address string
const char* hostserver = "oocsi.id.tue.nl";
// OOCSI reference for the entire sketch
// To connect to an OOCSI network, that is, a running OOCSI server, you first need to create an OOCSI reference:
OOCSI oocsi = OOCSI();

////////////////////////////////////////////////////////////// assign sensor pins
#define LIGHT_SENSOR_PIN 36 // ESP32 pin GIOP36 connected to LDR sensor's pin
const int trigPin = 5; // trigger pin distance sensor to GPIOP 5 
const int echoPin = 18; // echo pin distance sensor to GPIOP 18

int LDRvalue; //instance for the light sensor (LDR sensor)
#define SOUND_SPEED 0.034 //define sound speed in cm
long duration; // call duration
float distanceCm; // call distance in cm

#define DOOR_SENSOR_PIN  19  // ESP32 pin GPIO19 connected to door sensor's pin
int doorState;

////////////////////////////////////////////////////////////// SLEEP TIMER
const uint32_t SLEEP_DURATION = 600 * 1000000; // 10*60=600-10 (I want one reading per 10 minutes, taking into account delays etc.)
// ESP32 boot count
RTC_DATA_ATTR int bootCount = 0;
// Enter Deep Sleep with Timer Wake-up source
void deepSleep() {
    esp_sleep_enable_timer_wakeup(SLEEP_DURATION);
    esp_deep_sleep_start();
}



// put your setup code here, to run once:
void setup() {
  Serial.begin(9600);
  
  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP); // set ESP32 pin to input pull-up mode
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output for distance sensor
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input for distance sensor

  // Print boot counter every boot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  // output OOCSI activity on the built-in LED
  pinMode(LED_BUILTIN, OUTPUT);
  oocsi.setActivityLEDPin(LED_BUILTIN);
  // connect wifi and OOCSI to the server
  oocsi.connect(OOCSIName, hostserver, ssid, password);
}

void loop() {
  // make OOCSI message ready. Insert DF channel here
  oocsi.newMessage("HH_Window_receiver");
  // replace DEVICE with your reference ID from a device
  oocsi.addString("device_id", "db4881e805a254484");
  // identify participant
  oocsi.addString("participant", "H3");

  // create data messages for OOCSI
    // REED sensor state (open/closed)
  doorState = digitalRead(DOOR_SENSOR_PIN); // read window state (REED SENSOR: 0/1)
  oocsi.addInt("reed sensor", int(doorState)); // the raw analog reading: 0-1
  if (doorState == HIGH) {
    oocsi.addString("window", "open");
  } else {
    oocsi.addString("window", "closed");
  }
    // LDR sensor state (dark, shade, sunlight)
  LDRvalue = analogRead(LIGHT_SENSOR_PIN); //read input from the LDR sensor (between 0 and 4095)
  oocsi.addInt("light sensor", int(LDRvalue)); // the raw analog reading: 0-4095
  if (LDRvalue < 1000) {   // We'll have a few threshholds, qualitatively determined
    oocsi.addString("light", "low");
  } else if (LDRvalue < 2000) {
    oocsi.addString("light", "medium_low");
  } else if (LDRvalue < 300) {
    oocsi.addString("light", "medium_high");
  } else {
    oocsi.addString("light", "high");
  }
    // distance sensor distance value
  // Clears the distance sensor's trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the dinstance sensor's trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;
  // oocsi message
  oocsi.addInt("distance sensor", int(distanceCm));
  if (distanceCm > 30) {
    oocsi.addString("curtain", "open");
  } else {
    oocsi.addString("curtain", "closed");
  }
  oocsi.sendMessage();   // this command will send the message; don't forget to call this after creating a message
  oocsi.printSendMessage();   // prints out the raw message (how it is sent to the OOCSI server)
  oocsi.keepAlive();   // use the check() if you also need to process incoming messages/ use keepAlive() if you do NOT need to receive data from OOCSI

  Serial.println("Entering Deep Sleep mode..."); ////// SLEEP MODE (TO ADJUST SEE SLEEPTIMER ^)
  Serial.flush(); // make sure all data has been sent
  deepSleep();
}