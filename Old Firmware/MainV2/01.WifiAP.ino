
typedef struct {
  const char* zone;
  const char* ntpServer;
  int8_t      tzoff;
} Timezone_t;

static const Timezone_t TZ[] = {
  //{ "Asia/Dubai", "asia.pool.ntp.org", 4 },
  //{ "Asia/Karachi", "asia.pool.ntp.org", 5 },
  { "Asia/Dhaka", "asia.pool.ntp.org", 6 },
  //{ "Asia/Jakarta", "asia.pool.ntp.org", 7 },
};

WebServer Server;
AutoConnect       Portal(Server);
AutoConnectConfig Config;       // Enable autoReconnect supported on v0.9.4
AutoConnectAux    Timezone;

void rootPage() {
  String  content =
    "<html>"
    "<head>"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
    "<script type=\"text/javascript\">"
    "setTimeout(\"location.reload()\", 1000);"
    "</script>"
    "</head>"
    "<body>"
    "<h2 align=\"center\" style=\"color:blue;margin:20px;\">Automated Fish</h2>"
    //"<h3 align=\"center\" style=\"color:gray;margin:10px;\">{{DateTime}}</h3>"
    //"<p style=\"text-align:center;\">Reload the page to update the time.</p>"
    "<p></p><p style=\"padding-top:15px;text-align:center\">" AUTOCONNECT_LINK(COG_24) "</p>"
    "</body>"
    "</html>";
  Server.send(200, "text/html", content);
}

void startPage() {
  // Retrieve the value of AutoConnectElement with arg function of WebServer class.
  // Values are accessible with the element name.
  // The /start page just constitutes timezone,
  // it redirects to the root page without the content response.
  Server.sendHeader("Location", String("http://") + Server.client().localIP().toString() + String("/"));
  Server.send(302, "text/plain", "");
  Server.client().flush();
  Server.client().stop();
}

void APsetup() {
  delay(1000);
  // Enable saved past credential by autoReconnect option,
  // even once it is disconnected.
  Config.autoReconnect = true;    // Attempt automatic reconnection.


  Config.apid = "Automated_Fish_" + DeviceCode;
  Config.psk = "";
  Config.ota = AC_OTA_BUILTIN;

  Portal.config(Config);


  if (WiFi.status() == WL_CONNECTED) {
    // Here to do when WiFi is connected.
  }
  else {
    // Here to do when WiFi is not connected.
  }
  // Behavior a root path of ESP8266WebServer.
  Server.on("/", rootPage);
  Server.on("/start", startPage);   // Set NTP server trigger handler

  // Establish a connection with an autoReconnect option.
  if (Portal.begin()) {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
    IPAddress = WiFi.localIP().toString();
  }
}

void APsetupReconnect() {
  delay(1000);
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
  }
  File file = SPIFFS.open("/ConType.txt", "w");
  String ConTyp = "1";
  if (!file) {
    Serial.println("Failed to open file for reading");
  } else
  {
    file.print(ConTyp);
    file.close();
  }

  // Enable saved past credential by autoReconnect option,
  // even once it is disconnected.
  Config.autoReconnect = true;    // Attempt automatic reconnection.

  Config.apid = "Automated_Fish_" + DeviceCode;
  Config.psk = "automatedfish";
  Config.ota = AC_OTA_BUILTIN;

  Config.immediateStart = true;
  //Config.autoSave = AC_SAVECREDENTIAL_NEVER;
  Portal.config(Config);

  // Behavior a root path of ESP8266WebServer.
  Server.on("/", rootPage);
  Server.on("/start", startPage);   // Set NTP server trigger handler

  // Establish a connection with an autoReconnect option.
  if (Portal.begin()) {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
    IPAddress = WiFi.localIP().toString();
  }
}

void APloop() {
  Portal.handleClient();
}

//Connect to API
void WifiFunctions()
{
  HTTPClient http;

  //Device Is ON
  String IsOn = "true";
  (!DeviceIsON) ? IsOn = "false" : IsOn = "true";

  //Relay 1 AND 2
  String R1On = "true";
  (itsONRelay[1] == 0) ? R1On = "false" : R1On = "true";
  String R2On = "true";
  (itsONRelay[2] == 0) ? R2On = "false" : R2On = "true";

  //Temperature
  String T1 = String(Celcius); //"30";
  String T2 = String(Celcius2);

  //Has any changes from the device?
  String HasChanges = "false";
  StateChangedLocally ? HasChanges = "true" : "false";

  String QueryString = "http://api.automatedfishfarm.com/1.0/DC2/?Dc=" + DeviceCode + "&IsOn=" + IsOn + "&R1On=" + R1On + "&R2On=" + R2On + "&T1=" + T1 + "&T2=" + T2 + "&Hc=" + HasChanges + "&M=W&Cnt=" + RequestCount;

  //There is no more local changes after sending
  Serial.println(QueryString);
  StateChangedLocally = false;

  http.begin(QueryString);
  int httpCode = http.GET();            //Send the request
  if (httpCode == 200)
  {
    String payload = http.getString();    //Get the response payload

    Serial.print("Wifi http code:");
    Serial.println(httpCode);   //Print HTTP return code

    Serial.print("Wifi Response: ");
    Serial.println(payload);    //Print request response payload

    APIResponseHandler(payload);
    RequestCount++;
    http.end();  //Close connection
  }
}
