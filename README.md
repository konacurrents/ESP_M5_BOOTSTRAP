# ESP_IOT Bootstrap Framework v1.0

This short program will bootstrap to our full ESP_IOT system via the OTA (Over the Air) update.
The users uses the Serial Monitor's input (at the top of the window). After typing in the ssid and password,
the user types "m5atom" and an OTA update will occur. 

From then on you will be in the ESP_IOT full code base.

The hope is compiling and running this small program will be easier (as all the obsure libraries aren't used, but some of them are). We will document that better as well.

The goal of ESP_IOT is to have a set of modules that can be included in the loop and setup. Adding them will add and manipulate
various IoT capabilities, while using the ESP_IOT framework to connect the various devices. The ESP-32 devices support bluetooth and 
wifi but have a limited user interface. The M5 based ESP-32 extends with a display, speaker, temperature, axcelerometor sensers. Other devices 
such as M5 V2 incorporate a camera with machine learning functionality.

[API Manual](https://github.com/konacurrents/SemanticMarkerAPI)

[More Docs](https://github.com/konacurrents/SemanticMarkerESP-32)

When incorporated with the network through WIFI and a pub/sub engine like MQTT, using JSON formatted messages, a powerful
collaboration supports the IoT suite of applications.

# Important Game Changing Features
Here are a some of the most important features:

1. WIFI Interface
2. MQTT Messaging
3. BLE (Bluetooth) interface
4. JSON formated messages (over MQTT or BLE)
5. OTA - Over the Air updates (insitufield updates)
6. Object Oriented Software Architecture (C and C++)
7. Extensible and Adaptable via compile flags (ifdef)
8. Group messaging - permission based shared threads
9. QR Codes (Semantic Marker) in displays for dynamic feedback
10. Node-Red cloud processing (http REST interface)

## Over the Air (OTA) updates

A quick update on OTA, this requires a "http" interface (vs https). This means 
there must be a server that supports http - which are becoming rare (as https is required for everything.)
 
We are using [kttp://KnowledgeShark.org](http://KnowledgeShark.org) for this.
