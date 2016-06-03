// vim:set sw=4 ts=4 ai et:

/*
 * Arduino sketch for Telly
 * (c) Copyright 2016 Let's Robot.
 */

#include <Servo.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>

#include "library.h"
#include "config.h"

/***/

#ifdef TEENSY
    #define GRIPPER_PIN     2
    #define NEOPIXEL_PIN    3
    #define LEFT_PIN        8
    #define RIGHT_PIN       10
#else
    #define GRIPPER_PIN     2222
    #define NEOPIXEL_PIN    6
    #define LEFT_PIN        9
    #define RIGHT_PIN       10
#endif

/*
 * Continuous rotation servos for the two wheels.  Because of the way the
 * servos had to be mounted, they're facing different directions.  With this
 * library, 90 is the stop value, 0 is full-speed in one direction, and 180
 * is full-speed in the other direction.
 */
#define LEFT_STOP       95
#define LEFT_FORWARD    (LEFT_STOP  -10)
#define LEFT_BACKWARD   (LEFT_STOP  +10)
//
#define RIGHT_STOP      95
#define RIGHT_FORWARD   (RIGHT_STOP +10)
#define RIGHT_BACKWARD  (RIGHT_STOP -10)

Servo left_servo, right_servo;

/*
 * How long in milliseconds to drive the motors when moving forward and
 * backward, and left and right.
 */
#define MAX_DRIVE_TIME      3000
#define MAX_TURN_TIME       3000
#define DEFAULT_DRIVE_TIME  1000
#define DEFAULT_TURN_TIME    250

/*
 * I2C, for supporting the old protocol's LED interface
 */
#define I2C_ADDRESS      0x04
#define NUM_LEDS         (9*2)
#define I2C_TIMEOUT_TIME 1000

/*
 * The mapping of LEDs as they appear on the NeoPixel strip, vs the human
 * address we'd like to give them.
 */
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

void stop(void) {
    left_servo.write(LEFT_STOP);
    right_servo.write(RIGHT_STOP);

    /*
     * If the value was 1, this is a special case, and the STOP is being
     * executed immediately.  Return the normal OK string.  Otherwise, the
     * stop is the result of a previously scheduled asynchronous movement
     * command.  Return the asynchronous OK  string.
     */

    if (compat) {
        OK();
    }

    else {
        if (stop_time == 1)
            OK();
        else
            redraw_prompt = true;
            Serial.println("\rASYNC_OK");
    }

    stop_time = 0;
}

void move(int left, int right, int default_time) {
    int duration;

    duration = nextarg_int(10, MAX_DRIVE_TIME, default_time, -1);

    if (duration < 0) {
        Serial.println("ASYNC_ERR Invalid argument");
        return;
    }

    if (! compat)
        Serial.println("ASYNC_RUNNING");
    left_servo.write(left);
    right_servo.write(right);
    schedule_stop(duration);
}

void heartbeat(void) {
    if (0 < stop_time && stop_time < millis())
        stop();
}

/***/

void cmd_forward (void) { move(LEFT_FORWARD,  RIGHT_FORWARD,  DEFAULT_DRIVE_TIME); }
void cmd_back    (void) { move(LEFT_BACKWARD, RIGHT_BACKWARD, DEFAULT_DRIVE_TIME); }
void cmd_left    (void) { move(LEFT_BACKWARD, RIGHT_FORWARD,  DEFAULT_TURN_TIME);  }
void cmd_right   (void) { move(LEFT_FORWARD,  RIGHT_BACKWARD, DEFAULT_TURN_TIME);  }

void cmd_stop(void) {
    if (stop_time)
        schedule_stop(1);
    heartbeat();
    OK();
}

void cmd_led(void) {
    int pixel = nextarg_int(0, 0xff, -1, -1);
    int red   = nextarg_int(0, 0xff, -1, -1);
    int green = nextarg_int(0, 0xff, -1, -1);
    int blue  = nextarg_int(0, 0xff, -1, -1);

    int i;

    if (red < 0 || green < 0 || blue < 0 || pixel < 0) {
        ERR("Invalid argument");
        return;
    }

    if (pixel == 0xFF) {
        for (i = 0; i < NUM_LEDS; i++)
            eyes.setPixelColor(i, red, green, blue);
        servo_detach();
        eyes_show();
        servo_attach();
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
    servo_detach();
    eyes_show();
    servo_attach();
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
    left_servo.attach(LEFT_PIN);
    right_servo.attach(RIGHT_PIN);
}

void servo_detach() {
    left_servo.detach();
    right_servo.detach();
}

void setup() {
    add_command("forward",  cmd_forward);
    add_command("back",     cmd_back);
    add_command("left",     cmd_left);
    add_command("right",    cmd_right);
    add_command("stop",     cmd_stop);
    add_command("f",        cmd_forward);
    add_command("b",        cmd_back);
    add_command("l",        cmd_left);
    add_command("r",        cmd_right);
    add_command("s",        cmd_stop);
    add_command("led",      cmd_led);
    add_command("blink",    cmd_blink);

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

    eyes.begin();
    eyes_show();

    servo_attach();

    pinMode(LED_BUILTIN, OUTPUT);

    for (int i = 0; i < 8; i++) {
        digitalWrite(LED_BUILTIN, LOW);
        delay(33);

        digitalWrite(LED_BUILTIN, HIGH);
        delay(33);
    }

    cmd_blink();
}

void loop() {
    library_loop();
}
