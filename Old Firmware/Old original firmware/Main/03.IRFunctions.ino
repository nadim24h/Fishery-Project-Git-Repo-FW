#define IR_code1  1086128383 //948321248 // code received from button 1
#define IR_code2  1086161023 //948321232 // code received from button 2
#define IR_code3  1086144703 //948321264 // code received from button 3
#define IR_code4  1086177343 //948321224 // code received from button 4
#define IR_code5  1086136543 //948321256 // code received from button 5
#define IR_code6  1086169183 //948321240 // code received from button 6
#define IR_code7  1086152863 // code received from button 7
#define IR_code8  1086185503 // code received from button 8
#define IR_code9  1086132463 // code received from button 9
#define IR_code0  1086165103 // code received from button 0

#define IR_power_button  1086138583

void RelayONOffBasedOnIR(unsigned int value)
{
  
  switch(value) {
   case IR_code1://TOOGLE RELAY 1
     if(DeviceIsON){
        StateChangedLocally =true;
        if(itsONRelay[1] == 1) {
            itsONRelay[1] = 0;
            Serial.println("Relay 1 OFF");
         } else {
             itsONRelay[1] = 1;
             Serial.println("Relay 1 ON");
         }
      }
      break;
      
    case IR_code2://TOOGLE RELAY 2
      if(DeviceIsON){
          StateChangedLocally = true;
          if(itsONRelay[2] == 1) {
            itsONRelay[2] = 0;
            Serial.println("Relay 2 OFF");
         } else { 
             itsONRelay[2] = 1;
             Serial.println("Relay 2 ON");
         }
      }
      break;
    case IR_code5: //WIFI
      if(DeviceIsON){
        Serial.println("5 pressed.");
         ConnectionMode = 1; //WIFI
         if(!SPIFFS.begin(true)){
              Serial.println("An Error has occurred while mounting SPIFFS");
              break;
          }
          File file = SPIFFS.open("/ConType.txt","w");
          String ConTyp = "1";
          if(!file){
              Serial.println("Failed to open file for reading"); 
              break;
          }else
          {
            file.print(ConTyp);
            file.close();
          }
          GPRSDisconnect();
          delay(500);
          ESP.restart();
      }
      break;
    case IR_code6: //GPRS
      if(DeviceIsON){
        Serial.println("6 pressed.");
         ConnectionMode = 2; //GSM
         if(!SPIFFS.begin(true)){
              Serial.println("An Error has occurred while mounting SPIFFS");
              break;
          }
          File file = SPIFFS.open("/ConType.txt","w");
          String ConTyp = "2";
          if(!file){
              Serial.println("Failed to open file for reading");
              break;
          }else
          {
            file.print(ConTyp);
            file.close();
          }
          ESP.restart();
      }
      break;
    case IR_code7: //NONE
      if(DeviceIsON){
          ConnectionMode = 3; //NONE
           if(!SPIFFS.begin(true)){
              Serial.println("An Error has occurred while mounting SPIFFS"); 
          }
          File file = SPIFFS.open("/ConType.txt","w");
          String ConTyp = "3";
          if(!file){
              Serial.println("Failed to open file for reading"); 
          }else
          {
            file.print(ConTyp);
            file.close();
          }
          GPRSDisconnect();
          delay(500);
          ESP.restart();
      }
      break;
     case IR_code8: //Wifi Reconnect
      if(DeviceIsON){
          ConnectionMode = 1; //WIFI
           if(!SPIFFS.begin(true)){
              Serial.println("An Error has occurred while mounting SPIFFS"); 
          }
          File file = SPIFFS.open("/ConType.txt","w");
          String ConTyp = "4";
          if(!file){
              Serial.println("Failed to open file for reading"); 
          }else
          {
            file.print(ConTyp);
            file.close();
          }
          GPRSDisconnect();
          delay(500);
          
          ESP.restart();
      }
      break;
    
    case IR_power_button:
      StateChangedLocally = true;
      if(DeviceIsON){
        DeviceIsON = false; 
        itsONRelay[1] = 0; 
        itsONRelay[2] = 0;
      }else{
        DeviceIsON = true;
      }
  }
  UpdateRelayStatus();
}

void IRReceiveFunction()
{
  if(irrecv.decode(&results) )
  {
      codeType = results.decode_type;
      if (codeType == NEC) {
        Serial.print("Received NEC: ");
        if (results.value == REPEAT) {
          // Don't record a NEC repeat value as that's useless.
          Serial.println("repeat; ignoring.");
          irrecv.resume();
          return;
        }
      } 
      else if (codeType == SONY) {
        Serial.print("Received SONY: ");
      } 
      else if (codeType == SAMSUNG) {
        Serial.print("Received SAMSUNG: ");
      }
      else if (codeType == PANASONIC) {
        Serial.print("Received PANASONIC: ");
      }
      else if (codeType == JVC) {
        Serial.print("Received JVC: ");
      }
      else if (codeType == RC5) {
        Serial.print("Received RC5: ");
      } 
      else if (codeType == RC6) {
        Serial.print("Received RC6: ");
      } 
      else {
        Serial.print("Unexpected codeType ");
        Serial.print(codeType, DEC);
        Serial.println("");
      }
      //Serial.println(results.value,HEX);
      unsigned int value = results.value;
      Serial.println(value);

      RelayONOffBasedOnIR(value);
      
      irrecv.resume();

      //ShowCoreNumber("IRReceiveFunction");
    }  
}
