#include<SPI.h>
#include<LoRa.h>
#include<WiFi.h>
#include<Adafruit_MQTT.h>
#include<Adafruit_MQTT_Client.h>
#include<math.h>

// LoRa pins
#define SS 5 // SPI slave select pin
#define RST 14 // LoRa reset pin
#define DIO0 2 // LoRa DIO0 pin

// Buzzer pin
#define BUZZER_PIN 4  

// Cloud connection settings
#define server "io.adafruit.com"
#define port 1883
#define username "SnehilShrey1010"
#define key "API key of Adafruit IO Cloud"
WiFiClient esp;

Adafruit_MQTT_Client mqtt(&esp, server, port, username, key);
Adafruit_MQTT_Publish feed1(&mqtt, username "/feeds/soil");
Adafruit_MQTT_Publish feed2(&mqtt, username "/feeds/vibration");
Adafruit_MQTT_Publish feed3(&mqtt, username "/feeds/rain");
Adafruit_MQTT_Publish feed4(&mqtt, username "/feeds/displacement");
Adafruit_MQTT_Publish feed5(&mqtt, username "/feeds/pitch");
Adafruit_MQTT_Publish feed6(&mqtt, username "/feeds/roll");

void setup() {

  Serial.begin(115200);
  while(!Serial);
  Serial.println("Serial communication not begun."); //Print the sentence if any trouble in serial communication occurs
  
  LoRa.setPins(SS, RST, DIO0);

  while(!LoRa.begin(866E6)) {
    Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(0xF3);
  Serial.println("LORA INITIALIZED!");

  WiFi.begin("TIAwifi","#tia@12345");
  Serial.begin(115200);
  Serial.print("Connecting to");
  Serial.print("TIAwifi");

  while(WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println("CONNECTED TO MQTT!");
}

void loop() {

  int Packet = LoRa.parsePacket();
  if(Packet) {
    Serial.print("RECEIVED PACKET!");
  }

  String Value ="";
  while(LoRa.available()) {
    Value += (char)LoRa.read();
    Serial.println(Value);
  }

  String tokens[7]; // soil, vibration, rain, displacement, tilt
  int index = 0;

  int start = 0;
  for(int i = 0; i < Value.length(); i++) {
    if(Value[i] == ';') {
      tokens[index++] = Value.substring(start, i);
      start = i + 1;
    }
  }
  tokens[index++] = Value.substring(start);

  float soil = tokens[0].toInt();          
  float vibration = tokens[1].toInt();     
  float rain = tokens[2].toFloat();
  float displacement = tokens[3].toFloat();
  int tilt_x = tokens[4].toInt();
  int tilt_y = tokens[5].toInt();
  int tilt_z = tokens[6].toInt();

  float pitch = atan2(-tilt_x, sqrt((tilt_y * tilt_y) + (tilt_z * tilt_z))) * 180.0 / 3.141;
  float roll  = atan2(tilt_y, tilt_z) * 180.0 / 3.141;

  if(mqtt.connected()) {
    bool success = true;
    feed1.publish(soil);
    feed2.publish(vibration);
    feed3.publish(rain);
    feed4.publish(displacement);
    feed5.publish(pitch);
    feed6.publish(roll);

    if(success) {
      Serial.println("Data published SUCCESSFULLY!");
    }
    else {
      Serial.println("Trouble. Data NOT PUBLISHED!");
    }
  }

  if(soil > 30 || vibration == 1 || rain > 45 || displacement > 7 || pitch > 10 || roll > 10) {
    digitalWrite(BUZZER_PIN, HIGH);
  } 
  else {
    digitalWrite(BUZZER_PIN, LOW);
  }
}