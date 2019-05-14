// Probe Final
// change file to write every test
// change start operations/stall time based on needs

// Packages
#include<math.h>
#include<Servo.h>
#include<Stepper.h>
#include<DallasTemperature.h>
#include<OneWire.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include<Wire.h>
#include<SPI.h>
#include<MS5837.h>
#include<SD.h>

// SD Module
#define CS_pin 53

// Servo objects
Servo armservo;  
Servo midservo;  
Servo endservo; 
Servo cam;

// Temperature sensor
#define one_wire_pin 38
OneWire one_wire(one_wire_pin);
DallasTemperature temp_sensor(&one_wire);

// Pressure sensor
MS5837 pressure_sensor;

// Scoop arm
int armpos = 20;    // variable to store the servo position
int midpos = 20;
int endpos = 20;
int Steps2Take;
#define arm_ser 9
#define mid_ser 8
#define end_ser 7

// To stop system
int RUNNING = 1;

// Soil Collector
#define IN1 5
#define IN2 4
#define IN3 3
#define IN4 2
#define STEPS 32
Stepper small_stepper(STEPS, IN1, IN3, IN2, IN4);

// Camera
#define camera_ser 35

// Front Motors
#define motor11 22
#define motor12 23
#define motor21 25
#define motor22 24

// Back Motors
#define motor31 27
#define motor32 26
#define motor41 28
#define motor42 29

// Ballast
#define stp 43
#define dir 44
#define MS1 39
#define MS2 42
#define MS3 41
#define EN 40

// Accelerometer
#define LIS3DH_CLK 13
#define LIS3DH_MISO 12
#define LIS3DH_MOSI 11
#define LIS3DH_CS 10
Adafruit_LIS3DH accel = Adafruit_LIS3DH(LIS3DH_CS, LIS3DH_MOSI, LIS3DH_MISO, LIS3DH_CLK);

File probe_data;

void setup() {
  Serial.begin(9600);
  Serial.println("Probe Action");
  // SD card setup
  SD.begin(CS_pin);
  probe_data = SD.open("TEST_1.txt", FILE_WRITE);
  probe_data.println("PROBE DATA");
  
  // Drive motors
  pinMode(motor11, OUTPUT);
  pinMode(motor12, OUTPUT);
  pinMode(motor21, OUTPUT);
  pinMode(motor21, OUTPUT);
  pinMode(motor31, OUTPUT);
  pinMode(motor32, OUTPUT);
  pinMode(motor41, OUTPUT);
  pinMode(motor41, OUTPUT);
  
  // Arm motors
  armservo.attach(arm_ser); 
  midservo.attach(mid_ser);
  endservo.attach(end_ser);
  armservo.write(armpos); midservo.write(midpos); endservo.write(endpos);
  
  // Camera
  cam.attach(camera_ser);
  
  // Sensors
  Wire.begin();
  temp_sensor.begin();
  accel.begin();
  accel.setRange(LIS3DH_RANGE_4_G);
  pressure_sensor.init();
  pressure_sensor.setModel(MS5837::MS5837_30BA);
  pressure_sensor.setFluidDensity(1029);
  
  // Ballast
  pinMode(stp, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(MS3, OUTPUT);
  pinMode(EN, OUTPUT);
  
  // Ballast default
  digitalWrite(stp, LOW);
  digitalWrite(dir, LOW);
  digitalWrite(MS1, LOW);
  digitalWrite(MS2, LOW);
  digitalWrite(MS3, LOW);
  digitalWrite(EN, LOW);
}

float startTime = millis();
float reachedBottomTime;
boolean manualBottomReset = false;
void loop() {

  float timestamp = millis() - startTime;
  ReadWrite(timestamp);

  boolean reachedBottom = timestamp/1000 > 5;
  if(reachedBottom){
    resetBottomTime();
    float timeSinceBottom = (millis() - reachedBottomTime)/1000;
    if((timeSinceBottom >= 0) && (timeSinceBottom < 1)){
      camera();
    }
    
    if((timeSinceBottom >=15) && (timeSinceBottom < 25)){
      forwards();
    }
    
    if((timeSinceBottom >=25) && (timeSinceBottom < 30)){
      stopMotors();
    }
    if((timeSinceBottom >= 30) && (timeSinceBottom < 31)){
      camera();
    }
    if((timeSinceBottom >= 40) && (timeSinceBottom < 80)){
      arm();
    }
    
    if((timeSinceBottom >= 80) && (timeSinceBottom < 90)){
      forwards();
    }
    if((timeSinceBottom >= 80) && (timeSinceBottom < 85)){
      camera();
    }
    if((timeSinceBottom >=90) && (timeSinceBottom < 95)){
      stopMotors();
    }
    if((timeSinceBottom >= 95) && (timeSinceBottom < 135)){
      arm();
    }
    if((timeSinceBottom >= 135)){
      stopAll();
    }
  }

  //delay(20);
}

// Only allow one manual startTime reset
void resetBottomTime() {
  if (!manualBottomReset) {
    reachedBottomTime = millis();
  }

  manualBottomReset = true;
}

void ReadWrite(float timestamp){
  temp_sensor.requestTemperatures();
  float temperature = temp_sensor.getTempCByIndex(0);
  
//  accel.read();
//  sensors_event_t event;
//  accel.getEvent(&event);
//  float x_accel = event.acceleration.x; //m/s^2
//  float y_accel = event.acceleration.y; 
//  float z_accel = event.acceleration.z; 
  
  pressure_sensor.read();
  float pressure = pressure_sensor.pressure(); //mBa
  float depth = pressure_sensor.depth(); // m
  probe_data.print(timestamp/1000); probe_data.print("s");probe_data.print(" ");
  probe_data.print(temperature); probe_data.print("C"); probe_data.print(" ");
  probe_data.print(pressure); probe_data.print("mBa"); probe_data.print(" ");
  probe_data.print(depth); probe_data.print("m"); probe_data.println(" ");
  // Write to SD Card
}

void backwards(){
  digitalWrite(motor11, HIGH);
  digitalWrite(motor12, LOW);
  digitalWrite(motor21, HIGH);
  digitalWrite(motor22, LOW);
  digitalWrite(motor31, HIGH);
  digitalWrite(motor32, LOW);
  digitalWrite(motor41, HIGH);
  digitalWrite(motor42, LOW);

}

void forwards(){
  digitalWrite(motor11, LOW);
  digitalWrite(motor12, HIGH);
  digitalWrite(motor21, LOW);
  digitalWrite(motor22, HIGH);
  digitalWrite(motor31, LOW);
  digitalWrite(motor32, HIGH);
  digitalWrite(motor41, LOW);
  digitalWrite(motor42, HIGH);
 
}

void camera(){
  int angle = 1;
  int pos = 1;
  while(true){
    if(angle == 180) pos = -1;
    if(pos >0){
      cam.write(angle);
      delay(20);
      angle++;
    }
    if(pos < 0){
      cam.write(angle);
      delay(20);
      angle--;
    }
    if(angle == 0) break;
  }
}

void arm(){
  // put your main code here, to run repeatedly:
  //arm close
  for(armpos = 30; armpos <150; armpos+=1){
    armservo.write(armpos);
    delay(15);
  }
  delay(500);
  
  //mid close
  for(midpos = 10; midpos <30; midpos+=1){
    midservo.write(midpos);
    delay(15);
  }
  delay(1000);

  //scoop
  for(endpos = 100; endpos >30; endpos-=1){
    endservo.write(endpos);
    delay(15);
  }
  delay(500);

  //mid close
  for(midpos = 30; midpos <70; midpos+=1){
    midservo.write(midpos);
    delay(15);
  }
  delay(500);

  //arm up
  for(armpos = 150; armpos>50; armpos-=1){
    armservo.write(armpos);
    delay(15);
  }
  delay(500);

  //mid close
  for(midpos = 70; midpos <140; midpos+=1){
    midservo.write(midpos);
    delay(15);
  }
  delay(500);

  //scoop dump
  for(endpos = 30; endpos <130; endpos+=1){
    endservo.write(endpos);
    delay(15);
  }
  delay(500);

    //arm up
  for(armpos = 50; armpos>30; armpos-=1){
    armservo.write(armpos);
    delay(15);
  }
  delay(500);

  //mid open
  for(midpos = 140; midpos >10; midpos-=1){
    midservo.write(midpos);
    delay(30);
  }
  delay(500);

  small_stepper.setSpeed(500);
  Steps2Take  =  410;
  small_stepper.step(Steps2Take);
  delay(2000);
}

void stopMotors(){
  digitalWrite(motor11, HIGH);
  digitalWrite(motor12, HIGH);
  digitalWrite(motor21, HIGH);
  digitalWrite(motor22, HIGH);
  digitalWrite(motor31, HIGH);
  digitalWrite(motor32, HIGH);
  digitalWrite(motor41, HIGH);
  digitalWrite(motor42, HIGH);
}

void releaseBallast() {
  digitalWrite(EN, HIGH);
}

void stopAll() {
  // Stop drive motors
  stopMotors();

  // Release ballast
  releaseBallast();
  probe_data.close();

  // Detach all servos
  armservo.detach();  
  midservo.detach();
  endservo.detach();
  cam.detach();
//  probe_data = SD.open("test_1.txt");
//  while(probe_data.available()){
//    Serial.write(probe_data.read());
//  }
//  probe_data.close();
}
