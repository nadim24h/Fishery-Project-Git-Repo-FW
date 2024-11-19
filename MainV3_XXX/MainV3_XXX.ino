
#include <HTTPClient.h> // added by NADIM

#include <OneWire.h>//Temp
#include <DallasTemperature.h> //Temp
#include <ArduinoJson.h>
//Infrared
#include <IRremote.h>

//Clock //LCD
#include <Wire.h>

// API details NADIM
const char* baseURL = "http://88.222.244.92:3000/api/v1";
const int deviceId = 1; // Device ID

//#include "RTClib.h"//Clock
const int DS1307 = 0x68; // Address of DS1307 see data sheets

//LCD
#include <LiquidCrystal_I2C.h>//LCD

//SPIFF
#include "SPIFFS.h"

//WIFI
#include <WiFi.h>
#include <WebServer.h>
#include <time.h>
#include <AutoConnect.h>

// added by NADIM
//Device Serial Number
String DeviceCode = "+1234567890";

//GPRS Setup
// Your GPRS credentials (leave empty, if missing)
const char apn[]      = "internet"; // Your APN
const char gprsUser[] = ""; // User
const char gprsPass[] = ""; // Password
const char simPIN[]   = ""; // SIM card PIN code, if any
// TTGO T-Call pin definitions FOR GPRS Module
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
#define I2C_SDA              21
#define I2C_SCL              22

// Set serial for debug console (to the Serial Monitor, default speed 115200)
//#define SerialMon Serial
// Set serial for AT commands (to the module)
#define SerialAT  Serial1

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800      // Modem is SIM800
#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb

// Define the serial console for debug prints, if needed
//#define TINY_GSM_DEBUG SerialMon
//#define DUMP_AT_COMMANDS

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
TinyGsm modem(SerialAT);

// Server details
const char server[] = "api.automatedfishfarm.com"; //"vsh.pp.ua";
const int  port = 80;
TinyGsmClient client(modem);
HttpClient http(client, server, port);

bool DeviceIsON = true;
int ConnectionMode = 1; //1 =Wifi;2=GSM;3=none
bool UpdateConnectionModeSpiff = false;
int RequestCount = 0;
String IPAddress = "";

//Multi Tasking
TaskHandle_t Task1;
TaskHandle_t Task2;
TaskHandle_t Task3;

//Temperature Sensor
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature Tsensor1(&oneWire);
float Celcius = 0.0;

//Temperature Sensor 2
#define ONE_WIRE_BUS_2 35
#define USER_LED 15

OneWire oneWire2(ONE_WIRE_BUS_2);
DallasTemperature Tsensor2(&oneWire2);
float Celcius2 = 0.0;

//Temperature MaxLimits
float Relay1Limit = 90;
float Relay2Limit = 90;

//IR
int RECV_PIN = 19;
IRrecv irrecv(RECV_PIN);
int codeType = -1;
decode_results results;
bool StateChangedLocally = false;

//LCD
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);


//Relay
#define RELAY_1 32//12
//2
#define RELAY_2 33//13
//15
int itsONRelay[] = {0, 0, 0, 0};

//General
void ShowCoreNumber(String Method)
{
  Serial.print(Method);
  Serial.print(" works at: ");
  Serial.println(xPortGetCoreID());
}

//Using it for time
byte decToBcd(byte val) {
  return ((val / 10 * 16) + (val % 10));
}
byte bcdToDec(byte val) {
  return ((val / 16 * 10) + (val % 16));
}


void UpdateRelayStatus()
{
  if ((itsONRelay[1] == 1) &&  (Celcius < Relay1Limit) )
  {
    digitalWrite(RELAY_1, HIGH);
  } else {
    digitalWrite(RELAY_1, LOW);
  }
  if ((itsONRelay[2] == 1) && (Celcius2 < Relay2Limit) )
  {
    digitalWrite(RELAY_2, HIGH);
  } else {
    digitalWrite(RELAY_2, LOW);
  }
}
void UpdateConType(int conType) {
  ConnectionMode = conType;
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");

  }
  File file = SPIFFS.open("/ConType.txt", "w");
  String ConTyp = "1";
  if (conType == 2)
  {
    ConTyp = "2";
  }
  else if (conType == 3)
  {
    ConTyp = "3";
  }

  if (!file) {
    Serial.println("Failed to open file for reading");

  } else
  {
    file.print(ConTyp);
    file.close();
  }
}
void APIResponseHandler(String payload)
{
  if (payload == "")
  {
    return;
  }

  //https://arduinojson.org/v5/assistant/
  const size_t capacity = 200 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + 60;
  DynamicJsonDocument doc(capacity);

  //const char* json = payload.c_str();//"{\"IsOn\":true,\"r1\":{\"IsOn\":true,\"TmLim\":0},\"r2\":{\"IsOn\":true,\"TmLim\":0}}";

  const char* json = payload.c_str();//"{"IsOn":1,       "dr1":{"IsOn":1,"TmLim":0},"dr2":{"IsOn":1,"TmLim":0}}";
  Serial.print("JSON Response: ");
  Serial.println(json);            // {"IsOn":1,"CT":1,"dr1":{"IsOn":1,"TmLim":0},"dr2":{"IsOn":1,"TmLim":0}}
  deserializeJson(doc, json);

  // Print the entire parsed JSON document
  Serial.print("Parsed JSON   :");
  serializeJson(doc, Serial);
  Serial.println("");

  //  int respIsOn = doc["deviceState"]; // 1
  int respIsOn = doc["data"]["deviceState"];
  int scheduleIsOn = doc["data"]["schedulerState"];
  //  int conType = doc["CT"]; // 1//2//3
  int conType = 1; // 1//2//3

  int dr1_IsOn = doc["relay1"]["IsOn"]; // 1
  float dr1_TmLim = doc["relay1"]["TmLim"]; // 0

  int dr2_IsOn = doc["relay2"]["IsOn"]; // 1
  float dr2_TmLim = doc["relay2"]["TmLim"]; // 0

  Relay1Limit = dr1_TmLim;
  Relay2Limit = dr2_TmLim;

  Serial.println("****API RESPONSE HANDLER****");
  Serial.print("r1: ");
  Serial.print(dr1_IsOn);
  Serial.print(" r2: ");
  Serial.println(dr2_IsOn);

  Serial.print("respIsOn: ");
  Serial.println(respIsOn);

  Serial.print("scheduleIsOn: ");
  Serial.println(scheduleIsOn);
  
  if (respIsOn == 1 )
  {
    DeviceIsON = true;
    Serial.println("Device Is ON**");
    digitalWrite(USER_LED, LOW);
    if (conType != ConnectionMode)
    {
      UpdateConType(conType);
      GPRSDisconnect();
      delay(500);
      ESP.restart();

    }
  }
  else
  {
    Serial.println("Device Is OFF*");
    digitalWrite(USER_LED, HIGH);
    DeviceIsON = false;
  }
  if (DeviceIsON)
  {
    if (dr1_IsOn == 1 )
    {
      Serial.println("API relay1 ON");
      itsONRelay[1] = 1;
    }
    else
    {
      Serial.println("API relay1 OFF");
      itsONRelay[1] = 0;
    }

    if (dr2_IsOn == 1 )
    {
      Serial.println("API relay2 ON");
      itsONRelay[2] = 1;
    } else
    {
      Serial.println("API relay2 OFF");
      itsONRelay[2] = 0;
    }
  }
  else
  {
    Serial.println("Device Is OFF");
    itsONRelay[1] = 0;
    itsONRelay[2] = 0;
  }

  UpdateRelayStatus();
}
