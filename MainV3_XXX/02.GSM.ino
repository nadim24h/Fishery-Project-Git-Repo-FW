void GSMSetup()
{
  //GSM SETUP
  // Set-up modem reset, enable, power pins
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);

  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);

  // Set GSM module baud rate and UART pins
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println("Initializing modem...");
  modem.restart();
  // Or, use modem.init() if you don't need the complete restart

  //String modemInfo = modem.getModemInfo();
  //SerialMon.print("Modem: ");
  //SerialMon.println(modemInfo);

  // Unlock your SIM card with a PIN if needed
  if (strlen(simPIN) && modem.getSimStatus() != 3 ) {
    modem.simUnlock(simPIN);
  }
}

void GPRSDisconnect()
{
  digitalWrite(MODEM_POWER_ON, LOW);

  /*if (modem.isGprsConnected()) {
    client.stop();
    Serial.println(F("Server disconnected"));

    modem.gprsDisconnect();
    Serial.println(F("GPRS disconnected"));
    }*/

}
void turnOffNetlight()
{
  Serial.println("Turning off SIM800 Red LED...");
  modem.sendAT("+CNETLIGHT=0");
}

void turnOnNetlight()
{
  Serial.println("Turning on SIM800 Red LED...");
  modem.sendAT("+CNETLIGHT=1");
}

void GSMAPIConnect()
{
  //String DeviceCode="A2001XBCW";
  String IsOn = "true";
  (!DeviceIsON) ? IsOn = "false" : IsOn = "true";

  String R1On = "true";
  (itsONRelay[1] == 0) ? R1On = "false" : R1On = "true";
  String R2On = "true";
  (itsONRelay[2] == 0) ? R2On = "false" : R2On = "true";

  String T1 = String(Celcius); //"30";
  String T2 = String(Celcius2);

  //Has any changes from the device?
  String HasLocallyChanges = "false";
  StateChangedLocally ? HasLocallyChanges = "true" : "false";

  Serial.print(F("Performing HTTP GET request... "));
  //const char resource[] = "/1.0/DC2/?Dc=A2001XBCW&IsOn=true&R1On=true&R2On=true&T1=30&T2=34";
  String resource = "/1.0/DC2/?Dc=" + DeviceCode + "&IsOn=" + IsOn + "&R1On=" + R1On + "&R2On=" + R2On + "&T1=" + T1 + "&T2=" + T2 + "&Hc=" + HasLocallyChanges + "&M=G&Cnt=" + RequestCount;
  //String resource = "/1.0/DeviceComm/?DeviceCode="+DeviceCode+"&IsOn="+IsOn+"&R1On="+R1On+"&R2On="+R2On+"&T1="+T1+"&T2="+T2+"&Hc="+HasLocallyChanges;
  //There is no more local changes after sending
  Serial.println(resource);

  int err = http.get(resource);
  if (err != 0) {
    Serial.println(F("failed to connect"));
    delay(10000);
    return;
  }

  int status = http.responseStatusCode();
  Serial.print(F("Response status code: "));
  Serial.println(status);
  if (!status) {
    delay(10000);
    return;
  }

  //Headers
  //SerialMon.println(F("Response Headers:"));
  //while (http.headerAvailable()) {
  //String headerName = http.readHeaderName();
  //String headerValue = http.readHeaderValue();
  //SerialMon.println("    " + headerName + " : " + headerValue);
  //}

  //int length = http.contentLength();
  //if (length >= 0) {
  //  SerialMon.print(F("Content length is: "));
  //  SerialMon.println(length);
  //}

  if (http.isResponseChunked()) {
    Serial.println(F("The response is chunked"));
  }

  String body = http.responseBody();
  Serial.println(F("Response GSM:"));
  Serial.println(body);
  StateChangedLocally = false;
  RequestCount++;
  APIResponseHandler(body);

}

void GSMConnect() {
  Serial.print("Waiting for network...");
  if (modem.isNetworkConnected()) {
    Serial.println("Network connected");
  } else if (!modem.waitForNetwork(240000L)) {
    Serial.println(" fail");
    delay(1000);
    return;
  }
  Serial.println(" OK");
  Serial.print(F("Connecting to APN: "));
  Serial.print(apn);

  if (modem.isGprsConnected()) {
    Serial.println("GPRS connected");
  } else if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    Serial.println(" fail");
    delay(10000);
    return;
  }
  Serial.println(" OK");

  Serial.print("Connecting to ");
  Serial.print(server);
  if (!client.connect(server, port)) {
    Serial.println(" fail");
    delay(10000);
    return;
  }
  Serial.println(" OK");

  GSMAPIConnect();

}
