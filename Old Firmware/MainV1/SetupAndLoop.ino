


void setup(void)
{
  Serial.begin(115200);
  //Serial.println("Setup");
  delay(500);


  if (UpdateConnectionModeSpiff) {
    UpdateConType(ConnectionMode);
  }
  //READING CONNECTION MODE FROM SPIFF
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    //return;
  }
  File file = SPIFFS.open("/ConType.txt");
  String ConTyp = "3";
  if (!file) {
    Serial.println("Failed to open file for reading");
  } else
  {
    while (file.available()) {
      ConTyp = char(file.read());
    }
    Serial.println("Spiff " + ConTyp);
    if (ConTyp == "3")
    {
      //NONE
      ConnectionMode = 3;
    }
    else if (ConTyp == "2")
    {
      //GSM
      ConnectionMode = 2;
    }
    else if (ConTyp == "1")
    {
      //WIFI
      ConnectionMode = 1;
    }
    file.close();
  }

  //INITIALIZING TEMPERATURE DATA
  Tsensor1.begin();//Temperature Sensor
  Tsensor2.begin();//Temperature Sensor 2

  //Init IR
  irrecv.enableIRIn(); // Start the receiver

  //INIT CLOCK
  Wire.begin();//Clock DS3231
  //IsRTCFound = rtc.begin();//Clock DS3231

  //LCD
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Initializing..");

  //RELAY
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(USER_LED, OUTPUT); // for LED
  digitalWrite(USER_LED, HIGH);


  //Initializing the IR functions for runtime connection type change
  InitTask2();
  //Need to call this from SPIFFS
  //SET CONNECTION TYPES
  if (ConTyp == "1")
  {
    Serial.println("ConTyp 1 WIFI");

    lcd.clear();
    lcd.print("Initializing");
    lcd.setCursor(0, 1);
    lcd.print("WIFI...");

    ConnectionMode = 1;
    APsetup();//WIFI AP SETUP
  }
  else if (ConTyp == "4")
  {
    Serial.println("ConTyp 4 Reconnect wifi");

    lcd.clear();
    lcd.print("Initializing");
    lcd.setCursor(0, 1);
    lcd.print("WIFI...");

    ConnectionMode = 1;
    APsetupReconnect();
  }
  else if (ConTyp == "2")
  {
    Serial.println("ConTyp 2 GPRS");

    lcd.clear();
    lcd.print("Initializing");
    lcd.setCursor(0, 1);
    lcd.print("GPRS...");

    ConnectionMode = 2;
    GSMSetup();
  }


  if (ConnectionMode == 1)
  {

  }

  //INITIALIZING THREADS
  //InitTasks();
  InitTask1();
  InitTask3();
}


void readTime() {
  lcd.clear();
  //const char* days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
  //const char* months[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

  // Initializes all values:
  int second = 0;
  int minute = 0;
  int hour = 0;
  //int weekday = 0;
  //int monthday = 0;
  //int month = 0;
  //int year = 0;

  //Wire.beginTransmission(DS1307);
  //Wire.write(byte(0));
  //Wire.endTransmission();
  //Wire.requestFrom(DS1307, 7);
  //second = bcdToDec(Wire.read());
  //minute = bcdToDec(Wire.read());
  //hour = bcdToDec(Wire.read());



  ////weekday = bcdToDec(Wire.read());
  ////monthday = bcdToDec(Wire.read());
  ////month = bcdToDec(Wire.read());
  ////year = bcdToDec(Wire.read());

  //PRINTING TIME
  //char LCD_Time[15];
  //lcd.clear();
  //lcd.setCursor(0, 0);
  //sprintf(LCD_Time, "Time: %02d:%02d:%02d", hour, minute, second);
  //Serial.println(LCD_Time);
  //lcd.print(LCD_Time);


  //PRINTING TEMPERATURE
  char LCD_Temp[16];
  char LCD_Temp2[16];
  if (Celcius < -90)
  {
    Celcius = 0;
  }
  if (Celcius2 < -90)
  {
    Celcius2 = 0;
  }
  //sprintf(LCD_Temp, "T1:%02.00f T2:%02.00f   ", Celcius, Celcius2);
  sprintf(LCD_Temp,  "Temp 1:%02.00f    ", Celcius);
  sprintf(LCD_Temp2, "Temp 2:%02.00f    ", Celcius2);

  lcd.setCursor(0, 0);
  lcd.print(LCD_Temp);

  lcd.setCursor(0, 1);
  lcd.print(LCD_Temp2);

  //lcd.backlight();
  //delay(1000);
  //lcd.noBacklight();

  /*
    const char* AMPM = 0;
    Serial.print(days[weekday-1]);
    Serial.print(" ");
    Serial.print(months[month-1]);
    Serial.print(" ");
    Serial.print(monthday);
    Serial.print(", 20");
    Serial.print(year);
    Serial.print(" ");
    if (hour > 12) {
    hour -= 12;
    AMPM = " PM";
    }
    else AMPM = " AM";
    //Serial.print(hour);
    //Serial.print(":");
    //sprintf(buffer, "%02d", minute);
    //Serial.print(buffer);
    Serial.println(AMPM);
    //lcd.print(snum);*/
}

void ShowRelayStatus()
{
  lcd.clear();

  if (Celcius < -90)
  {
    Celcius = 0;
  }
  if (Celcius2 < -90)
  {
    Celcius2 = 0;
  }

  lcd.setCursor(0, 0);

  char LCD_Relay[16];

  if (itsONRelay[1] == 1 &&  (Celcius < Relay1Limit))
  {
    sprintf(LCD_Relay, "R1:ON T1:%02.00f", Celcius);
    //lcd.print(LCD_Relay);
    //lcd.print("Relay 1: ON    ");
  }
  else {
    if (Celcius > Relay1Limit) {
      sprintf(LCD_Relay, "R1:OFF T1:%02.00f HT", Celcius);
    } else {
      sprintf(LCD_Relay, "R1:OFF T1:%02.00f", Celcius);
    }

    //lcd.print(LCD_Relay);
    //lcd.print("Relay 1: OFF   ");
  }
  lcd.print(LCD_Relay);


  lcd.setCursor(0, 1);
  char LCD_Relay2[16];

  if (itsONRelay[2] == 1 &&  (Celcius2 < Relay2Limit))
  {
    sprintf(LCD_Relay2, "R2:ON T2:%02.00f", Celcius2);
    //lcd.print(LCD_Relay2);
    //lcd.print("Relay 2: ON    ");
  }
  else {
    if (Celcius2 > Relay2Limit)
    {
      sprintf(LCD_Relay2, "R2:OFF T2:%02.00f HT", Celcius2);
    } else
    {
      sprintf(LCD_Relay2, "R2:OFF T2:%02.00f", Celcius2);
    }

    //lcd.print(LCD_Relay2);
    //lcd.print("Relay 2: OFF   ");
  }
  lcd.print(LCD_Relay2);

  UpdateRelayStatus();
}
void ShowConnectionType()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  if (ConnectionMode == 1)
  {
    lcd.print("Con Mode: Wifi");
  }
  else if (ConnectionMode == 2)
  {
    lcd.print("Con Mode: GSM");
  }
  else {
    lcd.print("Con Mode: None");
  }

  lcd.setCursor(0, 1);
  lcd.print(IPAddress);

}

void loop(void)
{
  //Serial.println("loop");

  //ShowCoreNumber("LOOP");

  if (DeviceIsON)
  {
    lcd.backlight();
    //readTime();
    //delay(3000);
    ShowRelayStatus();
    delay(4000);
    ShowConnectionType();
  } else
  {
    lcd.clear();
    lcd.noBacklight();
  }
  delay(4000);
  //ESP.restart();

}
