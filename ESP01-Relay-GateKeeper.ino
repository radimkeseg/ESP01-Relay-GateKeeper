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

//for now works well in case of 60 pixel strip only
//in other cases comment the line below
#define CLICK

#include "MyWifi.h"
MyWifi myWifi; 

#include "MyPubSub.h"
MyPubSub *myPubSub;

#include "Interval.h"

#include "Pubee.h"
Pubee pubee;

bool isInSetupMode = false;

//esp01 -> generic8266 flash:1M+128k SPIFF
#define SETUP_PIN 0      //GPIO0
#define TX 1             //LED_BUILTIN
#define RX 3
#define LED_STRIP 2
/*//node mcu test and debug
#define SETUP_PIN D7      //GPIO0
#define TX LED_BUILTIN
#define RX D7
*/
#define AP_NAME "GateKeeper"

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

static void publish(pubeeStates status){
  if(myWifi.getCustomSettings().settings.MQTT){
    if(pubee.setStatus(status)){
       myPubSub->reconnect();
       myPubSub->publish(("{ \"state\":\""+pubee.getStatus()+"\" }").c_str()); 
    }
  }
}

/* webserver handlers */
bool action = false;
Interval ActionInterval;

static String stFncHandleAction(){ 
  if(myWifi.getServer().arg("restart")!=NULL){
    myWifi.restart(1); //restart in 1 sec
  }
  
  return "OK";
}

#include <ArduinoJson.h>
static String stHandleSubCallback(char* topic, byte* payload, unsigned int length){
  StaticJsonDocument<1024> doc;  //should be set dynamic, maybe later 255 should be enough
  DeserializationError error = deserializeJson(doc, (char*)payload);

  if (error) {
    return "JSON ERROR";
  }

//pin click used to trigger automations e.g. relays
#ifdef CLICK
//click the SETUP PIN -> ESP01 GPIO0
//{ click:1, sec: 1 }
  int click = doc["click"];
  int duration = doc["duration"]; //in ms
  if(click>0){
    digitalWrite(SETUP_PIN, LOW); 
    delay(duration>0?duration:20);
  }
#endif

    int sec = doc["sec"];
    if(sec>0){
      action = true;
      ActionInterval.set( sec * 1000 );
    }

  return "OK";
}

/**/

void setup() {
  //Serial.begin(115200);

  myWifi.setup(AP_NAME,60); //1 min to configure the WIFI 
  myWifi.setActionHandler( stFncHandleAction );
  myWifi.getCustomSettings().print();

  if(myWifi.getCustomSettings().settings.MQTT){
    myPubSub = new MyPubSub(myWifi.getWifiClient(), myWifi.getCustomSettings().settings.MQTT_BROKER, myWifi.getCustomSettings().settings.MQTT_IN_TOPIC, myWifi.getCustomSettings().settings.MQTT_OUT_TOPIC );
    myPubSub->setCredentials(myWifi.getCustomSettings().settings.MQTT_DEVICE_ID, myWifi.getCustomSettings().settings.MQTT_USER, myWifi.getCustomSettings().settings.MQTT_PASSWORD);
    myPubSub->setHandleSubCallback( stHandleSubCallback );
    myPubSub->setup();
  }

#ifdef CLICK
  pinMode(SETUP_PIN, OUTPUT);
  digitalWrite(SETUP_PIN, HIGH); 
#endif  

  delay(100);  
  publish(RESTART);
}

void loop() {
  pubeeStates ps = IDLE;
  bool fast=false;  
  
  // Handle web server
  myWifi.handleClient();
  if(myWifi.getCustomSettings().settings.MQTT) myPubSub->handleClient();
  
  if(action){
    if(ActionInterval.expired()){
#ifdef CLICK
      digitalWrite(SETUP_PIN, HIGH);         
#endif
      action = false;
    }else{
      fast=true;
      ps = ACTION;
    }
  }

  if(fast)
    publish(ps);
  else
   publish(IDLE);
  delay(fast?100:200);
}
