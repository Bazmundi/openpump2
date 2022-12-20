/*
OpenPump
Copyright (C) 2022 Asterion Daedalus

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>
#include <esp_now.h>

#define PUMP_ON true;
#define PUMP_OFF false;
bool pumpState = PUMP_OFF;

// pump will either respond to a message or not and the pressence flag will be set if it successfully receives a message
#define PUMP_PRESENT true;
#define PUMP_UNAVAILABLE false;
bool pumpPresence = PUMP_UNAVAILABLE;

// message will either be herald message 'false' to find pump, or pedal down message 'true' to tell pump to toggle on/off
#define PEDAL_PRESSED true;
#define PUMP_CONNECT false;
typedef struct struct_message {
  bool pumpFlag_msg;
} struct_message;

struct_message pumpMessage;

#define PUMP_OFF false;
#define PUMP_ON true;
bool togglePumpFlag = !PEDAL_PRESSED;
bool togglePump = PUMP_OFF;

// setup led to blink
#define LED_PIN LED_BUILTIN
const int LED_ON = HIGH;
const int LED_OFF = LOW;

// callback to check on success of message send
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  
  Serial.print(" Pedal press received "); 
  
  memcpy(&pumpMessage, incomingData, sizeof(pumpMessage));

  if (not pumpMessage.pumpFlag_msg) {
    Serial.println( "Pedal first contact");
    digitalWrite(LED_PIN, LED_ON);
    delay(2000);
    digitalWrite(LED_PIN, LED_OFF);
  }
  else {
    Serial.println( "Pedal pressed");
    togglePumpFlag = !togglePumpFlag;
  } 
}


// start ESP-NOW
void InitESPNow(void) {
  //If the initialization was successful
  if (esp_now_init() == ESP_OK) {
    Serial.println("\nESPNow Init Success");
  }
  //If there was an error
  else {
    Serial.println("ESPNow Init Failed");
    ESP.restart();
  }
}

const int RELAY_ON = HIGH;
const int RELAY_OFF = LOW;
const int RELAY_ONE = 0;
const int RELAY_TWO = 1;
const int RELAY_THREE = 2;
const int RELAY_FOUR = 3;
const int  relays[]        = {GPIO_NUM_17,GPIO_NUM_16,GPIO_NUM_27,GPIO_NUM_14};

// setup device
void setup() {
  Serial.begin(115200);
  
  // setup access point
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();  
  
  Serial.print("\nMAC: ");
  Serial.println(WiFi.macAddress());
 
  // start ESP-NOW
  InitESPNow();

  esp_now_register_recv_cb(OnDataRecv);
  
  // pin modes
  pinMode(LED_PIN, OUTPUT);

  pinMode(relays[RELAY_ONE],OUTPUT);
  pinMode(relays[RELAY_TWO],OUTPUT);
  pinMode(relays[RELAY_THREE],OUTPUT);
  pinMode(relays[RELAY_FOUR],OUTPUT);
  
  // initial gpio states
  digitalWrite(LED_PIN, LED_OFF);

  digitalWrite(relays[RELAY_ONE],RELAY_OFF);
  digitalWrite(relays[RELAY_TWO],RELAY_OFF);
  digitalWrite(relays[RELAY_THREE],RELAY_OFF);
  digitalWrite(relays[RELAY_FOUR],RELAY_OFF);

}

void loop()
{
   
  if (togglePumpFlag) {

    togglePumpFlag = !togglePumpFlag;

    if (!togglePump) {
      togglePump = !togglePump;
      digitalWrite(relays[RELAY_ONE],RELAY_ON);
      delay(500);
      digitalWrite(relays[RELAY_TWO],RELAY_ON);
      digitalWrite(LED_PIN,LED_ON);
      Serial.println("Pump ON");
    } else {
      togglePump = !togglePump;
      digitalWrite(relays[RELAY_TWO],RELAY_OFF);
      delay(500);
      digitalWrite(relays[RELAY_ONE],RELAY_OFF);
      digitalWrite(LED_PIN,LED_OFF);
      Serial.println("Pump OFF");
    }
  }

}

