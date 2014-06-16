/**
 * This is the main twitchBot program file.
 */

#include "simpletools.h"
#include "fdserial.h"
#include "abdrive.h"

fdserial *term; //enables full-duplex serilization of the terminal (In otherwise, 2 way signals between this computer and the robot)
int ticks = 12; //each tick makes the wheel move by 3.25mm, 64 ticks is a full wheel rotation (or 208mm)
int turnTick = 6; //Turning is at half the normal rate
int maxSpeed = 128; //the maximum amount of ticks the robot can travel with the "drive_goto" function
int minSpeed = 1; //the lowest amount of ticks the robot can travel with the "drive_goto" function

int main()
{
  //access the simpleIDE terminal
  simpleterm_close();
  //set full-duplex serialization for the terminal
  term = fdserial_open(31, 30, 0, 115200);

  //I'm not to sure what writeChar does, seems to get to text onto the console
  //dprint is the Parallax version of fprint used for their microcontrollers
  writeChar(term, CLS);
  dprint(term, "Click this terminal, \n");
  dprint(term, "and type on keyboard...\n\n");

  char c;

  while (1){
    c = fdserial_rxChar(term); //Get the character entered from the terminal
    if (c != -1)
    {
      //Use the below to see if any key input is being read by the terminal
      // dprint(term, "You typed: %c\n", c);
      //Link key presses to directional commands, and print the command to strings
      if (c == 'f') { //press "f" fof forward
        dprint(term, "Foward, \n",c);
        drive_goto(ticks, ticks);
      } else if (c == 'b') { //press "b" for backward
        dprint(term, "Backward, \n", c);
        drive_goto(-ticks, -ticks);
      } else if (c == 'r') { //press "r" for right turn
        dprint(term, "Right, \n", c);
        drive_goto(turnTick, -turnTick);
      } else if (c == 'l') { //press "l" for left turn
        dprint(term, "Left, \n", c);
        drive_goto(-turnTick, turnTick);

        //speeding up and slowing down
      } else if (c == 'u') {
        ticks = ticks +2;
        turnTick = turnTick +1;
        if (ticks > maxSpeed) {//clamp speed so it can't go over the maximum speed
           ticks = maxSpeed;
        }
        dprint(term, "Robot will now move %d ticks per second. \n", ticks);
      } else if (c == 'd') {
        ticks = ticks - 2;
        turnTick = turnTick -1;
        if (ticks < minSpeed) {//clamp speed so it can't go negative or 0
          ticks = minSpeed;
        }
        dprint(term, "Robot will now move %d ticks per second. \n", ticks);
      }
    }
  }
}

