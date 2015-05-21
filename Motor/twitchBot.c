/**
 * This is the main twitchBot program file.
 */
#include "simpletools.h"
#include "fdserial.h"
#include "abdrive.h"
#include "ping.h"
#include "servo.h"

void Drive(int leftTicks, int rightTicks, int speed);
void Grip(int gripTicks, int gripSpeed);

fdserial *term; //enables full-duplex serilization of the terminal (In otherwise, 2 way signals between this computer and the robot)
int ticks = 12; //each tick makes the wheel move by 3.25mm, 64 ticks is a full wheel rotation (or 208mm)
int turnTick = 6; //Turning is at half the normal rate
int maxSpeed = 128; //the maximum amount of ticks the robot can travel with the "drive_goto" function
int minSpeed = 2; //the lowest amount of ticks the robot can travel with the "drive_goto" function
int maxTurnSpeed = 64;
int minTurnSpeed = 2;
int gripDegree = 0; //Angle of the servo that controls the gripper
int gripState = -1;

int pingDistance;

int main()
{
  //access the simpleIDE terminal
  simpleterm_close();
  //set full-duplex serialization for the terminal
  term = fdserial_open(31, 30, 0, 9600);

  char c;

  //servo_angle(16, gripDegree); //Orient gripper to half open on start
  //pause(3000);

  while (1)
  {
    c = fdserial_rxChar(term); //Get the character entered from the terminal
    if (c != -1)
    {
      //Use the below to see if any key input is being read by the terminal
      // dprint(term, "You typed: %c\n", c);

      //Link key presses to directional commands, and print the command to strings
      if (c == 'f') //press "f" fof forward
      {           
        Drive(ticks, ticks, maxSpeed);
        dprint(term, "ok\n");
      } 
      else if (c == 'b') //press "b" for backward
      { 
        Drive(-ticks, -ticks, maxSpeed);
        dprint(term, "ok\n");
      } 
      else if (c == 'r') //press "r" for right turn
      { 
        Drive(turnTick, -turnTick, maxTurnSpeed);
        dprint(term, "ok\n");
      } 
      else if (c == 'l') //press "l" for left turn
      { 
        Drive(-turnTick, turnTick, maxTurnSpeed);
        dprint(term, "ok\n");
      } 

      //Increasing and Decreasing Drive Speed
      else if (c == 'u') 
      {
        ticks = ticks + 2;
        if (ticks > maxSpeed) //clamp speed so it can't go over the maximum speed
        {
           ticks = maxSpeed;
        }
        dprint(term, "move_speed %d\n", ticks);
      } 
      else if (c == 'd') 
      {
        ticks = ticks - 2;
        if (ticks < minSpeed) //clamp speed so it can't go negative or 0
        {
          ticks = minSpeed;
        }
        dprint(term, "move_speed %d\n", ticks);
      }

      //Increase Turn Speed;
      else if (c == 't') 
      {
        turnTick = turnTick + 1;
        if (turnTick > maxTurnSpeed)
        {
          turnTick = maxTurnSpeed;
        }
        dprint(term, "turn_speed %d\n", turnTick);
      }
      
      //decrease turn speed
      else if (c == 'n')
      {
        turnTick = turnTick - 1;
        if (turnTick < minTurnSpeed)
          {
          turnTick = minTurnSpeed;
          }
        dprint(term, "turn_speed %d\n", turnTick);
      }

      else if (c == 'p') //ping distance
      {
        pingDistance = ping(8);
        dprint(term, "echo %d\n", pingDistance);
        dprint(term, "ok\n");
      }

      else if (c == 'k') // poke
      {
      }

      else if (c == 'a')
      {
        Attack();
      }
      else if (c == 'o') //open gripper
      {
        gripState = 0;
        gripDegree = -1800;
      }
      else if (c == 'g') //close gripper
      {
        gripState = 1;
        gripDegree = 1800;
      }
    } // End of Read Character Function
  if (gripState != -1) 
  {
    Grip(gripDegree, 2000);
  }
  } // End of While Loop
} //End of Main Loop

void Grip(int gripTicks, int gripSpeed)
{
  servo_setramp(16, 32);
  servo_angle(16, gripTicks);
  //dprint(term, "Using Gripper");
  //pause(gripSpeed);
  dprint(term, "ok\n");
  gripState = -1;

}

void Attack() {
  servo_angle(16, 1800);
  pause(750);
  servo_angle(16, 0);
  pause(750);
  //servo_stop();
  servo_disable(16);
  dprint(term, "ok\n");
  //servo_start();
}

void Drive(int leftTicks, int rightTicks, int speed)
{
  // Tick count when we started
  int startLeftTicks;
  int startRightTicks;
  drive_getTicks(&startLeftTicks, &startRightTicks);

  // Target tick counts
  const int leftTargetTicks  = startLeftTicks  + leftTicks;
  const int rightTargetTicks = startRightTicks + rightTicks;

  // Acceleration
  const int speedStep = 4; // Acceleration per 1/20th of a second
  int currentSpeed = 0;

  int destinationReached = 0;

  int leftOldTicks;
  int rightOldTicks;
  drive_getTicks(&leftOldTicks, &rightOldTicks);

  int stuck = 0;
  const int maxTimesStuck = 10; // This is the number of times the robot has to think it's stuck before it stops

  do
  {
    // Accelerate
    currentSpeed += speedStep;
    if(currentSpeed > speed)
      currentSpeed = speed;

    const int leftCurrentSpeed  = leftTicks  >= 0 ? currentSpeed : -currentSpeed;
    const int rightCurrentSpeed = rightTicks >= 0 ? currentSpeed : -currentSpeed;
    drive_speed(leftCurrentSpeed, rightCurrentSpeed);

    pause(50);

    // Current tick count
    int leftCurrentTicks;
    int rightCurrentTicks;
    drive_getTicks(&leftCurrentTicks, &rightCurrentTicks);

    // Check if we're stuck
    if(leftOldTicks  == leftCurrentTicks ||
       rightOldTicks == rightCurrentTicks)
      stuck++;
    else
    {
//      dprint(term, "stuck: %d\n", stuck);/**/
      stuck = 0;
    }

    if(stuck > maxTimesStuck)
    {
//      dprint(term, "Oh God, I really am stuck!!!\n");/**/
      drive_speed(0, 0);
      return;
    }

    leftOldTicks = leftCurrentTicks;
    rightOldTicks = rightCurrentTicks;

    destinationReached = (leftTicks  >= 0 && leftCurrentTicks  >= leftTargetTicks)  ||
                         (leftTicks  <= 0 && leftCurrentTicks  <= leftTargetTicks)  ||
                         (rightTicks >= 0 && rightCurrentTicks >= rightTargetTicks) ||
                         (rightTicks <= 0 && rightCurrentTicks <= rightTargetTicks);

//dprint(term, "currentSpeed: %d leftCurrentTicks: %d rightCurrentTicks: %d\n", currentSpeed, leftCurrentTicks, rightCurrentTicks);/**/
  }
  while(!destinationReached);

  // Deaccelerate
  while(currentSpeed > 0)
  {
    currentSpeed -= speedStep;
    if(currentSpeed < 0)
      currentSpeed = 0;

    const int leftCurrentSpeed  = leftTicks  >= 0 ? currentSpeed : -currentSpeed;
    const int rightCurrentSpeed = rightTicks >= 0 ? currentSpeed : -currentSpeed;
    drive_speed(leftCurrentSpeed, rightCurrentSpeed);
  }
}