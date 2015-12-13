#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

/*
 Motor Controller originally written for Roxi the Robot (c) Let's Robot 2015
 Two very important things:
 1. Each command needs to return "ok" when finished executing
 2. No other variables or values can be printed to serial for the robot to work with Pi
 */

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

//String readString; //raw output from the serial connection
char command; //command to be executed
bool execute = 0; //triggers command

//robot wheels servo address
int wrf = 0; //Wheel Right Front
int wrb = 8; //Wheel Right Back
int wlf = 2; //Wheel Left Front
int wlb = 3; //Wheel Right Back

//Steering servos for the wheels
int srf = 4; //Steering Right Front
int srb = 5; //Steering Right Back
int slf = 6; //Steering Left Front
int slb = 7; //Steering Left Back

String ok = "ok\n"; //Let's the Pi know i am done doing a thing.
int driveTime = 1000; //How long the robot drives per command
int turnTime = 500; //How long to allow the wheels time to turn before moving
int turnStep = 250; //Amount of time the robot will turn right or left
int wheelPos = 0; // 0 = default, 1 = forward positon, 2 = turning position, 3 = strafe position.

void setup() {
  Serial.begin(9600);

  //enable PWM
  pwm.begin();
  pwm.setPWMFreq(50);  // This is the maximum PWM frequency

  // save I2C bitrate
  uint8_t twbrbackup = TWBR;
  // must be changed after calling Wire.begin() (inside pwm.begin())
  TWBR = 12; // upgrade to 400KHz!
}

void doneOk () {
Serial.print(ok);
}

void loop() {

       while (Serial.available()) {
       delay(10);  
         if (Serial.available() >0) {
       char c = Serial.read(); //read each character
       //Serial.println(c);
       //readString.concat(c);
       //readString += c; //add each character to the string
       //Serial.println("Read String: " + readString);
       //if (c == '\r') {
        //set command to read string
        command = c;
        //Serial.println("Command Is: " + command);
        execute = 1; //enables the execution of a command
        //readString = ""; //reset readString to blank
       }
    //execute the command
    if (execute == 1){
      //Serial.print(command + "\n");
     if (command == 'f') {
      moveForward();
      doneOk();
      
     } else if (command == 'b') {
      //Serial.println("Moving Back");
      moveBack();
      doneOk();

     } else if (command == 'r') {
      turnRight();
      doneOk();
     } else if (command == 'l') {
      turnLeft();
      doneOk();
     } else if (command == 'a') {
      strafeLeft();
      doneOk();
     } else if (command == 'd') {
      strafeRight();
      doneOk();
     }
     // end of command loop
     else {
      //Serial.println("null command " + command);
      
      //Serial.print("Bad Command: " + command + " ");
      //delay(1000);
     // doneOk();
     }
   } //end of execution loop
    
     //reset command variables
     execute = 0; 
//     command = '';
  }
 }


void forwardPos() {

      if (wheelPos != 1) { //Check current Wheel Position
        //turn the wheels if they aren't in the proper config
        pwm.setPWM(srf, 0, 305);
        pwm.setPWM(srb, 0, 260);
        pwm.setPWM(slf, 0, 288);
        pwm.setPWM(slb, 0, 290);
        wheelPos = 1;
        delay(turnTime);
      }
}

void turnPos() {
      if (wheelPos !=2) {
        pwm.setPWM(srf, 0, 400);
        pwm.setPWM(srb, 0, 170);
        pwm.setPWM(slf, 0, 200);
        pwm.setPWM(slb, 0, 400);
        wheelPos = 2;
        delay(turnTime);
      }
}

void strafePos() {
      if (wheelPos !=3) {  
        pwm.setPWM(srf, 0, 505);
        pwm.setPWM(srb, 0, 80);
        pwm.setPWM(slf, 0, 120);
        pwm.setPWM(slb, 0, 488);
        wheelPos = 3;
        delay(turnTime);
      }
}

void moveForward() {

      //Serial.println("Moving Forward");
      forwardPos();
      pwm.setPWM(wrf, 0, 400);
      pwm.setPWM(wrb, 0, 400);
      pwm.setPWM(wlf, 0, 200);
      pwm.setPWM(wlb, 0, 200);

      delay(driveTime);
      stop();
}

void moveBack () {

      forwardPos();
      pwm.setPWM(wrf, 0, 200);
      pwm.setPWM(wrb, 0, 200);
      pwm.setPWM(wlf, 0, 400);
      pwm.setPWM(wlb, 0, 400);

      delay(driveTime);
      stop();
}

void turnRight () {

      turnPos();
      //then drive
      pwm.setPWM(wrf, 0, 200);
      pwm.setPWM(wrb, 0, 200);
      pwm.setPWM(wlf, 0, 200);
      pwm.setPWM(wlb, 0, 200);
      delay(turnStep);
      stop();
}

void turnLeft() {
      //change wheel position first
      turnPos();

      //then drive
      pwm.setPWM(wrf, 0, 400);
      pwm.setPWM(wrb, 0, 400);
      pwm.setPWM(wlf, 0, 400);
      pwm.setPWM(wlb, 0, 400);
      delay(turnStep);
      stop();
}

void strafeRight () {

      strafePos();
      pwm.setPWM(wrf, 0, 200);
      pwm.setPWM(wrb, 0, 400);
      pwm.setPWM(wlf, 0, 200);
      pwm.setPWM(wlb, 0, 400);
      delay(driveTime);
      stop();
}

void strafeLeft () {

      strafePos();

      pwm.setPWM(wrf, 0, 400);
      pwm.setPWM(wrb, 0, 200);
      pwm.setPWM(wlf, 0, 400);
      pwm.setPWM(wlb, 0, 200);
      delay(driveTime);
      stop();
}
void stop(){

      //Serial.println("Time to Stop");
      pwm.setPWM(wrf, 0, 0);
      pwm.setPWM(wrb, 0, 0);
      pwm.setPWM(wlf, 0, 0);
      pwm.setPWM(wlb, 0, 0);
}
