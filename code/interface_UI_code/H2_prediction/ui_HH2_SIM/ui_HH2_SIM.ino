#include <lvgl.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <ui.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

/*Don't forget to set Sketchbook location in File/Preferences to the path of your UI project (the parent foder of this INO file)*/

///////////////////////////////////////////////////////////////////////// init wifi and declare Http vars
// const char* ssid = "Proximus-Home-92F0";
// // Password of your Wifi network.
// const char* password = "w9bdkm3hjhm6p";

// const char* ssid = "Bastijn";
// // Password of your Wifi network.
// const char* password = "hetgroenehart!4";

const char* ssid = "Emilia’s iPhone";
// Password of your Wifi network.
const char* password = "ciaene11";

// declare setting char variable
// char SETTING;
int SETTING;

// For a final application, check the API call limits per hour/minute to avoid getting blocked/banned
unsigned long lastTime = 10001;
unsigned long firstTime = 0;
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

// Touchscreen pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
TFT_eSPI tft = TFT_eSPI();  //Added to be able to rotate the screen

// Touchscreen coordinates: (x, y) and pressure (z)
int x, y, z;

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

// If logging is enabled, it will inform the user about what is happening in the library
void log_print(lv_log_level_t level, const char * buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

// Get the Touchscreen data
void touchscreen_read(lv_indev_t * indev, lv_indev_data_t * data) {
  // Checks if Touchscreen was touched, and prints X, Y and Pressure (Z)
  if(touchscreen.tirqTouched() && touchscreen.touched()) {
    // Get Touchscreen points
    TS_Point p = touchscreen.getPoint();
    // Calibrate Touchscreen points with map function to the correct width and height
    x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    z = p.z;

    data->state = LV_INDEV_STATE_PRESSED;

    // Set the coordinates
    data->point.x = x;
    data->point.y = y;
  }
  else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

/*Set tick routine needed for LVGL internal timings*/
static uint32_t my_tick_get_cb (void) { return millis(); }


///////////////////////////////////////////////////////////////////// date labels on history tab /////////////////////////////////////////////////////////////////////
void gisteren_setdatelabels(lv_event_t * e) {
  String serverPath = "http://data.id.tue.nl/datasets/downloadPublic/json/SHFCRmZWTlBDZDVwaTgxK2NMeU4wNVAybFA3Tk9wMWJWM3N6dSs0U3hQWT0=";
  JSONVar myObject = JSON.parse(httpGETRequest(serverPath.c_str()));
  // JSON.typeof(jsonVar) can be used to get the type of the var
  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  }
  // lv_label_set_text(ui_GISTEREN_DAGKEUZE_LABEL1, myObject[0]["date_threedaysago"]);
  lv_label_set_text(ui_GISTEREN_DAGKEUZE_LABEL2, myObject[0]["date_twodaysago"]);
  lv_label_set_text(ui_GISTEREN_DAGKEUZE_LABEL3, myObject[0]["date_yesterday"]);
  //set the range for the temperature charts
  int mintemp = (double) myObject[0]["pp1"];
  int maxtemp = (double) myObject[0]["pp2"];
  lv_scale_set_range(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_Yaxis1,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_Yaxis2,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR_Yaxis1,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR_Yaxis2,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_TEMPERATUUR_LABELS_Yaxis1,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_TEMPERATUUR_LABELS_Yaxis2,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_MAX_TEMPERATUUR_Yaxis1,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_MAX_TEMPERATUUR_Yaxis2,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_MIN_TEMPERATUUR_Yaxis1,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_MIN_TEMPERATUUR_Yaxis2,  mintemp, maxtemp);

  lv_chart_set_range(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, LV_CHART_AXIS_PRIMARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, LV_CHART_AXIS_SECONDARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR, LV_CHART_AXIS_PRIMARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR, LV_CHART_AXIS_SECONDARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_TEMPERATUUR_LABELS, LV_CHART_AXIS_PRIMARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_TEMPERATUUR_LABELS, LV_CHART_AXIS_SECONDARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_MAX_TEMPERATUUR, LV_CHART_AXIS_PRIMARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_MAX_TEMPERATUUR, LV_CHART_AXIS_SECONDARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_MIN_TEMPERATUUR, LV_CHART_AXIS_PRIMARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_MIN_TEMPERATUUR, LV_CHART_AXIS_SECONDARY_Y, mintemp, maxtemp);

  // set values for the interface charts
  for (int i = 0; i < 24; i++) {
    lv_chart_series_t * ui_GISTEREN_CHART_RAAM_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_RAAM, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_RAAM, ui_GISTEREN_CHART_RAAM_series_1, (double) myObject[i]["window_door"]);
    lv_chart_series_t * ui_GISTEREN_CHART_GORDIJN__LINKS_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_GORDIJN__LINKS, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_GORDIJN__LINKS, ui_GISTEREN_CHART_GORDIJN__LINKS_series_1, (double) myObject[i]["curtain_left"]);
    lv_chart_series_t * ui_GISTEREN_CHART_GORDIJN_RECHTS_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_GORDIJN_RECHTS, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_GORDIJN_RECHTS, ui_GISTEREN_CHART_GORDIJN_RECHTS_series_1, (double) myObject[i]["curtain_right"]);
    lv_chart_series_t * ui_GISTEREN_CHART_SCHADUW_links_series_1_array = lv_chart_get_series_next(ui_GISTEREN_CHART_SCHADUW_links, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_SCHADUW_links, ui_GISTEREN_CHART_SCHADUW_links_series_1_array, (double) myObject[i]["shade_left"]);
    lv_chart_series_t * ui_GISTEREN_CHART_SCHADUW_rechts_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_SCHADUW_rechts, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_SCHADUW_rechts, ui_GISTEREN_CHART_SCHADUW_rechts_series_1, (double) myObject[i]["shade_right"]);
    lv_chart_series_t * ui_GISTEREN_CHART_BINNENDEUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BINNENDEUR, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_BINNENDEUR, ui_GISTEREN_CHART_BINNENDEUR_series_1, (double) myObject[i]["door_indoors"]);
    lv_chart_series_t * ui_GISTEREN_CHART_BINNEN_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR, ui_GISTEREN_CHART_BINNEN_TEMPERATUUR_series_1, (double) myObject[i]["Temperature_indoor"]);
    lv_chart_series_t * ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_series_1, (double) myObject[i]["Temperature_outdoor"]);
    lv_chart_series_t * ui_GISTEREN_CHART_MAX_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_MAX_TEMPERATUUR, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_MAX_TEMPERATUUR, ui_GISTEREN_CHART_MAX_TEMPERATUUR_series_1, 0);
    lv_chart_series_t * ui_GISTEREN_CHART_MIN_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_MIN_TEMPERATUUR, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_MIN_TEMPERATUUR, ui_GISTEREN_CHART_MIN_TEMPERATUUR_series_1, 0);
  }

  if(SETTING == 2) {
    String serverPath = "http://data.id.tue.nl/datasets/downloadPublic/json/enlRaW95YlNjNXpJQ3ZrTURKRlJaM2FwVllsN0gwVC9tRWRMOUpxa0xpND0=";
    JSONVar myObject = JSON.parse(httpGETRequest(serverPath.c_str()));
    // JSON.typeof(jsonVar) can be used to get the type of the var
    if (JSON.typeof(myObject) == "undefined") {
      Serial.println("Parsing input failed!");
      return;
    }
    for (int i = 0; i < 24; i++) {
      lv_chart_series_t * ui_GISTEREN_CHART_RAAM_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_RAAM, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_RAAM, ui_GISTEREN_CHART_RAAM_series_1, (double) myObject[i]["window_door"]);
      lv_chart_series_t * ui_GISTEREN_CHART_GORDIJN__LINKS_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_GORDIJN__LINKS, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_GORDIJN__LINKS, ui_GISTEREN_CHART_GORDIJN__LINKS_series_1, (double) myObject[i]["curtain_left"]);
      lv_chart_series_t * ui_GISTEREN_CHART_GORDIJN_RECHTS_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_GORDIJN_RECHTS, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_GORDIJN_RECHTS, ui_GISTEREN_CHART_GORDIJN_RECHTS_series_1, (double) myObject[i]["curtain_right"]);
      lv_chart_series_t * ui_GISTEREN_CHART_SCHADUW_links_series_1_array = lv_chart_get_series_next(ui_GISTEREN_CHART_SCHADUW_links, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_SCHADUW_links, ui_GISTEREN_CHART_SCHADUW_links_series_1_array, (double) myObject[i]["shade_left"]);
      lv_chart_series_t * ui_GISTEREN_CHART_SCHADUW_rechts_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_SCHADUW_rechts, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_SCHADUW_rechts, ui_GISTEREN_CHART_SCHADUW_rechts_series_1, (double) myObject[i]["shade_right"]);
      lv_chart_series_t * ui_GISTEREN_CHART_BINNENDEUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BINNENDEUR, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_BINNENDEUR, ui_GISTEREN_CHART_BINNENDEUR_series_1, (double) myObject[i]["door_indoors"]);
      lv_chart_series_t * ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_series_1, (double) myObject[i]["Temperature_outdoor"]);
      lv_chart_series_t * ui_GISTEREN_CHART_MAX_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_MAX_TEMPERATUUR, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_MAX_TEMPERATUUR, ui_GISTEREN_CHART_MAX_TEMPERATUUR_series_1, (double) myObject[i]["Temperature_indoor"]);
    } 
  } else if (SETTING == 1) {
    String serverPath = "http://data.id.tue.nl/datasets/downloadPublic/json/N20wY2ZLWW93dGh2S1QyclEzWEt5R1VlZmNIYnBLSUtFdWJaUXpXUGRUUT0=";
    JSONVar myObject = JSON.parse(httpGETRequest(serverPath.c_str()));
    // JSON.typeof(jsonVar) can be used to get the type of the var
    if (JSON.typeof(myObject) == "undefined") {
      Serial.println("Parsing input failed!");
      return;
    }    
    for (int i = 0; i < 24; i++) {
      lv_chart_series_t * ui_GISTEREN_CHART_RAAM_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_RAAM, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_RAAM, ui_GISTEREN_CHART_RAAM_series_1, (double) myObject[i]["window_door"]);
      lv_chart_series_t * ui_GISTEREN_CHART_GORDIJN__LINKS_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_GORDIJN__LINKS, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_GORDIJN__LINKS, ui_GISTEREN_CHART_GORDIJN__LINKS_series_1, (double) myObject[i]["curtain_left"]);
      lv_chart_series_t * ui_GISTEREN_CHART_GORDIJN_RECHTS_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_GORDIJN_RECHTS, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_GORDIJN_RECHTS, ui_GISTEREN_CHART_GORDIJN_RECHTS_series_1, (double) myObject[i]["curtain_right"]);
      lv_chart_series_t * ui_GISTEREN_CHART_SCHADUW_links_series_1_array = lv_chart_get_series_next(ui_GISTEREN_CHART_SCHADUW_links, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_SCHADUW_links, ui_GISTEREN_CHART_SCHADUW_links_series_1_array, (double) myObject[i]["shade_left"]);
      lv_chart_series_t * ui_GISTEREN_CHART_SCHADUW_rechts_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_SCHADUW_rechts, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_SCHADUW_rechts, ui_GISTEREN_CHART_SCHADUW_rechts_series_1, (double) myObject[i]["shade_right"]);
      lv_chart_series_t * ui_GISTEREN_CHART_BINNENDEUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BINNENDEUR, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_BINNENDEUR, ui_GISTEREN_CHART_BINNENDEUR_series_1, (double) myObject[i]["door_indoors"]);
      lv_chart_series_t * ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_series_1, (double) myObject[i]["Temperature_outdoor"]);
      lv_chart_series_t * ui_GISTEREN_CHART_MIN_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_MIN_TEMPERATUUR, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_MIN_TEMPERATUUR, ui_GISTEREN_CHART_MIN_TEMPERATUUR_series_1, (double) myObject[i]["Temperature_indoor"]);
    }
  }
}



///////////////////////////////////////////////////////////////////// set values for two days ago /////////////////////////////////////////////////////////////////////
void gisteren_setvalues_2(lv_event_t * e) {
  // lv_refr_now(NULL);
  String serverPath = "http://data.id.tue.nl/datasets/downloadPublic/json/aGlIRzJvd1BpNWxmeGFaV2dncVg0Z2IwRlR5b2tSTVQ4TzJzdzNueU9Fdz0=";
  JSONVar myObject = JSON.parse(httpGETRequest(serverPath.c_str()));
  // JSON.typeof(jsonVar) can be used to get the type of the var
  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  }
  //set the range for the temperature charts
  int mintemp = (double) myObject[0]["pp1"];
  int maxtemp = (double) myObject[0]["pp2"];
  lv_scale_set_range(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_Yaxis1,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_Yaxis2,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR_Yaxis1,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR_Yaxis2,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_TEMPERATUUR_LABELS_Yaxis1,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_TEMPERATUUR_LABELS_Yaxis2,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_MAX_TEMPERATUUR_Yaxis1,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_MAX_TEMPERATUUR_Yaxis2,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_MIN_TEMPERATUUR_Yaxis1,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_MIN_TEMPERATUUR_Yaxis2,  mintemp, maxtemp);

  lv_chart_set_range(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, LV_CHART_AXIS_PRIMARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, LV_CHART_AXIS_SECONDARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR, LV_CHART_AXIS_PRIMARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR, LV_CHART_AXIS_SECONDARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_TEMPERATUUR_LABELS, LV_CHART_AXIS_PRIMARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_TEMPERATUUR_LABELS, LV_CHART_AXIS_SECONDARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_MAX_TEMPERATUUR, LV_CHART_AXIS_PRIMARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_MAX_TEMPERATUUR, LV_CHART_AXIS_SECONDARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_MIN_TEMPERATUUR, LV_CHART_AXIS_PRIMARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_MIN_TEMPERATUUR, LV_CHART_AXIS_SECONDARY_Y, mintemp, maxtemp);

  for (int i = 0; i < 24; i++) {
    lv_chart_series_t * ui_GISTEREN_CHART_MAX_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_MAX_TEMPERATUUR, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_MAX_TEMPERATUUR, ui_GISTEREN_CHART_MAX_TEMPERATUUR_series_1, 0);
    lv_chart_series_t * ui_GISTEREN_CHART_MIN_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_MIN_TEMPERATUUR, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_MIN_TEMPERATUUR, ui_GISTEREN_CHART_MIN_TEMPERATUUR_series_1, 0);
    lv_chart_series_t * ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_series_1, (double) myObject[i]["Temperature_outdoor"]);
    lv_chart_series_t * ui_GISTEREN_CHART_BINNEN_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR, ui_GISTEREN_CHART_BINNEN_TEMPERATUUR_series_1, (double) myObject[i]["Temperature_indoor"]);
    lv_chart_series_t * ui_GISTEREN_CHART_RAAM_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_RAAM, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_RAAM, ui_GISTEREN_CHART_RAAM_series_1, (double) myObject[i]["window_door"]);
    lv_chart_series_t * ui_GISTEREN_CHART_GORDIJN__LINKS_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_GORDIJN__LINKS, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_GORDIJN__LINKS, ui_GISTEREN_CHART_GORDIJN__LINKS_series_1, (double) myObject[i]["curtain_left"]);
    lv_chart_series_t * ui_GISTEREN_CHART_GORDIJN_RECHTS_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_GORDIJN_RECHTS, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_GORDIJN_RECHTS, ui_GISTEREN_CHART_GORDIJN_RECHTS_series_1, (double) myObject[i]["curtain_right"]);
    lv_chart_series_t * ui_GISTEREN_CHART_SCHADUW_links_series_1_array = lv_chart_get_series_next(ui_GISTEREN_CHART_SCHADUW_links, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_SCHADUW_links, ui_GISTEREN_CHART_SCHADUW_links_series_1_array, (double) myObject[i]["shade_left"]);
    lv_chart_series_t * ui_GISTEREN_CHART_SCHADUW_rechts_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_SCHADUW_rechts, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_SCHADUW_rechts, ui_GISTEREN_CHART_SCHADUW_rechts_series_1, (double) myObject[i]["shade_right"]);
    lv_chart_series_t * ui_GISTEREN_CHART_BINNENDEUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BINNENDEUR, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_BINNENDEUR, ui_GISTEREN_CHART_BINNENDEUR_series_1, (double) myObject[i]["door_indoors"]);
  }
}

///////////////////////////////////////////////////////////////////// set values for yesterday /////////////////////////////////////////////////////////////////////
void gisteren_setvalues_3(lv_event_t * e) {
  String serverPath = "http://data.id.tue.nl/datasets/downloadPublic/json/UllFODNJazJpU0ZXTEFvU3hYYjdTMVNSRE1qUHhqamI5eWN0ckJ4V0xITT0=";
  JSONVar myObject = JSON.parse(httpGETRequest(serverPath.c_str()));
  // JSON.typeof(jsonVar) can be used to get the type of the var
  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  }
  //set the range for the temperature charts
  int mintemp = (double) myObject[0]["pp1"];
  int maxtemp = (double) myObject[0]["pp2"];
  lv_scale_set_range(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_Yaxis1,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_Yaxis2,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR_Yaxis1,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR_Yaxis2,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_TEMPERATUUR_LABELS_Yaxis1,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_TEMPERATUUR_LABELS_Yaxis2,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_MAX_TEMPERATUUR_Yaxis1,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_MAX_TEMPERATUUR_Yaxis2,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_MIN_TEMPERATUUR_Yaxis1,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_MIN_TEMPERATUUR_Yaxis2,  mintemp, maxtemp);

  lv_chart_set_range(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, LV_CHART_AXIS_PRIMARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, LV_CHART_AXIS_SECONDARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR, LV_CHART_AXIS_PRIMARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR, LV_CHART_AXIS_SECONDARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_TEMPERATUUR_LABELS, LV_CHART_AXIS_PRIMARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_TEMPERATUUR_LABELS, LV_CHART_AXIS_SECONDARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_MAX_TEMPERATUUR, LV_CHART_AXIS_PRIMARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_MAX_TEMPERATUUR, LV_CHART_AXIS_SECONDARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_MIN_TEMPERATUUR, LV_CHART_AXIS_PRIMARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_MIN_TEMPERATUUR, LV_CHART_AXIS_SECONDARY_Y, mintemp, maxtemp);

  // set values for the interface charts
  for (int i = 0; i < 24; i++) {
    lv_chart_series_t * ui_GISTEREN_CHART_MAX_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_MAX_TEMPERATUUR, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_MAX_TEMPERATUUR, ui_GISTEREN_CHART_MAX_TEMPERATUUR_series_1, 0);
    lv_chart_series_t * ui_GISTEREN_CHART_MIN_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_MIN_TEMPERATUUR, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_MIN_TEMPERATUUR, ui_GISTEREN_CHART_MIN_TEMPERATUUR_series_1, 0);
    lv_chart_series_t * ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_series_1, (double) myObject[i]["Temperature_outdoor"]);
    lv_chart_series_t * ui_GISTEREN_CHART_BINNEN_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR, ui_GISTEREN_CHART_BINNEN_TEMPERATUUR_series_1, (double) myObject[i]["Temperature_indoor"]);
    lv_chart_series_t * ui_GISTEREN_CHART_RAAM_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_RAAM, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_RAAM, ui_GISTEREN_CHART_RAAM_series_1, (double) myObject[i]["window_door"]);
    lv_chart_series_t * ui_GISTEREN_CHART_GORDIJN__LINKS_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_GORDIJN__LINKS, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_GORDIJN__LINKS, ui_GISTEREN_CHART_GORDIJN__LINKS_series_1, (double) myObject[i]["curtain_left"]);
    lv_chart_series_t * ui_GISTEREN_CHART_GORDIJN_RECHTS_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_GORDIJN_RECHTS, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_GORDIJN_RECHTS, ui_GISTEREN_CHART_GORDIJN_RECHTS_series_1, (double) myObject[i]["curtain_right"]);
    lv_chart_series_t * ui_GISTEREN_CHART_SCHADUW_links_series_1_array = lv_chart_get_series_next(ui_GISTEREN_CHART_SCHADUW_links, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_SCHADUW_links, ui_GISTEREN_CHART_SCHADUW_links_series_1_array, (double) myObject[i]["shade_left"]);
    lv_chart_series_t * ui_GISTEREN_CHART_SCHADUW_rechts_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_SCHADUW_rechts, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_SCHADUW_rechts, ui_GISTEREN_CHART_SCHADUW_rechts_series_1, (double) myObject[i]["shade_right"]);
    lv_chart_series_t * ui_GISTEREN_CHART_BINNENDEUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BINNENDEUR, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_BINNENDEUR, ui_GISTEREN_CHART_BINNENDEUR_series_1, (double) myObject[i]["door_indoors"]);
  }
}

///////////////////////////////////////////////////////////////////// set values for TODAY /////////////////////////////////////////////////////////////////////
void gisteren_setvalues_4(lv_event_t * e) {
  String serverPath = "http://data.id.tue.nl/datasets/downloadPublic/json/SHFCRmZWTlBDZDVwaTgxK2NMeU4wNVAybFA3Tk9wMWJWM3N6dSs0U3hQWT0=";
  JSONVar myObject = JSON.parse(httpGETRequest(serverPath.c_str()));
  // JSON.typeof(jsonVar) can be used to get the type of the var
  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  }
  //set the range for the temperature charts
  int mintemp = (double) myObject[0]["pp1"];
  int maxtemp = (double) myObject[0]["pp2"];
  lv_scale_set_range(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_Yaxis1,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_Yaxis2,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR_Yaxis1,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR_Yaxis2,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_TEMPERATUUR_LABELS_Yaxis1,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_TEMPERATUUR_LABELS_Yaxis2,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_MAX_TEMPERATUUR_Yaxis1,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_MAX_TEMPERATUUR_Yaxis2,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_MIN_TEMPERATUUR_Yaxis1,  mintemp, maxtemp);
  lv_scale_set_range(ui_GISTEREN_CHART_MIN_TEMPERATUUR_Yaxis2,  mintemp, maxtemp);

  lv_chart_set_range(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, LV_CHART_AXIS_PRIMARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, LV_CHART_AXIS_SECONDARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR, LV_CHART_AXIS_PRIMARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR, LV_CHART_AXIS_SECONDARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_TEMPERATUUR_LABELS, LV_CHART_AXIS_PRIMARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_TEMPERATUUR_LABELS, LV_CHART_AXIS_SECONDARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_MAX_TEMPERATUUR, LV_CHART_AXIS_PRIMARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_MAX_TEMPERATUUR, LV_CHART_AXIS_SECONDARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_MIN_TEMPERATUUR, LV_CHART_AXIS_PRIMARY_Y, mintemp, maxtemp);
  lv_chart_set_range(ui_GISTEREN_CHART_MIN_TEMPERATUUR, LV_CHART_AXIS_SECONDARY_Y, mintemp, maxtemp);

  // set values for the interface charts
  for (int i = 0; i < 24; i++) {
    lv_chart_series_t * ui_GISTEREN_CHART_RAAM_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_RAAM, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_RAAM, ui_GISTEREN_CHART_RAAM_series_1, (double) myObject[i]["window_door"]);
    lv_chart_series_t * ui_GISTEREN_CHART_GORDIJN__LINKS_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_GORDIJN__LINKS, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_GORDIJN__LINKS, ui_GISTEREN_CHART_GORDIJN__LINKS_series_1, (double) myObject[i]["curtain_left"]);
    lv_chart_series_t * ui_GISTEREN_CHART_GORDIJN_RECHTS_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_GORDIJN_RECHTS, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_GORDIJN_RECHTS, ui_GISTEREN_CHART_GORDIJN_RECHTS_series_1, (double) myObject[i]["curtain_right"]);
    lv_chart_series_t * ui_GISTEREN_CHART_SCHADUW_links_series_1_array = lv_chart_get_series_next(ui_GISTEREN_CHART_SCHADUW_links, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_SCHADUW_links, ui_GISTEREN_CHART_SCHADUW_links_series_1_array, (double) myObject[i]["shade_left"]);
    lv_chart_series_t * ui_GISTEREN_CHART_SCHADUW_rechts_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_SCHADUW_rechts, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_SCHADUW_rechts, ui_GISTEREN_CHART_SCHADUW_rechts_series_1, (double) myObject[i]["shade_right"]);
    lv_chart_series_t * ui_GISTEREN_CHART_BINNENDEUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BINNENDEUR, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_BINNENDEUR, ui_GISTEREN_CHART_BINNENDEUR_series_1, (double) myObject[i]["door_indoors"]);
    lv_chart_series_t * ui_GISTEREN_CHART_BINNEN_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR, ui_GISTEREN_CHART_BINNEN_TEMPERATUUR_series_1, (double) myObject[i]["Temperature_indoor"]);
    lv_chart_series_t * ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_series_1, (double) myObject[i]["Temperature_outdoor"]);
    lv_chart_series_t * ui_GISTEREN_CHART_MAX_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_MAX_TEMPERATUUR, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_MAX_TEMPERATUUR, ui_GISTEREN_CHART_MAX_TEMPERATUUR_series_1, 0);
    lv_chart_series_t * ui_GISTEREN_CHART_MIN_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_MIN_TEMPERATUUR, NULL);
    lv_chart_set_next_value(ui_GISTEREN_CHART_MIN_TEMPERATUUR, ui_GISTEREN_CHART_MIN_TEMPERATUUR_series_1, 0);
  }

  if(SETTING == 2) {
    String serverPath = "http://data.id.tue.nl/datasets/downloadPublic/json/enlRaW95YlNjNXpJQ3ZrTURKRlJaM2FwVllsN0gwVC9tRWRMOUpxa0xpND0=";
    JSONVar myObject = JSON.parse(httpGETRequest(serverPath.c_str()));
    // JSON.typeof(jsonVar) can be used to get the type of the var
    if (JSON.typeof(myObject) == "undefined") {
      Serial.println("Parsing input failed!");
      return;
    }
    for (int i = 0; i < 24; i++) {
      lv_chart_series_t * ui_GISTEREN_CHART_RAAM_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_RAAM, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_RAAM, ui_GISTEREN_CHART_RAAM_series_1, (double) myObject[i]["window_door"]);
      lv_chart_series_t * ui_GISTEREN_CHART_GORDIJN__LINKS_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_GORDIJN__LINKS, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_GORDIJN__LINKS, ui_GISTEREN_CHART_GORDIJN__LINKS_series_1, (double) myObject[i]["curtain_left"]);
      lv_chart_series_t * ui_GISTEREN_CHART_GORDIJN_RECHTS_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_GORDIJN_RECHTS, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_GORDIJN_RECHTS, ui_GISTEREN_CHART_GORDIJN_RECHTS_series_1, (double) myObject[i]["curtain_right"]);
      lv_chart_series_t * ui_GISTEREN_CHART_SCHADUW_links_series_1_array = lv_chart_get_series_next(ui_GISTEREN_CHART_SCHADUW_links, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_SCHADUW_links, ui_GISTEREN_CHART_SCHADUW_links_series_1_array, (double) myObject[i]["shade_left"]);
      lv_chart_series_t * ui_GISTEREN_CHART_SCHADUW_rechts_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_SCHADUW_rechts, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_SCHADUW_rechts, ui_GISTEREN_CHART_SCHADUW_rechts_series_1, (double) myObject[i]["shade_right"]);
      lv_chart_series_t * ui_GISTEREN_CHART_BINNENDEUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BINNENDEUR, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_BINNENDEUR, ui_GISTEREN_CHART_BINNENDEUR_series_1, (double) myObject[i]["door_indoors"]);
      lv_chart_series_t * ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_series_1, (double) myObject[i]["Temperature_outdoor"]);
      lv_chart_series_t * ui_GISTEREN_CHART_MAX_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_MAX_TEMPERATUUR, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_MAX_TEMPERATUUR, ui_GISTEREN_CHART_MAX_TEMPERATUUR_series_1, (double) myObject[i]["Temperature_indoor"]);
    } 
  } else if (SETTING == 1) {
    String serverPath = "http://data.id.tue.nl/datasets/downloadPublic/json/N20wY2ZLWW93dGh2S1QyclEzWEt5R1VlZmNIYnBLSUtFdWJaUXpXUGRUUT0=";
    JSONVar myObject = JSON.parse(httpGETRequest(serverPath.c_str()));
    // JSON.typeof(jsonVar) can be used to get the type of the var
    if (JSON.typeof(myObject) == "undefined") {
      Serial.println("Parsing input failed!");
      return;
    }    
    for (int i = 0; i < 24; i++) {
      lv_chart_series_t * ui_GISTEREN_CHART_RAAM_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_RAAM, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_RAAM, ui_GISTEREN_CHART_RAAM_series_1, (double) myObject[i]["window_door"]);
      lv_chart_series_t * ui_GISTEREN_CHART_GORDIJN__LINKS_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_GORDIJN__LINKS, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_GORDIJN__LINKS, ui_GISTEREN_CHART_GORDIJN__LINKS_series_1, (double) myObject[i]["curtain_left"]);
      lv_chart_series_t * ui_GISTEREN_CHART_GORDIJN_RECHTS_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_GORDIJN_RECHTS, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_GORDIJN_RECHTS, ui_GISTEREN_CHART_GORDIJN_RECHTS_series_1, (double) myObject[i]["curtain_right"]);
      lv_chart_series_t * ui_GISTEREN_CHART_SCHADUW_links_series_1_array = lv_chart_get_series_next(ui_GISTEREN_CHART_SCHADUW_links, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_SCHADUW_links, ui_GISTEREN_CHART_SCHADUW_links_series_1_array, (double) myObject[i]["shade_left"]);
      lv_chart_series_t * ui_GISTEREN_CHART_SCHADUW_rechts_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_SCHADUW_rechts, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_SCHADUW_rechts, ui_GISTEREN_CHART_SCHADUW_rechts_series_1, (double) myObject[i]["shade_right"]);
      lv_chart_series_t * ui_GISTEREN_CHART_BINNENDEUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BINNENDEUR, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_BINNENDEUR, ui_GISTEREN_CHART_BINNENDEUR_series_1, (double) myObject[i]["door_indoors"]);
      lv_chart_series_t * ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_series_1, (double) myObject[i]["Temperature_outdoor"]);
      lv_chart_series_t * ui_GISTEREN_CHART_MIN_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_MIN_TEMPERATUUR, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_MIN_TEMPERATUUR, ui_GISTEREN_CHART_MIN_TEMPERATUUR_series_1, (double) myObject[i]["Temperature_indoor"]);
    }
  }

}

///////////////////////////////////////////////////////////////////// set values for TOMORROW /////////////////////////////////////////////////////////////////////
void gisteren_setvalues_1(lv_event_t * e) {
  if(SETTING == 2) {
    String serverPath = "http://data.id.tue.nl/datasets/downloadPublic/json/MHM1Y2VKdUZsQUxhVnZ1c3o3V2NWTWREOFl4cHZIRkdNTk5rZlRucHoyST0=";
    JSONVar myObject = JSON.parse(httpGETRequest(serverPath.c_str()));
    // JSON.typeof(jsonVar) can be used to get the type of the var
    if (JSON.typeof(myObject) == "undefined") {
      Serial.println("Parsing input failed!");
      return;
    }
    for (int i = 0; i < 24; i++) {
      lv_chart_series_t * ui_GISTEREN_CHART_BINNEN_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR, ui_GISTEREN_CHART_BINNEN_TEMPERATUUR_series_1, 0);
      lv_chart_series_t * ui_GISTEREN_CHART_RAAM_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_RAAM, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_RAAM, ui_GISTEREN_CHART_RAAM_series_1, (double) myObject[i]["window_door"]);
      lv_chart_series_t * ui_GISTEREN_CHART_GORDIJN__LINKS_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_GORDIJN__LINKS, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_GORDIJN__LINKS, ui_GISTEREN_CHART_GORDIJN__LINKS_series_1, (double) myObject[i]["curtain_left"]);
      lv_chart_series_t * ui_GISTEREN_CHART_GORDIJN_RECHTS_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_GORDIJN_RECHTS, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_GORDIJN_RECHTS, ui_GISTEREN_CHART_GORDIJN_RECHTS_series_1, (double) myObject[i]["curtain_right"]);
      lv_chart_series_t * ui_GISTEREN_CHART_SCHADUW_links_series_1_array = lv_chart_get_series_next(ui_GISTEREN_CHART_SCHADUW_links, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_SCHADUW_links, ui_GISTEREN_CHART_SCHADUW_links_series_1_array, (double) myObject[i]["shade_left"]);
      lv_chart_series_t * ui_GISTEREN_CHART_SCHADUW_rechts_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_SCHADUW_rechts, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_SCHADUW_rechts, ui_GISTEREN_CHART_SCHADUW_rechts_series_1, (double) myObject[i]["shade_right"]);
      lv_chart_series_t * ui_GISTEREN_CHART_BINNENDEUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BINNENDEUR, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_BINNENDEUR, ui_GISTEREN_CHART_BINNENDEUR_series_1, (double) myObject[i]["door_indoors"]);
      lv_chart_series_t * ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_series_1, (double) myObject[i]["Temperature_outdoor"]);
      lv_chart_series_t * ui_GISTEREN_CHART_MAX_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_MAX_TEMPERATUUR, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_MAX_TEMPERATUUR, ui_GISTEREN_CHART_MAX_TEMPERATUUR_series_1, (double) myObject[i]["Temperature_indoor"]);
    } 
  } else if (SETTING == 1) {
    String serverPath = "http://data.id.tue.nl/datasets/downloadPublic/json/Mi90bjBsRDdZK21wVTBzTE1ZUlBCNlIxM3hhMVZORnNlSlMvRmVyZUFsdz0=";
    JSONVar myObject = JSON.parse(httpGETRequest(serverPath.c_str()));
    // JSON.typeof(jsonVar) can be used to get the type of the var
    if (JSON.typeof(myObject) == "undefined") {
      Serial.println("Parsing input failed!");
      return;
    }    
    for (int i = 0; i < 24; i++) {
      lv_chart_series_t * ui_GISTEREN_CHART_BINNEN_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_BINNEN_TEMPERATUUR, ui_GISTEREN_CHART_BINNEN_TEMPERATUUR_series_1, 0);
      lv_chart_series_t * ui_GISTEREN_CHART_RAAM_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_RAAM, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_RAAM, ui_GISTEREN_CHART_RAAM_series_1, (double) myObject[i]["window_door"]);
      lv_chart_series_t * ui_GISTEREN_CHART_GORDIJN__LINKS_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_GORDIJN__LINKS, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_GORDIJN__LINKS, ui_GISTEREN_CHART_GORDIJN__LINKS_series_1, (double) myObject[i]["curtain_left"]);
      lv_chart_series_t * ui_GISTEREN_CHART_GORDIJN_RECHTS_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_GORDIJN_RECHTS, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_GORDIJN_RECHTS, ui_GISTEREN_CHART_GORDIJN_RECHTS_series_1, (double) myObject[i]["curtain_right"]);
      lv_chart_series_t * ui_GISTEREN_CHART_SCHADUW_links_series_1_array = lv_chart_get_series_next(ui_GISTEREN_CHART_SCHADUW_links, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_SCHADUW_links, ui_GISTEREN_CHART_SCHADUW_links_series_1_array, (double) myObject[i]["shade_left"]);
      lv_chart_series_t * ui_GISTEREN_CHART_SCHADUW_rechts_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_SCHADUW_rechts, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_SCHADUW_rechts, ui_GISTEREN_CHART_SCHADUW_rechts_series_1, (double) myObject[i]["shade_right"]);
      lv_chart_series_t * ui_GISTEREN_CHART_BINNENDEUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BINNENDEUR, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_BINNENDEUR, ui_GISTEREN_CHART_BINNENDEUR_series_1, (double) myObject[i]["door_indoors"]);
      lv_chart_series_t * ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_BUITEN_TEMPERATUUR, ui_GISTEREN_CHART_BUITEN_TEMPERATUUR_series_1, (double) myObject[i]["Temperature_outdoor"]);
      lv_chart_series_t * ui_GISTEREN_CHART_MIN_TEMPERATUUR_series_1 = lv_chart_get_series_next(ui_GISTEREN_CHART_MIN_TEMPERATUUR, NULL);
      lv_chart_set_next_value(ui_GISTEREN_CHART_MIN_TEMPERATUUR, ui_GISTEREN_CHART_MIN_TEMPERATUUR_series_1, (double) myObject[i]["Temperature_indoor"]);
    }
  }
}



void SETTINGS_SETMAX(lv_event_t * e)
{
  lv_bar_set_start_value(ui_settings_bar_range, 17, LV_ANIM_OFF);
  lv_label_set_text(ui_settings_bar_min_label, "17.6");

  lv_bar_set_value(ui_settings_bar_range, 22, LV_ANIM_OFF);
  lv_label_set_text(ui_settings_bar_max_label, "22.4");

  lv_bar_set_value(ui_settings_bar_actions, 6, LV_ANIM_OFF);
  lv_label_set_text(ui_settings_bar_actions_label, "6");

  lv_roller_set_options(ui_HOME_ACTIONLIST,"sluit gordijn R\nsluit binnendeur\n------\nopen gordijn R\nopen gordijn L\nsluit gordijn R\nsluit gordijn L", LV_ROLLER_MODE_NORMAL);
  lv_roller_set_options(ui_HOME_ACTIONLIST1, "20:30\n20:30\n---\n08:30\n09:00\n16:00\n16:30", LV_ROLLER_MODE_NORMAL);

  // SET THE SETTINGS COLOR BAR
  lv_obj_set_style_bg_color(ui_HOME_SETTING_PANEL_COLOR, lv_color_hex(0x843737), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(ui_HOME_SETTING_LABEL, "MAX");

  // SET THE MINMAX LABELS/COLORS IN THE HISTORY TAB
  lv_label_set_text(ui_GISTEREN_LABEL_minmax_keuze, "max");
  lv_obj_set_style_text_color(ui_GISTEREN_LABEL_minmax_keuze, lv_color_hex(0x843737), LV_PART_MAIN | LV_STATE_DEFAULT);

  SETTING = 2;
}

void SETTINGS_SETMIN(lv_event_t * e)
{
  lv_bar_set_start_value(ui_settings_bar_range, 10, LV_ANIM_OFF);
  lv_label_set_text(ui_settings_bar_min_label, "9. 7");

  lv_bar_set_value(ui_settings_bar_range, 17, LV_ANIM_OFF);
  lv_label_set_text(ui_settings_bar_max_label, "17.2");

  lv_bar_set_value(ui_settings_bar_actions, 7, LV_ANIM_OFF);
  lv_label_set_text(ui_settings_bar_actions_label, "7");

  lv_roller_set_options(ui_HOME_ACTIONLIST,"open raam\nopen gordijn L\nsluit binnendeur\n------\nsluit gordijn R\nsluit gordijn L\nopen gordijn R\nopen gordijn L", LV_ROLLER_MODE_NORMAL);
  lv_roller_set_options(ui_HOME_ACTIONLIST1, "20:30\n20:30\n22:30\n---\n08:30\n09:00\n17:00\n17:30", LV_ROLLER_MODE_NORMAL);

  // SET THE SETTINGS COLOR BAR
  lv_obj_set_style_bg_color(ui_HOME_SETTING_PANEL_COLOR, lv_color_hex(0x317d84), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(ui_HOME_SETTING_LABEL, "MIN");

  // SET THE MINMAX LABEL IN THE HISTORY TAB
  lv_label_set_text(ui_GISTEREN_LABEL_minmax_keuze, "min");
  lv_obj_set_style_text_color(ui_GISTEREN_LABEL_minmax_keuze, lv_color_hex(0x3fb7de), LV_PART_MAIN | LV_STATE_DEFAULT);

  SETTING = 1;
}

void setup ()
{
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(115200);
  Serial.println(LVGL_Arduino);

  //////////////////////////////////////////// HTTP request start
  WiFi.begin(ssid, password);

  //////////////////////////////////////////// Start LVGL
  lv_init();
  // Register print function for debugging
  lv_log_register_print_cb(log_print);

  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  // Set the Touchscreen rotation in landscape mode
  // Note: in some displays, the touchscreen might be upside down, so you might need to set the rotation to 1: touchscreen.setRotation(1);
  touchscreen.setRotation(0);

  // Create a display object
  lv_display_t * disp;
  // Initialize the TFT display using the TFT_eSPI library  //////////////////////////////////////////////////////////// KEEP
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));

  // lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_90)
  tft.begin();
  tft.setRotation(0);

  // Initialize an LVGL input device object (Touchscreen)
  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  // Set the callback function to read Touchscreen input
  lv_indev_set_read_cb(indev, touchscreen_read);

  ui_init(); /////////////////////////////////////////////// this calls the ui I made in squareline, export ui files to ui file in arduino library

}

unsigned long timer = 0;

void loop ()
{
  // // lv_timer_handler(); /* let the GUI do its work */
  lv_task_handler();  // let the GUI do its work
  lv_tick_inc(5);     // tell LVGL how much time has passed
  delay(5);           // let this time pass

  if(touchscreen.tirqTouched() && touchscreen.touched()){
  timer = millis();
  }
  else if(millis() - timer >= 90000){ //if one and a half minute passed
    // Serial.println("Sleep…");
    _ui_screen_change(&ui_INACTIVITY, LV_SCR_LOAD_ANIM_NONE, 100, 0, &ui_INACTIVITY_screen_init);
    timer = 0;
  }
}

// comment out events in ui_events.c in ui src