#pragma once
#include "wled.h"
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

class BluetoothSerialUsermod : public Usermod {
 
  public:

    void setup() {

      // Tell WLED to defer all WiFi Connectivity
      deferConnections = true;
      
      // Begin Bluetooth device with same name as server description
      SerialBT.begin(serverDescription);

      Serial.println("Bluetooth Serial Usermod setup complete");
      Serial.print("Bluetooth Name: ");
      Serial.println(serverDescription);
      Serial.print("Bluetooth Address: ");
      Serial.println("todo");
    }


    // Cleans up and lets WLED run WiFi normally.
    // Once called, this usermod is disabled until next boot
    void stop(){
      SerialBT.flush();
      SerialBT.end();
      deferConnections=false;
      Serial.println("Stop Bluetooth Serial and use WLED normally");
    }


    void loop() {

      // If Wifi Deferment has ended, do nothing
      if(!deferConnections){
        return;
      }

      // If not set up or if preset changes in first 5* seconds of boot, disable Bluetooth and end WiFi deferment
      unsigned long now = millis();
      if(now > 1000 && now < 6000 && (currentPreset != bootPreset || currentPreset == 0 && bootPreset == 0)){
        stop();
      }

      while (SerialBT.available() > 0)
      {
        yield();
        byte next = SerialBT.peek();

        // If 'e' is received, stop bluetooth and end WiFi deferment so WLED WiFi can start
        if (next == 'e'){
          stop();
          break;
        }

        else if (next == '{') { // JSON API
          bool verboseResponse = false;
          if (!requestJSONBufferLock(16)) return;
          DeserializationError error = deserializeJson(doc, SerialBT);
          if (error) {
            releaseJSONBufferLock();
            return;
          }
          verboseResponse = deserializeState(doc.as<JsonObject>());
          releaseJSONBufferLock();
        }
        SerialBT.read(); // Discard the byte
      }
    }

    uint16_t getId(){return USERMOD_ID_EXAMPLE;}
};
