#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include "FS.h"

ESP8266WebServer server(80);

const   size_t    JSON_SIZE               = JSON_OBJECT_SIZE(5) + 130;
          
String ssid = "";
String password = "";
    

void handleRoot(){
  digitalWrite(2, HIGH);    
  StaticJsonDocument<JSON_SIZE> jsonConfig;
  
  File file = SPIFFS.open("/config.html", "r");
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }
  String htmlCode = file.readString();  
  file.close();

  File data = SPIFFS.open("/data.json", "r");
  
  if (!deserializeJson(jsonConfig, data)){        
    
    
    htmlCode.replace("#SSID#", String(ssid));
    htmlCode.replace("#PASSWORD#", String(password));
    
    server.send(200, "text/html", htmlCode);

    data.close();
    digitalWrite(2, LOW);
  }  
}

void handleSaveConfig(){
  digitalWrite(2, HIGH);

  StaticJsonDocument<JSON_SIZE> jsonConfig;
  
  File file = SPIFFS.open(F("/data.json"), "w+");

  if (file) {
    jsonConfig["SSID"]    = server.arg(0);
    jsonConfig["PASSWORD"]   = server.arg(1);

    serializeJsonPretty(jsonConfig, file);
    file.close();
  }

  File html = SPIFFS.open("/success.html", "r");
  if(!html){
    Serial.println("Failed to open file for reading");
    return;
  }
  String htmlCode = html.readString();  
  html.close();
  
  server.send(200, "text/html", htmlCode);
  digitalWrite(2, LOW); 
}

void setup()
{  
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  Serial.begin(9600);
  
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  StaticJsonDocument<JSON_SIZE> jsonConfig;  

  File data = SPIFFS.open("/data.json", "r"); 
     
  if (!deserializeJson(jsonConfig, data)){        
    ssid = (String) jsonConfig["SSID"];
    password = (String) jsonConfig["PASSWORD"];
  }  

  Serial.println("Verificando SSID");
  Serial.println(ssid);
  
  if(ssid != "" && password != ""){
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password); 
    Serial.print("Conectando");
    while (WiFi.status() != WL_CONNECTED){
      delay(500);
      Serial.print(".");
    }  
    Serial.print("\nConectado | Endereço IP: ");
    Serial.println(WiFi.localIP()); 
  } else {
    // Se não existir Cria uma rede
    WiFi.hostname("SpeedTestAP");
    WiFi.softAP("SpeedTestAP", "SpeedTesteAP");
  }  
  if(MDNS.begin("esp8266")){
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);   
  server.on("/saveConfig", handleSaveConfig);  
  server.begin();
  Serial.print("\nHTTP Seerver Started");
}

void loop()
{
  server.handleClient();    
}
