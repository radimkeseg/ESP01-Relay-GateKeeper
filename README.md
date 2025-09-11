# ESP01-Relay GateKeeper

## Setup

* Download this project either with a git checkout or press "Download as zip"
* Install the following librarys with your Arduino Library Manager in Sketch > Include Library > Manage Libraries...
 * WiFiManager (tzapo)
 * PubSub (mqtt)
 * ArduinoJson (JsonParser)

## Example of interaction

MQTT
execute color effect
* payload -> { "color": "00ffff", sec : 5}

execute color effect and play song
* payload -> { "color": "00ffff", sec : 5, song: 1, volume: 30 }

click the SETUP PIN -> ESP01 GPIO0
* payload ->  { click:1, sec: 1 }


## Components
* Esp8266 ESP-01
* ESP-01 Relay Shield

