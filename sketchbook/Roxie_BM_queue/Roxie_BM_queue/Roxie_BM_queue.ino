// vim:set sw=4 ts=4 ai et:

/*
 * Arduino sketch for Telly
 * (c) Copyright 2016 Let's Robot.
 */

#include <Servo.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#include "library.h"
#include "config.h"

/***/
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
//robot wheels servo address
#define wrf 2 //Wheel Right Front
#define wrb 0 //Wheel Right Back
#define wlf 15 //Wheel Left Front
#define wlb 14 //Wheel Right Back

//Steering servos for the wheels
#define srf 4 //Steering Right Front
#define srb 3 //Steering Right Back
#define slf 12 //Steering Left Front
#define slb 13 //Steering Left Back
#define tilt 8
#define pan 9
#define NEOPIXEL_PIN    9

#define DEFAULT_PAN      280
#define DEFAULT_TILT     350
    
bool pan_go = false;
bool tilt_go = false;
uint16_t pan_angle = DEFAULT_PAN;
uint16_t pan_angle_prev = DEFAULT_PAN;
uint16_t tilt_angle = DEFAULT_TILT;
uint16_t tilt_angle_prev = DEFAULT_TILT;
uint16_t pan_desire = DEFAULT_PAN;
uint16_t tilt_desire = DEFAULT_TILT;
uint8_t a_step = 1;

#define ROVER_BUFF_LENGTH 20
char    rover_command_buf[ROVER_BUFF_LENGTH];
uint8_t rover_cmd_front = 0;
uint8_t rover_cmd_rear = 0;
/*
 * Continuous rotation servos for the two wheels.  Because of the way the
 * servos had to be mounted, they're facing different directions.  With this
 * library, 90 is the stop value, 0 is full-speed in one direction, and 180
 * is full-speed in the other direction.
 */
int driveTime = 1000; //How long the robot drives per command
int turnTime = 500; //How long to allow the wheels time to turn before moving
int turnStep = 250; //Amount of time the robot will turn right or left
int wheelPos = 0; // 0 = default, 1 = forward positon, 2 = turning position, 3 = strafe position.

int dspeed=10;
int tspeed=10;
int default_drive_time=1000;
int default_turn_time=250;
bool motion_done = true;
uint32_t prevMillis = 0;
uint32_t rover_Millis = 0;
bool rover_cmd_done = true;
/*
 * How long in milliseconds to drive the motors when moving forward and
 * backward, and left and right.
 */
#define MAX_DRIVE_TIME      3000
#define MAX_TURN_TIME       3000
//#define DEFAULT_DRIVE_TIME   750
//#define DEFAULT_TURN_TIME    500

/*
 * I2C, for supporting the old protocol's LED interface
 */
#define I2C_ADDRESS      0x08
#define NUM_LEDS         (9*2)
#define I2C_TIMEOUT_TIME 1000


/*
 * The mapping of LEDs as they appear on the NeoPixel strip, vs the human
 * address we'd like to give them.
 */
  // mirror (low towards center)
  int led_map[NUM_LEDS] = {
           0,  //  0
           3,  //  1
           6,  //  2
           1,  //  3
           4,  //  4
           7,  //  5
           2,  //  6
           5,  //  7
           8,  //  8
          15,  //  9
          16,  // 10
          17,  // 11
          12,  // 12
          13,  // 13
          14,  // 14
           9,  // 15
          10,  // 16
          11,  // 17
      };


Adafruit_NeoPixel eyes = Adafruit_NeoPixel(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
int eye_state;
bool eyes_no_show = false;


/***/

/*
 * Asyncrhonously scheduled stop.
 *
 * Values:
 *    0   No stop scheduled
 *    1   Stop immediately
 *   >1   Stop when millis() is greater than this number
 */
unsigned long stop_time;

void schedule_stop(unsigned long timer) {
    if (timer > 1)
        stop_time = millis() + timer;
    else
        stop_time = timer;
}

void stop(){

      //Serial.println("Time to Stop");
      pwm.setPWM(wrf, 0, 0);
      pwm.setPWM(wrb, 0, 0);
      pwm.setPWM(wlf, 0, 0);
      pwm.setPWM(wlb, 0, 0);
      rover_cmd_done = true;
      if (rover_cmd_front < ROVER_BUFF_LENGTH)
      rover_cmd_front = rover_cmd_front + 1;
      Serial.println(rover_cmd_front);
      rover_buf_clr();
      stop_time = 4294967295;
}


void cmd_stop(void) {
    if (stop_time)
        schedule_stop(1);
    heartbeat();
    //OK();
}

  void cmd_setdt(void) {
      int time = nextarg_int(1, 0x7fff, 3000, -1);
      //Serial.println(format("Drive time set: %d", spd));
      default_drive_time=time;
      OK();
  }

  void cmd_settt(void) {
      int time = nextarg_int(1, 0x7fff, 250, -1);
      //Serial.println(format("Turn time set: %d", spd));
      default_turn_time=time;
      OK();
  }

  void cmd_setds(void) {
      int spd = nextarg_int(1, 20, 10, -1);
      Serial.println(format("Drive speed set: %d", spd));
      dspeed=spd;
      OK();
  }

  void cmd_setts(void) {
      int spd = nextarg_int(1, 20, 10, -1);
      Serial.println(format("Turn speed set: %d", spd));
      tspeed=spd;
      OK();
  }

void cmd_led(void) {
    int pixel = nextarg_int(0, 0xff, -1, -1);
    int red   = nextarg_int(0, 0xff, -1, -1);
    int green = nextarg_int(0, 0xff, -1, -1);
    int blue  = nextarg_int(0, 0xff, -1, -1);

    int i;

    if (red < 0 || green < 0 || blue < 0 || pixel < 0) {
        ERR("Invalid led argument");
        return;
    }

    if (pixel == 0xFF) {
        for (i = 0; i < NUM_LEDS; i++)
            eyes.setPixelColor(i, red, green, blue);
        //servo_detach();
        eyes_show();
        //servo_attach();
        OK();
        return;
    }

    /*
     * The human-readable pixel index is 1 based, so the valid range
     * is [1, NUM_LEDS], not [0, NUM_LEDS-1].  After input validation,
     * subtract one from index to use a stanard 0-based array.
     */
    if (! (1 <= pixel && pixel <= NUM_LEDS))
        return ERR("LED index out of range");

    pixel -= 1;

    eyes.setPixelColor(led_map[pixel], red, green, blue);
    //servo_detach();
    eyes_show();
    //servo_attach();
    OK();
}

/*
 * I2C callbacks, for the old protocol
 */

void sendData() {
    //verbose("I2C write eye_state %d", eye_state);
    Wire.write(eye_state);
}

void receiveData(int num_bytes) {
    static int R, G, B;
    static int pixel;
    static unsigned long last_read;

    //verbose("I2C receiveDat num_bytes %d", num_bytes);

    /*
     * A timeout event, to help avoid out-of-sync errors with the Pi.  If we
     * haven't heard anything from the Pi in over half a second, go back to
     * state 0, assuming the next byte we're gonig to read is the start of a
     * new LED command.
     */
    if (millis() - last_read > I2C_TIMEOUT_TIME)
        eye_state = 0;

    last_read = millis();

    while (Wire.available()) {
        uint8_t val = Wire.read();
        //verbose("I2C read %d", val);

        switch (eye_state) {
            case 0:
                pixel = val;
                eye_state++;
                break;

            case 1:
                R = val;
                eye_state++;
                break;

            case 2:
                G = val;
                eye_state++;
                break;

            case 3:
                B = val;
                eye_state = 0;
                do_command(format("led %d %d %d %d", pixel, R, G, B));
                break;
        }
    }
}

/***/

#define EYE_BLINK_COLOR "0 25 25"

void blink_open() {
    eyes_no_show = true;
    do_command("led 255 " EYE_BLINK_COLOR);
    do_command("led   5 " "0 0 0");
    do_command("led  14 " "0 0 0");
    eyes_no_show = false;
    eyes_show();
}

void blink_close () {
    eyes_no_show = true;
    do_command("led 255 0 0 0");

    do_command("led   4 " EYE_BLINK_COLOR);
    do_command("led   5 " EYE_BLINK_COLOR);
    do_command("led   6 " EYE_BLINK_COLOR);

    do_command("led  13 " EYE_BLINK_COLOR);
    do_command("led  14 " EYE_BLINK_COLOR);
    do_command("led  15 " EYE_BLINK_COLOR);
    eyes_no_show = false;
    eyes_show();
}

void cmd_blink() {
    blink_open();   delay(350);
    blink_close(); delay(100);

    blink_open();   delay(350);
    blink_close(); delay(100);

    blink_open();
}

/***/

void eyes_show() {
    if (eyes_no_show)
        return;
    servo_detach();
    eyes.show();
    servo_attach();
}

void servo_attach() {
    //pan_servo.attach(12);
    // was 6, now 12
    //tilt_servo.attach(11);
}

void servo_detach() {
    //pan_servo.detach();
    //tilt_servo.detach();
}

void setup() {
  // Movement Commands
    add_command("forward",  cmd_moveForward);
    add_command("back",     cmd_moveBack);
    add_command("backward", cmd_moveBack);
    add_command("left",     cmd_turnLeft);
    add_command("right",    cmd_turnRight);
    add_command("stop",     cmd_stop);
    add_command("f",        cmd_f);
    add_command("b",        cmd_b);
    add_command("l",        cmd_l);
    add_command("r",        cmd_r);
    add_command("a",        cmd_a);
    add_command("d",        cmd_d);
    add_command("s",        cmd_stop);
    add_command("led",      cmd_led);
    add_command("blink",    cmd_blink);
    add_command("nod",      cmd_nod);
    add_command("no",       cmd_no);
   
    //Pan/Tilt Commands
    add_command("pan",     cmd_pan_buf);
    add_command("tilt",    cmd_tilt_buf);

    Serial.begin(19200);

    /*
     * For backwards compatability with the old protocol, only, which used I2C
     * to control the LEDs.
     *
     * Become an I2C slave.  First change the bitrate to 400khz, which must be
     * set before calling Wire.begin().  Then set the I2C callback functions.
     */
    TWBR = 12;
    Wire.begin(I2C_ADDRESS);
    Wire.onReceive(receiveData);
    Wire.onRequest(sendData);
    pwm.begin();
    pwm.setPWMFreq(50);  // This is the maximum PWM frequency
    eyes.begin();
    eyes_show();
    

    servo_attach();
    pwm.setPWM(pan, 0, 280);
    pwm.setPWM(tilt, 0, 350);
    pinMode(LED_BUILTIN, OUTPUT);

    for (int i = 0; i < 8; i++) {
        digitalWrite(LED_BUILTIN, LOW);
        delay(33);

        digitalWrite(LED_BUILTIN, HIGH);
        delay(33);
    }

    delay(1000);

    cmd_blink();
    Serial.println("loop");
}

void loop() {
    
    library_loop();
}
