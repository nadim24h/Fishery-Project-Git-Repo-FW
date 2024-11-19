#include <WiFi.h>
#include <HTTPClient.h>

int itsONRelay[] = {0, 0, 0, 0};
// Wi-Fi credentials
const char* ssid = "Nadim";
const char* password = "ssss1111";

// API details
const char* baseURL = "http://88.222.244.92:3000/api/v1";
const int deviceId = 1; // Device ID

void setup()
{
  Serial.begin(115200);

  // Connect to Wi-Fi
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  delay(2000);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected!");
  itsONRelay[1] = true;
  itsONRelay[2] = true;

  // Call the function to update relay states
  updateRelayStates(itsONRelay[1], itsONRelay[2]); // relay1 = true, relay2 = false
}

void loop()
{
  // No action in loop
}

void updateRelayStates(bool relay1, bool relay2)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;

    //Device Is ON
    //    String IsOn = "true";
    //    (!DeviceIsON) ? IsOn = "false" : IsOn = "true";

    //Relay 1 AND 2
    String R1On = "true";
    (itsONRelay[1] == 0) ? R1On = "false" : R1On = "true";
    String R2On = "true";
    (itsONRelay[2] == 0) ? R2On = "false" : R2On = "true";

    //Temperature
//    String T1 = String(Celcius); //"30";
//    String T2 = String(Celcius2);

    // Construct the URL
    String url = String(baseURL) + "/devices/" + String(deviceId);
    http.begin(url);

    // Set headers
    http.addHeader("Content-Type", "application/json");

//    // Create JSON payload
//    String jsonPayload =
//      "{\n\"relay1\": " + R1On +
//      ",\n\"relay2\": " + R2On +
//      "\n}";

    // Create JSON payload old format
    String jsonPayload =
      "{\n\"relay1\": " + R1On +
      ",\n\"relay2\": " + R2On +
      "\n}";

    // Print the JSON payload
    Serial.println("JSON Payload:");
    Serial.println(jsonPayload);

    // Send the PUT request
    int httpResponseCode = http.PUT(jsonPayload);

    // Check the response
    if (httpResponseCode > 0)
    {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      String response = http.getString();
      Serial.println("Response:");
      Serial.println(response);
    }
    else
    {
      Serial.print("Error on sending PUT: ");
      Serial.println(http.errorToString(httpResponseCode).c_str());
    }

    // End HTTP connection
    http.end();
  }
  else
  {
    Serial.println("Wi-Fi is not connected");
  }
}
