typedef struct {
  const char* zone;
  const char* ntpServer;
  int8_t      tzoff;
} Timezone_t;

static const Timezone_t TZ[] = {
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
  //static const char *wd[7] = { "Sun","Mon","Tue","Wed","Thr","Fri","Sat" };
  //struct tm *tm;
  //time_t  t;
  //char    dateTime[26];

  //t = time(NULL);
  //tm = localtime(&t);
  //sprintf(dateTime, "%04d/%02d/%02d(%s) %02d:%02d:%02d.", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, wd[tm->tm_wday], tm->tm_hour, tm->tm_min, tm->tm_sec);
  //content.replace("{{DateTime}}", String(dateTime));

  Server.send(200, "text/html", content);
}

void startPage() {
  // Retrieve the value of AutoConnectElement with arg function of WebServer class.
  // Values are accessible with the element name.
  //String  tz = Server.arg("timezone");

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
  //Config.reconnectInterval = 6;   // Seek interval time is 180[s].


  Config.apid = "Automated_Fish_" + DeviceCode;
  Config.psk = "";
  Config.ota = AC_OTA_BUILTIN;

  //Config.immediateStart = true;
  //Config.autoSave = AC_SAVECREDENTIAL_NEVER;
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
  //Config.reconnectInterval = 6;   // Seek interval time is 180[s].


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
  //  Serial.println("*****DeviceIsON:" + DeviceIsON);
  //  Serial.println("*****      IsOn:" + IsOn);
  //  Serial.println("--------------------------------");
  (!DeviceIsON) ? IsOn = "false" : IsOn = "true";
  //  Serial.println("*****DeviceIsON:" + DeviceIsON);
  //  Serial.println("*****      IsOn:" + IsOn);

  //Relay 1 AND 2
  String R1On = "true";
  (itsONRelay[1] == 0) ? R1On = "false" : R1On = "true";
  String R2On = "true";
  (itsONRelay[2] == 0) ? R2On = "false" : R2On = "true";

  //Temperature
  String T1 = String(Celcius); //"30";
  String T2 = String(Celcius2);

  //Has any changes from the device?
  String HasLocallyChanges = "false";
  StateChangedLocally ? HasLocallyChanges = "true" : "false";

  // Construct the URL NADIM
  String url = String(baseURL) + "/devices/" + String(deviceId);
  Serial.print("url: ");
  Serial.println(url);
  http.begin(url);

  // Set headers+++
  http.addHeader("Content-Type", "application/json");

  //NADIM
  Serial.println("*****deviceState-IsOn: " + IsOn);
  Serial.println("*****deviceState-DeviceIsON: " + DeviceIsON);
  String jsonPayload =
    "{\n\"relay1\": " + R1On +
    ",\n\"relay2\": " + R2On +
    ",\n\"deviceState\": " + IsOn +
    "\n}";

  //There is no more local changes after sending
  Serial.print("jsonPayload ");
  Serial.println(jsonPayload);
  StateChangedLocally = false;
  Serial.print("StateChangedLocally: ");
  Serial.println(StateChangedLocally);
  Serial.print("HasLocallyChanges: ");
  Serial.println(HasLocallyChanges);

  int httpCode = 0;
  if (HasLocallyChanges == "true")
  {
    Serial.println("State-Changed-Locally");
    //    Send the PUT request
    httpCode = http.PUT(jsonPayload);     //Send the request-+++
    Serial.println("http.PUT ACTIVATED");
  } else
  {
    httpCode = http.GET();            //Send the request
    Serial.println("http.GET ACTIVATED");
  }
   //JSON RESPONSE
 /* Parsed JSON   :{"success":true,"message":"Device read successfully.","data":{"id":1,"mac":"AA:BB:CC:DD:EE:FF","deviceId":"device-01","connectivityStatus":true,
                 "connectivityType":"0","deviceState":false,"schedulerState":true,"phone":"+1234567890","address":"1234 Farm Road, Springfield","relay1":false,"relay2":false,
                 "exportTime":null,"createdBy":1001,"updatedBy":{"userId":1,"username":"admin","timestamp":1696161923},"deletedBy":null,"retrievedBy":1003,"createdAt":1696161923,
                 "updatedAt":1.731998e12,"deletedAt":null,"userId":1,"sensors":[{"id":1,"sensorState":true,"sensorType":"Oxygen","unit":"µmol/L","value":22.5,"minValue":null,
                 "maxValue":null,"createdBy":1,"updatedBy":null,"deletedBy":null,"retrievedBy":null,"createdAt":1.731347e12,"updatedAt":1.731586e12,"deletedAt":null,"deviceId":1},
                 {"id":2,"sensorState":true,"sensorType":"Temperature","unit":"Celsius","value":22.5,"minValue":15,"maxValue":30,"createdBy":1,
                 "updatedBy":{"userId":1,"timestamp":"2024-11-11T10:00:00Z","changeDescription":"Initial sensor setup"},"deletedBy":null,"retrievedBy":null,"createdAt":1.731347e12,
                 "updatedAt":1.731347e12,"deletedAt":null,"deviceId":1}],"user":{"id":1,"uuid":"67cbf27c-89b3-40e1-9759-a4f0a8419354","username":"Smart.Farming",
                 "password":"$2a$10$evQ5jsifa97myHggHvZJ0uP/sdgriYrM3UlKuu2i2FNuftBxPgosO","email":"smart.farming@gmail.com","firstName":"smart","lastName":"Farming",
                 "phone":"01839865200","profilePicture":"http://88.222.244.92:3000/uploads/profile-pictures/1731203178064-88450785.jpg","coverPicture":null,"dateOfBirth":null,
                 "address":[{"area":"K","city":"K","house":"U","state":"K","county":"","street":"O","country":"Bangladesh","zipCode":"6","landmark":"","latitude":"","createdAt":"",
                 "longitude":"","updatedAt":"","addressType":"","addressLabel":"default"}],"gender":null,"bio":null,"details":null,"currentLocation":null,"socialMediaLinks":null,
                 "userRole":"admin","preferredLanguage":null,"timeZone":null,"isActive":true,"otp":"1782","otpExpires":"2024-11-03T07:24:04.000Z","isVerified":true,
                 "registrationType":"emailPassword","accessToken":null,"createdBy":null,"updatedBy":null,"deletedBy":null,"retrievedBy":null,"createdAt":1.730617e12,
                 "updatedAt":1.73135e12,"deletedAt":null}},"meta":null,"statusCode":200}
                 */
                 
  if (httpCode > 0)
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
