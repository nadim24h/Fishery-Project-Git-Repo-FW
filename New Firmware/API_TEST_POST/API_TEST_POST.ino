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
    sendHttpPostRequest();
}

void sendHttpPostRequest()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        http.begin("http://88.222.244.92:3000/api/v1/devices"); // Specify the URL for the POST request
        http.addHeader("Content-Type", "application/json");

        // JSON payload
        String jsonPayload = R"({
            "mac": "00:1B:44:11:3A:B9",
            "deviceId": "device-0001",
            "connectivityStatus": true,
            "connectionType": "wifi",
            "phone": "+8801516759718",
            "address": {
                "street": "123 Main St",
                "city": "Metropolis",
                "state": "NY",
                "zip": "10002"
            },
            "state": true,
            "scheduler": false,
            "userId": 1
        })";

        int httpResponseCode = http.POST(jsonPayload);

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
