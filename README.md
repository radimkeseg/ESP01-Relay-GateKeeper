# ESP01-Relay GateKeeper

## Setup

* Download this project either with a git checkout or press "Download as zip"
* Install the following librarys with your Arduino Library Manager in Sketch > Include Library > Manage Libraries...
 * WiFiManager (tzapo)
 * PubSub (mqtt)
 * ArduinoJson (JsonParser)

## Example of interaction

MQTT
click the SETUP PIN -> ESP01 GPIO0
* payload ->  { click:1, sec: 1 }


## Components
* Esp8266 ESP-01
* ESP-01 Relay Shield

