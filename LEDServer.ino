#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <Adafruit_DotStar.h>
#include <SPI.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUMPIXELS 60
#define DATAPIN    13 // GPIO15 - MISO
#define CLOCKPIN   14 // GPIO14 - CLK
#define DELIMITER ";"
#define PORT 9999

Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DATAPIN, CLOCKPIN);
MDNSResponder mdns;

const String tableID = "1";
// Network creds
const char* ssid = "ssid";
const char* password = "pass";
String XIP = "";

ESP8266WiFiMulti WiFiMulti;
ESP8266WebServer server(PORT);

void setup(void){
  
  Serial.begin(115200); 
  Serial.println("");
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);
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
  
  // Get external IP
  while(XIP == ""){
    XIP = getExternalIP();
    delay(1000);
  }
  
  Serial.print("External IP address: ");
  Serial.println(XIP);
  
  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }

  server.on("/", HTTP_GET, handleGet);
  
  server.on("/", HTTP_POST, handlePost);
  server.on("/solid", HTTP_POST, handleSolidPost);

  server.begin();
  Serial.println("HTTP server started");

  //setup strip
  strip.begin();
  strip.show();
}

String getExternalIP() {
  // Get external facing IP
  // For convenience, to display on Serial
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin("http://api.ipify.org/");
    int httpCode = http.GET();
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        http.end();
        return payload;
      }
    } else {
      Serial.printf("GET to ipify failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  return "";
}

char* string2char(String command){
    if(command.length()!=0){
        char *p = const_cast<char*>(command.c_str());
        return p;
    }
}

void handleGet(){
  Serial.println("[GET]");
  server.send(200, "text/html", "TableID: "+tableID);
}

void handlePost(){
  server.send(200, "text/html", "All good");
  Serial.println("[POST] /");
  Serial.print("data: ");
  Serial.println(server.arg("data"));
  int head = 0;
  char* input = string2char(server.arg("data"));
  char* colorStr;
  while ((colorStr = strtok_r(input, DELIMITER, &input)) != NULL && head < NUMPIXELS){
    long color = strtoul (colorStr, NULL, 16);
    strip.setPixelColor(head, color);
    head++;
  }
  strip.show();
}

void handleSolidPost(){
  server.send(200, "text/html", "All good");
  Serial.println("[POST] /solid");
  Serial.print("data: ");
  Serial.println(server.arg("data"));
  int head = 0;
  char* colorStr = string2char(server.arg("data"));
  long color = strtoul (colorStr, NULL, 16);
  while (head < NUMPIXELS){
    strip.setPixelColor(head, color);
    head++;
  }
  strip.show();
}

void loop(void){
  server.handleClient();
} 
