#include <WiFi.h>
#include <HTTPClient.h>

// Wi-Fi credentials
const char* ssid = "TESTING 2.4";
const char* password = "Virus1234";

void setup()
{
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }

    Serial.println("Connected to WiFi");
    sendHttpGetRequest();
}

void sendHttpGetRequest()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        http.begin("http://88.222.244.92:3000/api/v1/devices/2"); // Specify the URL for the GET request 'device-0001'
        int httpResponseCode = http.GET(); // Send the GET request

        if (httpResponseCode > 0)
        {
            // Print the HTTP response code and response payload
            Serial.println("HTTP Response code: " + String(httpResponseCode));
            String response = http.getString();
            Serial.println("Response: " + response);
        }
        else
        {
            Serial.println("Error on HTTP request");
        }

        http.end(); // Close connection
    }
    else
    {
        Serial.println("WiFi Disconnected");
    }
}

void loop()
{
    // Nothing here
}
