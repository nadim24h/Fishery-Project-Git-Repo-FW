#include <IRremote.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>

#define EEPROM_SIZE 512 // Define EEPROM size
#define BOOT_COUNT_ADDR EEPROM_SIZE - 1 // Use the last address for the counter

// Wi-Fi credentials
const char* ssid = "Nadim";
const char* password = "ssss1111";
const char* gsmApn = "internet";

// Pin definitions
#define LED_PIN 13
#define TEMP_SENSOR_PIN 14
#define IR_RECEIVER_PIN 19
#define RELAY1_PIN 32
#define RELAY2_PIN 33
#define GSM_RESET_PIN 5
#define GSM_TX_PIN 26
#define GSM_RX_PIN 27

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Motor states and EEPROM addresses
bool motor1State = false;
bool motor2State = false;
#define MOTOR1_ADDR 0
#define MOTOR2_ADDR 1
#define CONNECTION_METHOD_ADDR 2

// Task handles
TaskHandle_t IRTaskHandle = NULL;
TaskHandle_t WiFiTaskHandle = NULL;
TaskHandle_t LCDTaskHandle = NULL;
TaskHandle_t GSMTaskHandle = NULL;

// Connection state and timing
bool connectionMethod = false;
unsigned long wifiConnectedTime = 0;
unsigned long gsmConnectedTime = 0;

// Software serial for GSM
SoftwareSerial gsmSerial(GSM_RX_PIN, GSM_TX_PIN);

// Last IR command for filtering
unsigned long lastCommand = 0;
unsigned long lastReceiveTime = 0;

// Function declarations
void blinkLed();
void updateLCD();
void saveMotorState();
void toggleConnectionMethod();
void processIRCommand(unsigned long command);

// reset EEPROM at the uploading time
void resetEEPROM()
{
  for (int i = 0; i < EEPROM_SIZE; i++)
  {
    EEPROM.write(i, 0);
  }
  EEPROM.write(BOOT_COUNT_ADDR, 1); // Set counter to initialized state
  EEPROM.commit();
  Serial.println("EEPROM has been reset.");
}

// LED blink function for IR feedback
void blinkLed()
{
  digitalWrite(LED_PIN, HIGH);
  vTaskDelay(200 / portTICK_PERIOD_MS);
  digitalWrite(LED_PIN, LOW);
  vTaskDelay(200 / portTICK_PERIOD_MS);
}

// Update LCD with connection status and motor states
void updateLCD()
{
  lcd.clear();
  lcd.setCursor(0, 0);

  if (connectionMethod == 0 && WiFi.status() == WL_CONNECTED)
  {
    lcd.print("WiFi Connected");
  }
  else if (connectionMethod == 1 && gsmConnectedTime > 0)
  {
    lcd.print("GSM Connected");
  }
  else
  {
    lcd.print("No Connection");
  }

  lcd.setCursor(0, 1);
  lcd.print("M1: ");
  lcd.print(motor1State ? "ON " : "OFF ");
  lcd.print("M2: ");
  lcd.print(motor2State ? "ON " : "OFF");
}

// Save motor states to EEPROM
void saveMotorState()
{
  EEPROM.write(MOTOR1_ADDR, motor1State);
  EEPROM.write(MOTOR2_ADDR, motor2State);
  EEPROM.commit();
}

// Wi-Fi connection task
void WiFiTask(void *pvParameters)
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connecting to WiFi...");
    lcd.setCursor(0, 0);
    lcd.print("Concting to WiFi");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  wifiConnectedTime = millis();
  Serial.println("Connected to WiFi!");
  vTaskDelete(NULL);
}

// GSM connection task
void GSMTask(void *pvParameters)
{
  gsmSerial.begin(9600);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  gsmSerial.println("AT");
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  gsmSerial.println("AT+CGATT=1");
  vTaskDelay(2000 / portTICK_PERIOD_MS);

  gsmConnectedTime = millis();
  Serial.println("GSM Connected!");
  updateLCD();
  vTaskDelete(NULL);
}

// IR Task with command filtering
void IRTask(void *pvParameters)
{
  Serial.println("IRTask started...");
  while (true)
  {
    if (IrReceiver.decode())
    {
      unsigned long command = IrReceiver.decodedIRData.command;

      if (command != 0 && command != 40 && command != lastCommand)
      {
        lastCommand = command;
        lastReceiveTime = millis();
        Serial.print("IR Command Received: ");
        Serial.println(command, HEX);
        processIRCommand(command);
        blinkLed();
      }
      IrReceiver.resume();
    }

    if (millis() - lastReceiveTime > 500)
    {
      lastCommand = 0;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// Process IR commands
void processIRCommand(unsigned long command)
{
  switch (command)
  {
    case 0x54: // Motor 1 toggle
      motor1State = !motor1State;
      digitalWrite(RELAY1_PIN, motor1State ? HIGH : LOW);
      saveMotorState();
      updateLCD();
      break;

    case 0x16: // Motor 2 toggle
      motor2State = !motor2State;
      digitalWrite(RELAY2_PIN, motor2State ? HIGH : LOW);
      saveMotorState();
      updateLCD();
      break;

    case 0x15: // Toggle Wi-Fi/GSM
      toggleConnectionMethod();
      break;

    default:
      Serial.println("Unknown IR command");
      break;
  }
}

// Toggle between Wi-Fi and GSM connections
void toggleConnectionMethod()
{
  connectionMethod = !connectionMethod;
  EEPROM.write(CONNECTION_METHOD_ADDR, connectionMethod);
  EEPROM.commit();
  updateLCD();

  if (connectionMethod == 0)
  {
    Serial.println("Switching to Wi-Fi...");
    if (WiFiTaskHandle == NULL)
    {
      xTaskCreate(WiFiTask, "WiFiTask", 4096, NULL, 1, &WiFiTaskHandle);
    }
  }
  else
  {
    Serial.println("Switching to GSM...");
    if (GSMTaskHandle == NULL)
    {
      xTaskCreate(GSMTask, "GSMTask", 4096, NULL, 1, &GSMTaskHandle);
    }
  }
}

// LCD task to update motor and connection status
void LCDTask(void *pvParameters)
{
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.print("Welcome to HTF");
  delay(3000);

  while (true)
  {
    updateLCD();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// Setup function
void setup()
{
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  // Check the boot counter
  int bootCount = EEPROM.read(BOOT_COUNT_ADDR);
  if (bootCount == 0) // Check if uninitialized
  {
    resetEEPROM();
  }

  motor1State = EEPROM.read(MOTOR1_ADDR);
  motor2State = EEPROM.read(MOTOR2_ADDR);
  connectionMethod = EEPROM.read(CONNECTION_METHOD_ADDR);

  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  IrReceiver.begin(IR_RECEIVER_PIN, ENABLE_LED_FEEDBACK);
  digitalWrite(RELAY1_PIN, motor1State ? HIGH : LOW);
  digitalWrite(RELAY2_PIN, motor2State ? HIGH : LOW);
  updateLCD();

  if (connectionMethod == 0)
  {
    xTaskCreate(WiFiTask, "WiFiTask", 4096, NULL, 1, &WiFiTaskHandle);
  }
  else
  {
    xTaskCreate(GSMTask, "GSMTask", 4096, NULL, 1, &GSMTaskHandle);
  }

  xTaskCreate(IRTask, "IRTask", 4096, NULL, 1, &IRTaskHandle);
  xTaskCreate(LCDTask, "LCDTask", 2048, NULL, 1, &LCDTaskHandle);
}

// Main loop
void loop()
{
  // Handled by FreeRTOS tasks
}
