// Robotic Probe

#include<math.h>
#include<Servo.h>
#include<Stepper.h>

Servo armservo;  // create servo object to control a servo
Servo midservo;  // twelve servo objects can be created on most boards
Servo endservo;
Servo cam;
#define arm_ser 33
#define mid_ser 34
#define end_ser 35
#define camera_ser 36

int armpos = 0;    // variable to store the servo position
int midpos = 0;
int endpos = 0;
int x=0;

// Motors
#define motor11 22
#define motor12 23
#define motor21 25
#define motor22 24
int RUNNING = 1;

// Ballast
#define stp 26
#define dir 27
#define MS1 28
#define MS2 29
#define MS3 30
#define EN 31

void setup(){
  // Drive motors
  pinMode(motor11, OUTPUT);
  pinMode(motor12, OUTPUT);
  pinMode(motor21, OUTPUT);
  pinMode(motor21, OUTPUT);
  // Arm motors
  armservo.attach(arm_ser);  // attaches the servo on pin 9 to the servo object
  midservo.attach(mid_ser);
  endservo.attach(end_ser);
  // Camera
  cam.attach(camera_ser);
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


void loop(){
  if (RUNNING) {
    forwards(5000);
    camera();
    arm();
    backwards(5000);
    // digitalWrite(EN, HIGH);
    releaseBallast();
    stopAll();
    RUNNING = 0;
  }
}

void forwards(int duration){
  digitalWrite(motor11, HIGH);
  digitalWrite(motor12, LOW);
  digitalWrite(motor21, HIGH);
  digitalWrite(motor22, LOW);

  delay(duration);

  stopMotors();
}

void backwards(int duration){
  digitalWrite(motor11, LOW);
  digitalWrite(motor12, HIGH);
  digitalWrite(motor21, LOW);
  digitalWrite(motor22, HIGH);

  delay(duration);

  stopMotors();
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
    for (armpos = 50; armpos <= 180; armpos += 1) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      armservo.write(armpos);              // tell servo to go to position in variable 'pos'
      delay(22);
      // waits 15ms for the servo to reach the position
    }
    for (midpos = 120; midpos >= 50; midpos -= 1) {
      midservo.write(midpos);
      delay(22);
      Serial.println(midpos);
    }


    endservo.write(170);
    delay(500);
    for (midpos = 150; midpos >= 120; midpos -= 1) { // goes from 180 degrees to 0 degrees
      midservo.write(midpos);              // tell servo to go to position in variable 'pos'
      delay(22);
      Serial.println(midpos);// waits 15ms for the servo to reach the position
    }
    for (armpos = 180; armpos >= 50; armpos -= 1) { // goes from 180 degrees to 0 degrees
      armservo.write(armpos);              // tell servo to go to position in variable 'pos'
      delay(22);
      // waits 15ms for the servo to reach the position
    }
    endservo.write(90);
}

void stopMotors(){
  digitalWrite(motor11, HIGH);
  digitalWrite(motor12, HIGH);
  digitalWrite(motor21, HIGH);
  digitalWrite(motor22, HIGH);
}

void stopAll() {
  // Stop drive motors
  stopMotors();

  // Release ballast
  releaseBallast();

  // Detach all servos
  armservo.detach();  // attaches the servo on pin 9 to the servo object
  midservo.detach();
  endservo.detach();
  cam.detach();
}

void releaseBallast() {
  digitalWrite(EN, HIGH);
}
