#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>


// Replace with your network credentials
const char* ssid     = "";
const char* password = "";

bool lightOn = false;

// Set web server port number to 80
ESP8266WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80

// Variable to store the HTTP request
String header;

// Assign output variables to GPIO pins
const int output13 = 13;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  pinMode(output13, OUTPUT);
  digitalWrite(output13, LOW);

  delay(10);
  Serial.println('\n');

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
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/toggle", []() {
    if(lightOn == true) {
      digitalWrite(output13, LOW);
      lightOn=false;
    } else {
     digitalWrite(output13, HIGH); 
      lightOn=true;
    }
    server.send(200, "text/plain", "Turning LED on.");
  });

   server.on("/ledOff", [](){
    digitalWrite(output13, LOW);
    server.send(200, "text/plain", "Turning LED off.");
   });

  server.begin();
  Serial.println("HTTP server started");
}


int counter = 0;

void loop(void) {
  server.handleClient();
  MDNS.update();
}
