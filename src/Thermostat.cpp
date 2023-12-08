#define BLYNK_TEMPLATE_ID "TMPLLLP4q0VX"
#define BLYNK_TEMPLATE_NAME "Home Automation"
#define BLYNK_AUTH_TOKEN "rXIt7yxsC1lhZh8VYo_JzcNMJFTLKn-k"

#include <Arduino.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <jled.h>
#include <ArduinoOTA.h>
#include "OTA.h"
// #include <FS.h>

//******************************Setup JLed Modes***************************************
JLed blink = JLed(LED_BUILTIN).Blink(500, 500).Forever().LowActive();
JLed breathe = JLed(LED_BUILTIN).Breathe(1600).Forever().LowActive();
JLed On = JLed(LED_BUILTIN).On().LowActive();
JLed Off = JLed(LED_BUILTIN).Off().LowActive();
//*************************************************************************************

// *************************** WiFi Setup **********************************************
// Home WiFi
#define wifi_ssid  "SpottsWiFi"
#define wifi_pass  "Nspotts42194"
// Phone Hotspot
// #define wifi_ssid "natespott"
// #define wifi_pass "Nspotts42194"
int wifiTimeout = 8000;
// *************************************************************************************

// **************************** IFTTT Setup ********************************************
String Key = "ZOl-kiJkYQGJKLrC6TNHOjh3JOxtHXHL9ia7agmNjc";
String Firewood_Event = "Firewood_Event";
String Safety_Event = "No_Temp_Update";

WiFiClientSecure clientS;
const char* ifttthost =  "maker.ifttt.com";
const int httpsPort = 443;
// *************************************************************************************

// *********************************** Blynk Setup *************************************
#define BLYNK_PRINT Serial

#include <BlynkSimpleEsp8266.h>

// char auth[] = "f11aca9b143a4f65abfa450369e8ff4c";
// char temp_auth[] = "b41c806b93a74d558a2cbcc61c82b04a";
// WidgetBridge Bridge_to_LCD(V24);
BlynkTimer Timer; // can schedule up to 16 tasks

// *************************************************************************************

// ************************** PID Setup ***************************************
#include <PID_v1.h>

//Define Variables we'll be connecting to
double Setpoint, Input, Output;
long min_output = 0;
long max_output = 4350; // 4166

//Specify the links and initial tuning parameters
double Kp=250, Ki=2, Kd=0;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);
// ****************************************************************************

// ************************ Stepper Variables & Defaults***********************
// Easy Driver Output Pin Variables
#define DIR     D1
#define STP     D2
#define MS1     D3
#define MS2     D4
#define EN      D5
#define endstop D6

long count = 0;
long max_count = max_output;
long target_position;
long temp = 78;
long _setpoint = 78;

long currenttime = 0;

long home_time = 0;
long home_interval = 10;  // minutes
bool first_home = true;

long safety_timer = 0;
long safety_interval = 60*5; // seconds, how long without temperature signal
long safety_check_timer = 0;
long safety_check_int = 60*120; // seconds, how often to check and send trigger

long add_firewood_timer = 0;
long add_firewood_interval = 5;   // minutes 30
long firewood_print_timer = 0;
long firewood_print_interval = 1;    // minutes 20
bool first_max = true;
int firewood_notifications = 1;
double firewood_needed_time = 0;
long last_temp;

long current_damper_pos = 0;
long damper_position_threshold = 100;

bool reset_button_state = HIGH;
int reset_button_pin = D7;

// *****************************************************************************


//************************************* Setup File System ***********************************
// // flag for saving config parameters to SPIFFS
// bool shouldSaveConfig = false;

// // Custom Parameters for user to set and their default values - these default values show on Wifi Manager
// char param_1[40];  
// char param_2[40];
// char param_3[40];


// #include "File_System.h"
// ******************************************************************************************

#include "stepper_functions.h"
#include "Callbacks.h"
#include "functions.h"

BLYNK_CONNECTED() {
  syncPins();
}

void setup() {

  Serial.begin(115200);

  pinMode(DIR, OUTPUT);
  pinMode(STP, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(EN, OUTPUT);
  pinMode(endstop, INPUT_PULLUP);
  pinMode(reset_button_pin, INPUT_PULLUP);

  // SPIFFS.format();
  //Mount_SPIFFS();

  Input = temp;
  Setpoint = 78;
  myPID.SetOutputLimits(min_output, max_output);
  //turn the PID on
  myPID.SetMode(AUTOMATIC);

  // Blynk.begin(BLYNK_AUTH_TOKEN, wifi_ssid, wifi_pass, IPAddress(159,65,55,83), 80); // use blynk.cloud IP address, use port 80 or 8080
  // Blynk.begin(BLYNK_AUTH_TOKEN, wifi_ssid, wifi_pass, IPAddress(64,225,16,22), 8080); // use IP address of blynk.cloud, use port 80
  blynk_setup();
  printWifiStatus();

  OTA_Functions();
  // Blynk.virtualWrite(V4, _setpoint);
  Blynk.virtualWrite(V2, current_damper_pos);

  Timer.setInterval(1000L, syncPins);
}


void loop() {
  ArduinoOTA.handle();
  Blynk.run();
  currenttime = millis();
  myPID.Compute();
  Input = temp;
  Setpoint = _setpoint;
  Timer.run();
  
  manageBlynkConnection2();

  CheckButton();
  SafetyCheck();
  AddFirewood();
  }
