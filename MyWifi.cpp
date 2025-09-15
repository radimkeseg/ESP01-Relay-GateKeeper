#include "coredecls.h"
/**The MIT License (MIT)

Copyright (c) 2021 by Radim Keseg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "MyWifi.h"
#include "settings.h"
#include "embHTML.h"

boolean MyWifi::wifi_apply_hostname_(String hostname) {
  bool ret = wifi_station_set_hostname(const_cast<char *>(hostname.c_str()));
  return ret;
}

void MyWifi::handle_root()
{
  String content = FPSTR(PAGE_INDEX);

  if (cs.settings.MQTT) content.replace("{mqtt}", "checked='checked'");
  else content.replace("{mqtt}", "");
  
  content.replace("{mqtt_broker}", String(cs.settings.MQTT_BROKER).c_str() );
  content.replace("{mqtt_user}", String(cs.settings.MQTT_USER).c_str() );
  content.replace("{mqtt_password}", String(cs.settings.MQTT_PASSWORD).c_str() );
  content.replace("{mqtt_device_id}", String(cs.settings.MQTT_DEVICE_ID).c_str() );
  content.replace("{mqtt_otopic}", String(cs.settings.MQTT_OUT_TOPIC).c_str() );
  content.replace("{mqtt_itopic}", String(cs.settings.MQTT_IN_TOPIC).c_str() );
  
  server->send(200, "text/html", content);
} 


void MyWifi::handle_store_settings(){
  cs.settings.MQTT = server->arg("_mqtt").length()>0;       
  strncpy(cs.settings.MQTT_BROKER, server->arg("_mqtt_broker").c_str(), 256); 
  strncpy(cs.settings.MQTT_USER, server->arg("_mqtt_user").c_str(), 256); 
  strncpy(cs.settings.MQTT_PASSWORD, server->arg("_mqtt_password").c_str(), 256); 
  strncpy(cs.settings.MQTT_DEVICE_ID, server->arg("_mqtt_device_id").c_str(), 256); 
  strncpy(cs.settings.MQTT_OUT_TOPIC, server->arg("_mqtt_otopic").c_str(), 256); 
  strncpy(cs.settings.MQTT_IN_TOPIC, server->arg("_mqtt_itopic").c_str(), 256);   

  cs.print();          
  cs.write();
  server->send(200, "text/html", "OK - restart");
  
  restart(1);
} 

void MyWifi::handle_data(){
  if(fDataHandler!=NULL){
      server->send(200, "application/json", fDataHandler());
  }else{
      server->send(200, "application/json", "{}");
  }
}

void MyWifi::handle_action(){
  if(fActionHandler!=NULL){
      server->send(200, "text/html", fActionHandler());
  }else{
      server->send(200, "text/html", "NO ACTION");
  }
}

void MyWifi::setDataHandler(fncHandleData fDataHandler){
  this->fDataHandler = fDataHandler;
}

void MyWifi::setActionHandler(fncHandleAction fActionHandler){
  this->fActionHandler = fActionHandler;
}

void MyWifi::reset(unsigned int inSec){
    delay(inSec*1000);
    //reset and try again, or maybe put it to deep sleep
    pinMode(0, OUTPUT);   
    digitalWrite(0,HIGH);  //from some reason this has to be set before reset|restart https://github.com/esp8266/Arduino/issues/1017
    ESP.reset();
    delay(5000);  
}

void MyWifi::restart(unsigned int inSec){
  delay(inSec*1000);
  ESP.restart();
  esp_yield();
}

void MyWifi::forceManualConfig(const char* APname){
      if (!wifiManager->startConfigPortal(APname)) {
        restart(3);
      }  
}


//---------------------------------------------------------
String MyWifi::getIP(){
  char ipAddr[16]; //aaa.bbb.ccc.ddd
  IPAddress ip = WiFi.localIP();
  sprintf(ipAddr, "%d\.%d\.%d\.%d", ip[0], ip[1], ip[2], ip[3]);
  return String(ipAddr);
/*
  String s="";
  IPAddress ip = WiFi.localIP();
  for (int i=0; i<4; i++)
    s += i  ? "." + String(ip[i]) : String(ip[i]);
  return s;
*/
}
String MyWifi::getMAC(){
  return WiFi.macAddress();
}

bool MyWifi::isConnected(){
  return WiFi.status() == WL_CONNECTED;
}


//---------------------------------------------------------
void MyWifi::setup(const char* APname, int timeout_in_sec){
  // Uncomment for testing wifi manager
  //  wifiManager.resetSettings();
  //or use this for auto generated name ESP + ChipID

  //Manual Wifi
  //WiFi->begin(WIFI_SSID, WIFI_PWD);

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  wifiManager->setTimeout((timeout_in_sec<0)?180:timeout_in_sec);

  String hostname = ((APname==NULL)?String(HOSTNAME):String(APname)) + String("-") + String(ESP.getChipId(), HEX);
  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if(!wifiManager->autoConnect( hostname.c_str() ) ){
    //restart(); //no wifi mode - uncomment the restart() if endess retry is needed
  }else{     
      // OTA Setup
      WiFi.hostname(hostname);

      httpUpdater->setup(server, "/firmware", update_username, update_password );
    
    
      //user setting handling
      server->on("/", std::bind(&MyWifi::handle_root, this));
      server->on("/settings", std::bind(&MyWifi::handle_store_settings,this)); 
      server->on("/data",  std::bind(&MyWifi::handle_data,this)); 
      server->on("/action",std::bind(&MyWifi::handle_action,this)); 
    
      server->begin(); 
    
      mdns.addService("http", "tcp", 80); 
  }
  
  cs.init();
  cs.read();   
  cs.print();
}

void MyWifi::handleClient(){
  // Handle web server
  server->handleClient(); 
}

WiFiClient& MyWifi::getWifiClient(){
  if(wfclient==NULL) wfclient = new WiFiClient();
  return *wfclient;
}

ESP8266WebServer& MyWifi::getServer(){
  if(server==NULL) server = new ESP8266WebServer(80);
  return *server;  
}

CustomSettings& MyWifi::getCustomSettings(){
   return cs;
}
