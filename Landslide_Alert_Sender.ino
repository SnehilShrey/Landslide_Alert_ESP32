#include<SPI.h> //For communication between ESP32 and LoRa 
#include<LoRa.h> //To initialize LoRa for sending data
#include<Wire.h> //For communication with MPU6050 Sensor(Tilt calculation) using I2C
#include<Adafruit_MPU6050.h> //For initializing MPU6050 driver
#include<Adafruit_Sensor.h> //To interface sensor with MPU6050

#define SOIL_PIN 34 //Soil moisture Sensor
#define VIB_PIN 27 //Vibration Sensor
#define RAIN_PIN 35 //Rainfall Sensor
#define TRIG_PIN 26 //Ultrasonic trigger pin for displacement
#define ECHO_PIN 25 //Ultrasonic echo pin for displacemnt

#define SS 5 //LoRa slave select pin
#define RST 14 //LoRa reset pin
#define DIO0 2 //LoRa DIO0 pin

Adafruit_MPU6050 mpu;

void setup() {
  //Initializing the Serial Monitor
  Serial.begin(115200);

  LoRa.setPins(SS, RST, DIO0);
  //Initializing the Sensors
  pinMode(SOIL_PIN, INPUT);
  pinMode(VIB_PIN, INPUT);
  pinMode(RAIN_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  // Initializing the serial communication
  Wire.begin();
  //Initializing MPU6050
  if(!mpu.begin()) {
    Serial.println("MPU6050 NOT FOUND!");
  }
  Serial.println("MPU6050 Initialized!");
  //Initializing LoRa
  if(!LoRa.begin(866E6)) {
    Serial.println("LoRa NOT FOUND!");
    delay(500);
  }
  Serial.println("LoRa Initialized!");

  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa is synced!");
}

void loop() {
  int soilValue = analogRead(SOIL_PIN); //It will give analog value 
  float soilPercent = (soilValue / 4095.0) * 100; //It will map the value as percentage from 0 to 100.
  String soil = String(soilPercent,2);

  int vibValue = digitalRead(VIB_PIN); //It will give HIGH(1) if vibration detected, otherwise LOW(0)
  String vib = String(vibValue);

  int rainValue = analogRead(RAIN_PIN); //It will give analog value
  float rainPercent = (rainValue / 4095.0) * 100; //It will map the value as percentage from 0 to 100.
  String rain = String(rainPercent,2);

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  int distance = (duration * 0.034)/2;
  String shift = String(distance);

  sensors_event_t a, g, temp; //sensors_event_t=predefined structure, a=accelarometer, g=gyroscope, t=temperature
  mpu.getEvent(&a, &g, &temp); 
  float accX = a.acceleration.x;
  float accY = a.acceleration.y;
  float accZ = a.acceleration.z;
  String X = String(accX);
  String Y = String(accY);
  String Z = String(accZ);

  String Data = soil+","+vib+","+rain+","+shift+","+X+","+Y+","+Z+".";

  LoRa.beginPacket();
  LoRa.print(Data);
  LoRa.endPacket();

  Serial.println("Data Sent: " + Data);

  delay(10000);
}