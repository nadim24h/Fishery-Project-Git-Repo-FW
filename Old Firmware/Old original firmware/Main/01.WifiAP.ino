 
/*static const char AUX_TIMEZONE[] PROGMEM = R"(
{
  "title": "TimeZone",
  "uri": "/timezone",
  "menu": true,
  "element": [
    {
      "name": "caption",
      "type": "ACText",
      "value": "Sets the time zone to get the current local time.",
      "style": "font-family:Arial;font-weight:bold;text-align:center;margin-bottom:10px;color:DarkSlateBlue"
    },
    {
      "name": "timezone",
      "type": "ACSelect",
      "label": "Select TZ name",
      "option": [],
      "selected": 10
    },
    {
      "name": "newline",
      "type": "ACElement",
      "value": "<br>"
    },
    {
      "name": "start",
      "type": "ACSubmit",
      "value": "OK",
      "uri": "/start"
    }
  ]
}
)";*/


typedef struct {
  const char* zone;
  const char* ntpServer;
  int8_t      tzoff;
} Timezone_t;

static const Timezone_t TZ[] = {
  //{ "Europe/London", "europe.pool.ntp.org", 0 },
  //{ "Europe/Berlin", "europe.pool.ntp.org", 1 },
  //{ "Europe/Helsinki", "europe.pool.ntp.org", 2 },
  //{ "Europe/Moscow", "europe.pool.ntp.org", 3 },
  //{ "Asia/Dubai", "asia.pool.ntp.org", 4 },
  //{ "Asia/Karachi", "asia.pool.ntp.org", 5 },
  { "Asia/Dhaka", "asia.pool.ntp.org", 6 },
  //{ "Asia/Jakarta", "asia.pool.ntp.org", 7 },
  //{ "Asia/Manila", "asia.pool.ntp.org", 8 },
  //{ "Asia/Tokyo", "asia.pool.ntp.org", 9 },
  //{ "Australia/Brisbane", "oceania.pool.ntp.org", 10 },
  //{ "Pacific/Noumea", "oceania.pool.ntp.org", 11 },
  //{ "Pacific/Auckland", "oceania.pool.ntp.org", 12 },
  //{ "Atlantic/Azores", "europe.pool.ntp.org", -1 },
  //{ "America/Noronha", "south-america.pool.ntp.org", -2 },
  //{ "America/Araguaina", "south-america.pool.ntp.org", -3 },
  //{ "America/Blanc-Sablon", "north-america.pool.ntp.org", -4},
  //{ "America/New_York", "north-america.pool.ntp.org", -5 },
  //{ "America/Chicago", "north-america.pool.ntp.org", -6 },
  //{ "America/Denver", "north-america.pool.ntp.org", -7 },
  //{ "America/Los_Angeles", "north-america.pool.ntp.org", -8 },
  //{ "America/Anchorage", "north-america.pool.ntp.org", -9 },
  //{ "Pacific/Honolulu", "north-america.pool.ntp.org", -10 },
  //{ "Pacific/Samoa", "oceania.pool.ntp.org", -11 }
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

  /*for (uint8_t n = 0; n < sizeof(TZ) / sizeof(Timezone_t); n++) {
    String  tzName = String(TZ[n].zone);
    if (tz.equalsIgnoreCase(tzName)) {
      configTime(TZ[n].tzoff * 3600, 0, TZ[n].ntpServer);
      Serial.println("Time zone: " + tz);
      Serial.println("ntp server: " + String(TZ[n].ntpServer));
      break;
    }
  }*/

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
  
  
  Config.apid="Automated_Fish_"+DeviceCode;
  Config.psk ="";
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
   if(!SPIFFS.begin(true)){
        Serial.println("An Error has occurred while mounting SPIFFS"); 
    }
    File file = SPIFFS.open("/ConType.txt","w");
    String ConTyp = "1";
    if(!file){
        Serial.println("Failed to open file for reading"); 
    }else
    {
      file.print(ConTyp);
      file.close();
    }
    
  // Enable saved past credential by autoReconnect option,
  // even once it is disconnected.
  Config.autoReconnect = true;    // Attempt automatic reconnection.
  //Config.reconnectInterval = 6;   // Seek interval time is 180[s].
  
  
  Config.apid="Automated_Fish_"+DeviceCode;
  Config.psk ="automatedfish";
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
    String IsOn="true";
    (!DeviceIsON)?IsOn="false":IsOn="true";

    //Relay 1 AND 2
    String R1On="true";
    (itsONRelay[1] == 0)?R1On="false":R1On="true";
    String R2On="true";
    (itsONRelay[2] == 0)?R2On="false":R2On="true";

    //Temperature
    String T1= String(Celcius);//"30";
    String T2=String(Celcius2);

    //Has any changes from the device?
    String HasChanges ="false";
    StateChangedLocally?HasChanges = "true":"false";
    
    String QueryString = "http://api.automatedfishfarm.com/1.0/DC2/?Dc="+DeviceCode+"&IsOn="+IsOn+"&R1On="+R1On+"&R2On="+R2On+"&T1="+T1+"&T2="+T2+"&Hc="+HasChanges+"&M=W&Cnt="+RequestCount;
    //String QueryString = "http://api.automatedfishfarm.com/1.0/DeviceComm/?DeviceCode="+DeviceCode+"&IsOn="+IsOn+"&R1On="+R1On+"&R2On="+R2On+"&T1="+T1+"&T2="+T2+"&Hc="+HasChanges;
    //String QueryString = "http://localhost:50883/1.0/DeviceComm/?DeviceCode="+DeviceCode+"&IsOn="+IsOn+"&R1On="+R1On+"&R2On="+R2On+"&T1="+T1+"&T2="+T2;
    //There is no more local changes after sending
    Serial.println(QueryString);
    StateChangedLocally = false;
    
    http.begin(QueryString);
    int httpCode = http.GET();            //Send the request
    if(httpCode == 200)
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
