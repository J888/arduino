#include <ArduinoJson.h>
#include <LedControl.h>
#include <b64.h>
#include <HttpClient.h>
#include <string>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>

// Replace with your network credentials
const char *ssid = "";
const char *password = "";
ESP8266WebServer server(80); // Create a webserver object that listens for HTTP request on port 80

int BUTTON = 10;

int DIN = 16;
int CS = 5;
int CLK = 4;

const uint64_t HEART_FRAMES[] = {
  0x0000081c3e7f7f36,
  0x000010387cfefe6c,
  0x0010387cfefe6c00,
  0x10387cfefe6c0000,
  0x081c3e7f7f360000,
  0x00081c3e7f7f3600,
  0x0000081c3e7f7f36,
  0x0000000000000000
};
const int HEART_FRAMES_LEN = sizeof(HEART_FRAMES)/8;

const uint64_t SENDING_FRAMES[] = {
  0x1800000000000000,
  0x0018000000000000,
  0x0000180000000000,
  0x0000001800000000,
  0x0000000018000000,
  0x0000000000180000,
  0x0000000000001800,
  0x0000000000000018,
  0x0000000000000000
};
const int SENDING_FRAMES_LEN = sizeof(SENDING_FRAMES)/8;

LedControl ledDisplay = LedControl(DIN, CLK, CS, 0);

void setup()
{
  Serial.begin(115200);

  ledDisplay.clearDisplay(0);
  ledDisplay.shutdown(0, false);
  ledDisplay.setIntensity(0, 10);
  

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("Local IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(BUTTON, INPUT);
}

const int kNetworkTimeout = 30 * 1000; // Number of milliseconds to wait without receiving any data before we give up
const int kNetworkDelay = 1000;        // Number of milliseconds to wait if no data is available before trying again

const void sendComm(HttpClient http)
{
  int err = 0;
  err = http.get("mini-comm-store.herokuapp.com", "/coms/send/627909b0-a7f5-4cad-8723-256d7b6a68ba");
  if (err == 0) {
    Serial.println("Starting sendCommunication");
    err = http.responseStatusCode();
    if (err >= 0)
    {
      Serial.print("Got status code: ");
      Serial.println(err);
    }
  }
}

const boolean getComm(HttpClient http, const char *url, const char *path)
{
  std::string res = "";
  int err = 0;
  err = http.get(url, path);
  if (err == 0)
  {
    Serial.println("Starting getComm");
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
        while ((http.connected() || http.available()) &&
               ((millis() - timeoutStart) < kNetworkTimeout))
        {
          if (http.available())
          {
            c = http.read();
            res += c;
            bodyLen--;
            // We read something, reset the timeout counter
            timeoutStart = millis();
          }
          else
          {
            delay(kNetworkDelay); // We haven't got any data, so let's pause to allow some to arrive
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
      Serial.print("Connect failed: ");
      Serial.println(err);
    }
  }
  http.stop();
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, res);
  boolean commRes = doc["comm"];
  Serial.print("comm = ");
  Serial.println(commRes);
  return commRes;
}

void displayImage(uint64_t image) {
  for(int i=0; i<8; i++) {
    byte row = (image >> i * 8) & 0xFF;
    for(int j=0; j < 8; j++) {
      ledDisplay.setLed(0, i, j, bitRead(row, j));
    }
  }
}

bool gotPublicIp = false;
WiFiClient client;
HttpClient http(client);

int temp = 0;
int i = 0;
int sendFramesCounter = 0;
boolean turnOnTheLights = false;
boolean playSendAnimation = false;
int fastCheckAgainLoops = 0;
void loop()
{
  int temp = digitalRead(BUTTON);
  if(temp == HIGH && !playSendAnimation) {
    Serial.println("Button is pressed. Sending comm");
    sendComm(http);
    playSendAnimation = true;
  }

  if(playSendAnimation) {
    displayImage(SENDING_FRAMES[sendFramesCounter]);
    if (++sendFramesCounter >= SENDING_FRAMES_LEN ) {
      sendFramesCounter = 0;
      playSendAnimation = false;
    }
    delay(100);
  }

  if(turnOnTheLights) {
    displayImage(HEART_FRAMES[i]);
    if (++i >= HEART_FRAMES_LEN ) {
      i = 0;
      turnOnTheLights = false;
      fastCheckAgainLoops = 5;
    }
  } else {
    if(!playSendAnimation) { // don't check for new comms while doing the "SEND" animation
      boolean gotMail = getComm(http, "mini-comm-store.herokuapp.com", "/coms/get/627909b0-a7f5-4cad-8723-256d7b6a68ba");
      if(gotMail) {
        Serial.println("Turning on the lights");
        turnOnTheLights = true;
      } 
    }
  }

  if(turnOnTheLights) { delay(200); } 
  else {
    if(!playSendAnimation) {
      if(fastCheckAgainLoops > 0) {
  //      delay(0);
        fastCheckAgainLoops = fastCheckAgainLoops - 1;
      } else {
        delay(5000); 
      } 
    }
  }
}

