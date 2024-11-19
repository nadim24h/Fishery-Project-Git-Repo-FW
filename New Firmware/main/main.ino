#include <IRremote.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>

const char* ssid = "TESTING 2.4";
const char* password = "Virus1234";
const char* gsmApn = "internet"; // Change to your GSM provider's APN
const char* gsmUsername = "";
const char* gsmPassword = "";

#define LED_PIN 13
#define TEMP_SENSOR_PIN 14
#define IR_RECEIVER_PIN 19
#define RELAY1_PIN 32
#define RELAY2_PIN 33
#define GSM_RESET_PIN 5
#define GSM_TX_PIN 26
#define GSM_RX_PIN 27

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Motor states
bool motor1State = false;
bool motor2State = false;

// EEPROM addresses for motor states
#define MOTOR1_ADDR 0
#define MOTOR2_ADDR 1
#define CONNECTION_METHOD_ADDR 2 // New address for connection method

TaskHandle_t IRTaskHandle = NULL;
TaskHandle_t WiFiTaskHandle = NULL;
TaskHandle_t LCDTaskHandle = NULL;
TaskHandle_t GSMTaskHandle = NULL;

unsigned long wifiConnectedTime = 0; // Stores the time when Wi-Fi connects
unsigned long gsmConnectedTime = 0; // Stores the time when GSM connects

// Create software serial for GSM
SoftwareSerial gsmSerial(GSM_RX_PIN, GSM_TX_PIN);

// Connection method flag: 0 = Wi-Fi, 1 = GSM
bool connectionMethod = false;

void blinkLed()
{
  // Blink LED for 200ms
  digitalWrite(LED_PIN, HIGH);  // Turn on LED
  vTaskDelay(200 / portTICK_PERIOD_MS);
  digitalWrite(LED_PIN, LOW);   // Turn off LED
  vTaskDelay(200 / portTICK_PERIOD_MS);  // Delay before the next IR command processing
}
// Function to update the LCD display with connection status and motor states
void updateLCD()
{
  lcd.clear();
  lcd.setCursor(0, 0);

  // Display connection status and duration
  if (connectionMethod == 0 && WiFi.status() == WL_CONNECTED) // Wi-Fi mode
  {
    lcd.print("WiFi Connected");
  }
  else if (connectionMethod == 1 && gsmConnectedTime > 0) // GSM mode and connected
  {
    lcd.print("GSM Connected");
  }
  else // No connection, show motor status directly
  {
//    lcd.print("No Connection");
//    lcd.setCursor(0, 1);
//    lcd.print("M1: ");
//    lcd.print(motor1State ? "ON " : "OFF ");
//    lcd.print("M2: ");
//    lcd.print(motor2State ? "ON " : "OFF");

    // Debugging serial output to confirm motor status is being printed
    Serial.print("LCD Updated - Motor 1: ");
    Serial.print(motor1State ? "ON " : "OFF");
    Serial.print(", Motor 2: ");
    Serial.println(motor2State ? "ON " : "OFF");
  }
}

// Save motor states to EEPROM
void saveMotorState()
{
  EEPROM.write(MOTOR1_ADDR, motor1State);
  EEPROM.write(MOTOR2_ADDR, motor2State);
  EEPROM.commit(); // Ensure data is written
}

// Wi-Fi Task
void WiFiTask(void *pvParameters)
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connecting to WiFi...");
    lcd.setCursor(0, 0);
    lcd.print("Con.ing to WiFi");
    //    delay(1000);
  }
  Serial.println("Connected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  vTaskDelete(NULL); // Delete task after connecting
}

// GSM Task
void GSMTask(void *pvParameters)
{
  gsmSerial.begin(9600);
  //  delay(1000);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println("Connecting to GSM...");

  // Start communication with GSM
  gsmSerial.println("AT");  // Check GSM module
  //  delay(1000);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  gsmSerial.println("AT+CSQ");  // Check signal quality
  //  delay(1000);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  gsmSerial.println("AT+CGATT=1");  // Attach to GPRS
  //  delay(2000);
  vTaskDelay(2000 / portTICK_PERIOD_MS);

  Serial.println("GSM Connected!");

  updateLCD(); // Update LCD with GSM connection info
  vTaskDelete(NULL); // Delete task after connecting
}

// IR Remote Control Task with added debugging
void IRTask(void *pvParameters)
{
  Serial.println("IRTask started...");

  while (true)
  {
    if (IrReceiver.decode())  // Check if IR data is available
    {
      unsigned long command = IrReceiver.decodedIRData.command;
      if (command != 0)  //to skip false command
      {
        Serial.print("IR Command Received: ");
        Serial.println(command, HEX);  // Print command in HEX format for clarity
      }
      IrReceiver.resume(); // Prepare for the next value

      switch (command)
      {
        case 0x54: // IR code for Motor 1 toggle
          motor1State = !motor1State;
          digitalWrite(RELAY1_PIN, motor1State ? HIGH : LOW);
          blinkLed();
          saveMotorState(); // Save updated state
          updateLCD();
          Serial.println("Motor 1 toggled");
          //          vTaskDelay(500 / portTICK_PERIOD_MS);
          break;

        case 0x16: // IR code for Motor 2 toggle
          motor2State = !motor2State;
          digitalWrite(RELAY2_PIN, motor2State ? HIGH : LOW);
          blinkLed();
          saveMotorState(); // Save updated state
          updateLCD();
          Serial.println("Motor 2 toggled");
          //          vTaskDelay(500 / portTICK_PERIOD_MS);
          break;

        case 0x15: // IR code for toggling between Wi-Fi and GSM
          if (connectionMethod == 0) // Currently using Wi-Fi, switch to GSM
          {
            connectionMethod = 1; // Set method to GSM
            EEPROM.write(CONNECTION_METHOD_ADDR, connectionMethod); // Save to EEPROM
            EEPROM.commit();
            Serial.println("Switching to GSM connection...");
            // Start GSM task if not already started
            if (GSMTaskHandle == NULL)
            {
              xTaskCreate(GSMTask, "GSMTask", 4096, NULL, 1, &GSMTaskHandle);
            }
          }
          else // Currently using GSM, switch to Wi-Fi
          {
            connectionMethod = 0; // Set method to Wi-Fi
            EEPROM.write(CONNECTION_METHOD_ADDR, connectionMethod); // Save to EEPROM
            EEPROM.commit();
            Serial.println("Switching to Wi-Fi connection...");

            // Start Wi-Fi task if not already started
            if (WiFiTaskHandle == NULL)
            {
              xTaskCreate(WiFiTask, "WiFiTask", 4096, NULL, 1, &WiFiTaskHandle);
            }
          }
          blinkLed();
          updateLCD(); // Update LCD with new connection method
          vTaskDelay(500 / portTICK_PERIOD_MS);
          break;

        case 0x0:
          //          Serial.println("Unknown IR command");
          break;

        default:
          Serial.println("Unknown IR command");
      }
    }
    else
    {
      //      Serial.println("Waiting for IR signal...");
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Slight delay to avoid spamming the serial monitor
  }
}



// LCD Display Task - always show connection status and motor status
void LCDTask(void *pvParameters)
{
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Welcome to HTF");
  lcd.setCursor(2, 1);
  lcd.print("booting...");
  delay(3000);

  while (true)
  {
    lcd.clear();

    // Display Wi-Fi connection status if connected
    if (WiFi.status() == WL_CONNECTED && connectionMethod == 0)
    {
      lcd.setCursor(0, 0);
      lcd.print("WiFi-Connected ");
    }
    // Display GSM connection status if connected
    else if (connectionMethod == 1)
    {
      lcd.setCursor(0, 0);
      lcd.print("GSM-Connected");
    }
    // Display "No Connection" if neither Wi-Fi nor GSM is connected
    else
    {
      lcd.setCursor(0, 0);
      lcd.print("No-Connection");
    }

//    Always show motor status on the second line
    lcd.setCursor(0, 1);
    lcd.print("M1: ");
    lcd.print(motor1State ? "ON " : "OFF");
    lcd.print(" M2: ");
    lcd.print(motor2State ? "ON " : "OFF");

    // Log motor state to serial for debugging
    Serial.print("Motor 1: ");
    Serial.print(motor1State ? "ON " : "OFF");
    Serial.print(", Motor 2: ");
    Serial.println(motor2State ? "ON " : "OFF");

    // Delay to avoid rapid updates, set to 5 seconds for readability
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}


void setup()
{
  Serial.begin(115200);

  // Initialize EEPROM with size 2 bytes (one byte per motor state)
  EEPROM.begin(4);

  // Retrieve saved motor states from EEPROM
  motor1State = EEPROM.read(MOTOR1_ADDR);
  motor2State = EEPROM.read(MOTOR2_ADDR);
  connectionMethod = EEPROM.read(CONNECTION_METHOD_ADDR);

  // Initialize pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  delay(3000);

  // Initialize IR receiver
  IrReceiver.begin(IR_RECEIVER_PIN, ENABLE_LED_FEEDBACK);

  // Display the last connection method on the LCD
  updateLCD();  // Show last connection status on LCD

  // Start appropriate connection task (Wi-Fi or GSM) based on the last method
  if (connectionMethod == 0)  // Wi-Fi
  {
    Serial.println("Last connection was Wi-Fi.");
    xTaskCreate(WiFiTask, "WiFiTask", 4096, NULL, 1, &WiFiTaskHandle);
  }
  else  // GSM
  {
    Serial.println("Last connection was GSM.");
    xTaskCreate(GSMTask, "GSMTask", 4096, NULL, 1, &GSMTaskHandle);
  }

  // Create FreeRTOS tasks
  xTaskCreate(IRTask, "IRTask", 4096, NULL, 1, &IRTaskHandle);
  xTaskCreate(LCDTask, "LCDTask", 2048, NULL, 1, &LCDTaskHandle);

  // Apply saved motor states to relays
  digitalWrite(RELAY1_PIN, motor1State ? HIGH : LOW);
  digitalWrite(RELAY2_PIN, motor2State ? HIGH : LOW);
}

void loop()
{
  // All tasks are handled by FreeRTOS
}
