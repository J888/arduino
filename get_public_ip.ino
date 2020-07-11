#include <b64.h>
#include <HttpClient.h>
#include <string>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>

// Replace with your network credentials
const char* ssid     = "";
const char* password = "";
ESP8266WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80

void setup() {
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("Local IP address: ");
  Serial.println(WiFi.localIP());

//  if (MDNS.begin("esp8266")) {
//    Serial.println("MDNS responder started");
//  }

  server.on("/", []() {
    Serial.println("It works!!");
    server.send(200, "text/plain", "Turning LED on.");
  });
}

const int kNetworkTimeout = 30*1000; // Number of milliseconds to wait without receiving any data before we give up
const int kNetworkDelay = 1000; // Number of milliseconds to wait if no data is available before trying again

const char* retrievePubIpAddr(HttpClient http) {
  int err = 0;
  std::string publicIp;
  
  err = http.get("api.ipify.org", "/");
  if (err == 0)
  {
    Serial.println("startedRequest ok");

    err = http.responseStatusCode();
    if (err >= 0)
    {
      Serial.print("Got status code: ");
      Serial.println(err);

      err = http.skipResponseHeaders();
      if (err >= 0)
      {
        int bodyLen = http.contentLength();
        Serial.print("Content length is: ");
        Serial.println(bodyLen);
        Serial.println();
        Serial.println("Body returned follows:");
      
        // Now we've got to the body, so we can print it out
        unsigned long timeoutStart = millis();
        char c;
        int counter = 0;
        // Whilst we haven't timed out & haven't reached the end of the body
        while ( (http.connected() || http.available()) &&
               ((millis() - timeoutStart) < kNetworkTimeout) )
        {
            if (http.available())
            {
                c = http.read();
                publicIp += c;
                bodyLen--;
                // We read something, reset the timeout counter
                timeoutStart = millis();
            }
            else
            {
                // We haven't got any data, so let's pause to allow some to
                // arrive
                delay(kNetworkDelay);
            }
        }       
      }
      else
      {
        Serial.print("Failed to skip response headers: ");
        Serial.println(err);
      }
    }
    else
    {    
      Serial.print("Getting response failed: ");
      Serial.println(err);
    }
  }
  else
  {
    Serial.print("Connect failed: ");
    Serial.println(err);
  }
  http.stop();
  return publicIp.c_str();
}

bool gotPublicIp = false;
WiFiClient client;
HttpClient http(client);

void loop() {
  const char * publicIpAddr;

  if(!gotPublicIp) {
    publicIpAddr = retrievePubIpAddr(http);
    Serial.print("Public ip address retrieved and it is: ");
    Serial.println(publicIpAddr);
    gotPublicIp = true;
  }
}
