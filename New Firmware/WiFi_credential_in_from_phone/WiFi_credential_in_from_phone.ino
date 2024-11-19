//#include <WiFi.h>
//#include <WebServer.h>
//#include <EEPROM.h>
//#include <DNSServer.h>
//
//#define EEPROM_SIZE 512
//#define SSID_ADDR 0
//#define PASS_ADDR 32
//
//// Access Point Credentials
//const char* apSSID = "Fishery Config";
//IPAddress local_IP(192, 168, 4, 1);
//IPAddress gateway(192, 168, 4, 1);
//IPAddress subnet(255, 255, 255, 0);
//
//DNSServer dnsServer;
//WebServer server(80);
//
//// Wi-Fi credentials storage
//char ssid[32] = "";
//char password[32] = "";
//
//// Function to reset EEPROM
//void resetEEPROM()
//{
//  for (int i = 0; i < EEPROM_SIZE; i++)
//  {
//    EEPROM.write(i, 0);
//  }
//  EEPROM.commit();
//  Serial.println("EEPROM reset done.");
//}
//
//// Save Wi-Fi credentials to EEPROM
//void saveWiFiCredentials(const String& ssidInput, const String& passInput)
//{
//  for (int i = 0; i < 32; i++)
//  {
//    EEPROM.write(SSID_ADDR + i, i < ssidInput.length() ? ssidInput[i] : 0);
//    EEPROM.write(PASS_ADDR + i, i < passInput.length() ? passInput[i] : 0);
//  }
//  EEPROM.commit();
//}
//
//// Handler for saving Wi-Fi credentials from the form
//void handleSave()
//{
//  String ssidInput = server.arg("ssid");
//  String passInput = server.arg("password");
//
//  saveWiFiCredentials(ssidInput, passInput);
//
//  server.send(200, "text/html", "<html><body><h2>Credentials saved! Rebooting...</h2></body></html>");
//  delay(2000);
//  ESP.restart();
//}
//
//// Display form for entering Wi-Fi credentials
////void handleRoot()
////{
////    String html = "<html><body><h2>Wi-Fi Configuration</h2>";
////    html += "<form action=\"/save\" method=\"POST\">";
////    html += "SSID: <input type=\"text\" name=\"ssid\"><br>";
////    html += "Password: <input type=\"text\" name=\"password\"><br>";
////    html += "<input type=\"submit\" value=\"Save\">";
////    html += "</form></body></html>";
////    server.send(200, "text/html", html);
////}
//void handleRoot()
//{
//    String html = "<html><head>";
//    html += "<style>";
//    html += "body { font-family: Arial, sans-serif; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; background-color: #f2f2f2; }";
//    html += ".container { text-align: center; padding: 20px; width: 90%; max-width: 400px; background-color: #ffffff; border-radius: 8px; box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.1); }";
//    html += "h2 { color: #333333; }";
//    html += "label { font-size: 14px; color: #666666; }";
//    html += "input[type='text'], input[type='password'] { width: 100%; padding: 10px; margin: 8px 0 16px 0; border: 1px solid #cccccc; border-radius: 4px; box-sizing: border-box; }";
//    html += "input[type='submit'] { background-color: #4CAF50; color: white; padding: 12px 20px; border: none; border-radius: 4px; cursor: pointer; font-size: 16px; }";
//    html += "input[type='submit']:hover { background-color: #45a049; }";
//    html += ".show-password { font-size: 14px; color: #666666; }";
//    html += "</style>";
//    html += "<script>";
//    html += "function togglePassword() {";
//    html += "  var passwordField = document.getElementById('password');";
//    html += "  if (passwordField.type === 'password') {";
//    html += "    passwordField.type = 'text';";
//    html += "  } else {";
//    html += "    passwordField.type = 'password';";
//    html += "  }";
//    html += "}";
//    html += "</script>";
//    html += "</head><body>";
//    html += "<div class='container'>";
//    html += "<h2>Wi-Fi Configuration</h2>";
//    html += "<form action=\"/save\" method=\"POST\">";
//    html += "<label for='ssid'>SSID:</label><br>";
//    html += "<input type=\"text\" id='ssid' name=\"ssid\" placeholder=\"Enter Wi-Fi SSID\"><br>";
//    html += "<label for='password'>Password:</label><br>";
//    html += "<input type=\"password\" id='password' name=\"password\" placeholder=\"Enter Wi-Fi Password\"><br>";
//    html += "<input type='checkbox' id='showPassword' onclick='togglePassword()'>";
//    html += "<label for='showPassword' class='show-password'> Show Password</label><br><br>";
//    html += "<input type=\"submit\" value=\"Save\">";
//    html += "</form>";
//    html += "</div></body></html>";
//    server.send(200, "text/html", html);
//}
//
//// Setup Wi-Fi AP mode with captive portal
//void setupWiFiAP()
//{
//  WiFi.softAPConfig(local_IP, gateway, subnet);
//  WiFi.softAP(apSSID);
//  Serial.println("AP Started: Fishery Config");
//
//  // Set up DNS for captive portal redirection
//  dnsServer.start(53, "*", local_IP);
//
//  server.on("/", handleRoot);
//  server.on("/save", handleSave);
//  server.begin();
//  Serial.println("Web server started");
//}
//
//// Try connecting to saved Wi-Fi credentials
//bool connectToSavedWiFi()
//{
//  // Load credentials from EEPROM
//  for (int i = 0; i < 32; i++)
//  {
//    ssid[i] = EEPROM.read(SSID_ADDR + i);
//    password[i] = EEPROM.read(PASS_ADDR + i);
//  }
//
//  if (strlen(ssid) > 0)
//  {
//    WiFi.begin(ssid, password);
//    Serial.print("Connecting to WiFi...");
//    unsigned long startAttemptTime = millis();
//
//    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000)
//    {
//      delay(500);
//      Serial.print(".");
//    }
//    return WiFi.status() == WL_CONNECTED;
//  }
//  return false;
//}
//
//void setup()
//{
//  Serial.begin(115200);
//  EEPROM.begin(EEPROM_SIZE);
//
//  // Reset EEPROM on first boot
//  if (EEPROM.read(511) == 0)
//  {
//    resetEEPROM();
//    EEPROM.write(511, 1);
//    EEPROM.commit();
//  }
//
//  // Try connecting with saved credentials, otherwise start AP with captive portal
//  if (!connectToSavedWiFi())
//  {
//    setupWiFiAP();
//  }
//  else
//  {
//    Serial.println("Connected to Wi-Fi!");
//  }
//}
//
//void loop()
//{
//  dnsServer.processNextRequest(); // Handle DNS for captive portal
//  server.handleClient();          // Handle web server requests
//}
