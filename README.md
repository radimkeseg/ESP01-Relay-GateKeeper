# ESP01-Relay GateKeeper

## Setup

* Download this project either with a git checkout or press "Download as zip"
* Install the following librarys with your Arduino Library Manager in Sketch > Include Library > Manage Libraries...
 * WiFiManager (tzapo)
 * PubSub (mqtt)
 * ArduinoJson (JsonParser)

## Configure
<img width="347" height="403" alt="image" src="https://github.com/user-attachments/assets/6410ea4f-0626-4774-bd67-da337f7a09ac" />


## Example of interaction

MQTT
click the SETUP PIN -> ESP01 GPIO0
* payload ->  { click:1, sec: 1 }


## Components
* Esp8266 ESP-01
* ESP-01 Relay Shield

