#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <DNSServer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define EEPROM_SIZE 512
#define SSID_ADDR 0
#define PASS_ADDR 32
#define Boot_button 0

// Access Point Credentials
const char* apSSID = "Fishery Config";
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

DNSServer dnsServer;
WebServer server(80);

// Wi-Fi credentials storage
char ssid[32] = "";
char password[32] = "";

// Function to reset EEPROM
void resetEEPROM()
{
  for (int i = 0; i < EEPROM_SIZE; i++)
  {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  Serial.println("EEPROM reset done.");
}

// Save Wi-Fi credentials to EEPROM
void saveWiFiCredentials(const String& ssidInput, const String& passInput)
{
  for (int i = 0; i < 32; i++)
  {
    EEPROM.write(SSID_ADDR + i, i < ssidInput.length() ? ssidInput[i] : 0);
    EEPROM.write(PASS_ADDR + i, i < passInput.length() ? passInput[i] : 0);
  }
  EEPROM.commit();
}

// Handler for saving Wi-Fi credentials from the form
void handleSave()
{
  String ssidInput = server.arg("ssid");
  String passInput = server.arg("password");

  saveWiFiCredentials(ssidInput, passInput);

  server.send(200, "text/html", "<html><body><h2>Credentials saved! Rebooting...</h2></body></html>");
  delay(2000);
  ESP.restart();
}

// Display form for entering Wi-Fi credentials
void handleRoot()
{
  String html = "<html><head>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; background: linear-gradient(135deg, #a1c4fd, #c2e9fb); }";
  html += ".container { text-align: center; padding: 80px; width: 95%; max-width: 800px; background-color: #ffffff; border-radius: 25px; box-shadow: 0px 6px 50px rgba(0, 0, 0, 0.25); }";
  html += "h1 { color: #005662; font-size: 48px; margin-bottom: 20px; font-weight: 900; }";
  html += "h2 { color: #005662; font-size: 60px; margin-bottom: 60px; font-weight: 800; }";
  html += "label { font-size: 48px; color: #333333; margin-top: 60px; display: block; text-align: center; font-weight: 800; }";
  html += "input[type='text'], input[type='password'] { width: 100%; padding: 25px; margin: 16px 0 30px 0; border: 2px solid #b0bec5; border-radius: 10px; box-sizing: border-box; font-size: 36px; text-align: center; }";
  html += "input[type='submit'] { background-color: #0288d1; color: white; padding: 22px 36px; border: none; border-radius: 10px; cursor: pointer; font-size: 32px; font-weight: 600; transition: 0.3s ease; }";
  html += "input[type='submit']:hover { background-color: #0277bd; }";
  html += ".show-password { font-size: 32px; color: #555555; cursor: pointer; font-weight: 500; }";
  html += "input[type='checkbox'] { width: 60px; height: 60px; cursor: pointer; margin-right: 20px; vertical-align: middle; }";
  html += ".tooltip { position: relative; display: inline-block; }";
  html += ".tooltip .tooltiptext { visibility: hidden; width: 280px; background-color: #333; color: #fff; text-align: center; border-radius: 8px; padding: 10px; position: absolute; z-index: 1; bottom: 140%; left: 50%; margin-left: -140px; opacity: 0; transition: opacity 0.3s; font-size: 24px; }";
  html += ".tooltip:hover .tooltiptext { visibility: visible; opacity: 1; }";
  html += "</style>";
  html += "<script>";
  html += "function togglePassword() {";
  html += "  var passwordField = document.getElementById('password');";
  html += "  passwordField.type = (passwordField.type === 'password') ? 'text' : 'password';";
  html += "}";
  html += "</script>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>HiTech Fishery</h1>";
  html += "<h2>Wi-Fi Configuration</h2>";
  html += "<form action=\"/save\" method=\"POST\">";
  html += "<label for='ssid'>Wi-Fi Network (SSID):</label>";
  html += "<div class='tooltip'><input type=\"text\" id='ssid' name=\"ssid\" placeholder=\"Enter your Wi-Fi SSID\">";
  html += "<span class='tooltiptext'>Enter the name of your Wi-Fi network</span></div><br>";
  html += "<label for='password'>Password:</label>";
  html += "<div class='tooltip'><input type=\"password\" id='password' name=\"password\" placeholder=\"Enter your Wi-Fi Password\">";
  html += "<span class='tooltiptext'>Enter the password for the Wi-Fi network</span></div><br>";
  html += "<input type='checkbox' id='showPassword' onclick='togglePassword()'>";
  html += "<label for='showPassword' class='show-password'> Show Password</label><br><br>";
  html += "<input type=\"submit\" value=\"Save & Connect\">";
  html += "</form>";
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}




// Try connecting to saved Wi-Fi credentials
bool connectToSavedWiFi()
{
  // Load credentials from EEPROM
  for (int i = 0; i < 32; i++)
  {
    ssid[i] = EEPROM.read(SSID_ADDR + i);
    password[i] = EEPROM.read(PASS_ADDR + i);
  }

  if (strlen(ssid) > 0)
  {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi...");
    unsigned long startAttemptTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000)
    {
      delay(500);
      Serial.print(".");
    }
    return WiFi.status() == WL_CONNECTED;
  }
  return false;
}

// Wi-Fi task to either connect to saved Wi-Fi or set up access point mode
void WiFiTask(void *parameter)
{
  if (!connectToSavedWiFi())
  {
    WiFi.softAPConfig(local_IP, gateway, subnet);
    WiFi.softAP(apSSID);
    Serial.println("AP Started: Fishery Config");

    dnsServer.start(53, "*", local_IP);
  }
  else
  {
    Serial.println("Connected to Wi-Fi! ");
  }
  vTaskDelete(NULL); // Delete task after Wi-Fi setup
}

// Web server task to handle client requests
void WebServerTask(void *parameter)
{
  server.on("/", handleRoot);
  server.on("/save", handleSave);
  server.begin();
  Serial.println("Web server started");

  while (true)
  {
    dnsServer.processNextRequest();
    server.handleClient();
    vTaskDelay(10 / portTICK_PERIOD_MS); // Avoid task hogging CPU
  }
}

void setup()
{
  Serial.begin(115200);
  delay(4000);

  EEPROM.begin(EEPROM_SIZE);
  pinMode(Boot_button, INPUT);
  
  delay(1000);
  bool buttonState = true;
  buttonState = digitalRead(Boot_button);
  Serial.print("\nbuttonState: ");
  Serial.println(buttonState);

  // Reset EEPROM on first boot
  if (buttonState == false)
  {
    resetEEPROM();
    EEPROM.commit();

  }

  // Create FreeRTOS tasks for Wi-Fi connection and web server
  xTaskCreate(WiFiTask, "WiFiTask", 4096, NULL, 1, NULL);
  xTaskCreate(WebServerTask, "WebServerTask", 8192, NULL, 1, NULL);
}

void loop()
{
  // loop is empty as tasks are running independently
}
