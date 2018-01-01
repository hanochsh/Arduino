//#include "WiFi.h"
#include <ESP8266WebServer.h>  // reuired for Server opertaions

unsigned long _connectTimeout         = 0;
boolean       _tryWPS                 = false;

void setup() {
  Serial.begin(115200);
  //Init WiFi as Station, start SmartConfig
  WiFi.mode(WIFI_AP_STA);
  if (connectWifi("", "") == WL_CONNECTED)   {
    Serial.println("IP Address:");
    Serial.println(WiFi.localIP());
    //connected
  }
}

void loop() {
}

int connectWifi(String ssid, String pass) {
  Serial.println("Connecting as wifi client...");

  //fix for auto connect racing issue
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Already connected. Bailing out.");
    return WL_CONNECTED;
  }
  //check if we have ssid and pass and force those, if not, try with last saved values
  if (ssid != "") {
    WiFi.begin(ssid.c_str(), pass.c_str());
  } else {
    if (WiFi.SSID() != NULL) {
      Serial.println("Using last saved values, should be faster");
      //trying to fix connection in progress hanging
      //ETS_UART_INTR_DISABLE();
      //wifi_station_disconnect();
      //ETS_UART_INTR_ENABLE();

      WiFi.begin();
    } else {
      Serial.println("No saved credentials");
      //WiFi.mode(WIFI_STA); 
      //Added Smart config by ioCare
      WiFi.beginSmartConfig();
      //Wait for SmartConfig packet from mobile
      Serial.println("Waiting for SmartConfig.");
      while (!WiFi.smartConfigDone()) {
        delay(500);
        Serial.print(".");
      }
      //Wait for WiFi to connect to AP
      Serial.println("Waiting for WiFi");
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }

      Serial.println("WiFi Connected.");

      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());

    }
  }

  int connRes = waitForConnectResult();
  Serial.println ("Connection result: ");
  Serial.println ( connRes );
  //not connected, WPS enabled, no pass - first attempt
  if (_tryWPS && connRes != WL_CONNECTED && pass == "") {
    //startWPS();
    //should be connected at the end of WPS
    connRes = waitForConnectResult();
  }
  return connRes;
}

uint8_t waitForConnectResult() {
  if (_connectTimeout == 0) {
    return WiFi.waitForConnectResult();
  } else {
    Serial.println("Waiting for connection result with time out");
    unsigned long start = millis();
    boolean keepConnecting = true;
    uint8_t status;
    while (keepConnecting) {
      status = WiFi.status();
      if (millis() > start + _connectTimeout) {
        keepConnecting = false;
        Serial.println("Connection timed out");
      }
      if (status == WL_CONNECTED || status == WL_CONNECT_FAILED) {
        keepConnecting = false;
      }
      delay(100);
    }
    return status;
  }
}
