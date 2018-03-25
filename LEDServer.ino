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

const String ID = "0";
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

  server.on("/id", HTTP_GET, handleIDGet);
  
  server.on("/", HTTP_POST, handlePost);
  server.on("/solid", HTTP_POST, handleSolidPost);

  server.begin();
  Serial.println("HTTP server started");

  char* colorStr = "FF0000";
  uint8_t rcolor =  strtoul (colorStr, NULL, 16) >> 16;
  uint8_t gcolor =  strtoul (colorStr+2, NULL, 16) >> 8;
  uint8_t bcolor =  strtoul (colorStr+4, NULL, 16);
//  uint32_t color = (uint32_t) strtoul (colorStr, NULL, 16);
  uint32_t ucolor = packRGB(gcolor,
                             rcolor,
                             bcolor);
  Serial.print("ucolor: ");
  Serial.println(ucolor);
  setColor(ucolor, 0, NUMPIXELS);
  
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

uint32_t packRGB(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

uint32_t packGRB(uint8_t g, uint8_t r, uint8_t b) {
  return ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;
}


void handleIDGet(){
  Serial.println("[GET] /id");
  server.send(200, "text/json", "{\"id\": "+ID+"}");
}

void handlePost(){
  server.send(200, "text/html", "All good");
  Serial.println("[POST] /");
  Serial.print("data: ");
  Serial.println(server.arg("data"));
  int head = 0;
  int mod = server.arg("mod") ? server.arg("mod").toInt() : 1;
  Serial.print("mod: ");
  Serial.println(mod);
  char* input = string2char(server.arg("data"));
  char* colorStr;
  while ((colorStr = strtok_r(input, DELIMITER, &input)) != NULL && head < NUMPIXELS){
    Serial.print("colorStr: ");
    Serial.println(colorStr);
    // RGB
//    uint32_t color = (uint32_t) strtoul (colorStr, NULL, 16);
     // ANYTHING ELSE
    uint8_t gcolor = (uint32_t) strtoul (colorStr, NULL, 16) >> 16;
    uint8_t rcolor = (uint32_t) strtoul (colorStr+2, NULL, 16) >> 8;
    uint8_t bcolor = (uint32_t) strtoul (colorStr+4, NULL, 16);
    uint32_t ucolor = packGRB(gcolor,
                              rcolor,
                              bcolor);
    setColor(ucolor, head, head+mod);
    head++;
  }
  strip.show();
}

void handleSolidPost(){
  server.send(200, "text/html", "All good");
  Serial.println("[POST] /solid");
  Serial.print("data: ");
  Serial.println(server.arg("data"));
  char* colorStr = string2char(server.arg("data"));
  Serial.print("colorStr: ");
  Serial.println(colorStr);
  // RGB
//  uint32_t color = (uint32_t) strtoul (colorStr, NULL, 16);
  
  // ANYTHING ELSE
  uint8_t rcolor = strtoul (colorStr, NULL, 16);
  uint8_t gcolor = strtoul (colorStr+2, NULL, 16);
  uint8_t bcolor = strtoul (colorStr+4, NULL, 16);
  uint32_t ucolor = packRGB(gcolor,
                            rcolor,
                            bcolor);
//  setColor(ucolor, 0, NUMPIXELS);
  setColorRGB(rcolor, gcolor, bcolor, 0, NUMPIXELS);
  strip.show();
}

void setColor(uint32_t color, int from, int to){
  int head = from;
  while (head < to && head < NUMPIXELS){
    Serial.print("ucolor: ");
    Serial.println(color);
    strip.setPixelColor(head, color);
    head++;
  }
}


void setColorRGB(uint8_t r, uint8_t g, uint8_t b, int from, int to){
  int head = from;
  while (head < to && head < NUMPIXELS){
    Serial.print("rcolor: ");
    Serial.println(r);
    Serial.print("gcolor: ");
    Serial.println(g);
    Serial.print("bcolor: ");
    Serial.println(b);
    strip.setPixelColor(head, g, r, b);
    head++;
  }
}

void loop(void){
  server.handleClient();
} 
