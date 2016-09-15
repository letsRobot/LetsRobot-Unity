#define SERVO_BUFF_LENGTH 20
#define MOVE_BUFF_LENGTH 20 

char rover_command_buf[20];
int16_t rover_distance_a_buf[20];
int16_t rover_distance_b_buf[20];
uint8_t rover_cmd_front = 0;
uint8_t rover_cmd_rear = 0;

char move_command_buf[MOVE_BUFF_LENGTH];
int16_t move_distance_a_buf[MOVE_BUFF_LENGTH];
int16_t move_distance_b_buf[MOVE_BUFF_LENGTH];
uint8_t move_cmd_front = 0;
uint8_t move_cmd_rear = 0;

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
     The human-readable pixel index is 1 based, so the valid range
     is [1, NUM_LEDS], not [0, NUM_LEDS-1].  After input validation,
     subtract one from index to use a stanard 0-based array.
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

void cmd_blink() {
  blink_open();   delay(350);
  blink_close(); delay(100);

  blink_open();   delay(350);
  blink_close(); delay(100);

  blink_open();
}


void cmd_setv(void) {
  int32_t value = nextarg_int(1, 20000, 0, -1);
  int32_t value2 = nextarg_int(1, 20000, 0, -1);
  Serial.println(format("Setting Motor Velocity %d", value));
  setpointA = value / cRate;
  setpointB = value2 / cRate;
  setpointAdefault = value / cRate;
  setpointBdefault = value2 / cRate;
}


void cmd_brightness(void)
{
  uint8_t value = nextarg_int(0, 255, 50, -1);
  eyes.setBrightness(value);
  eyes.show();
}

void cmd_poke_eyes(){
  do_command("led 255 " "20 0 0");
  do_command("led 5 " "20 20 20");
  do_command("led 14 " "20 20 20");
  do_command("led 1 " "20 20 20");
  do_command("led 2 " "20 20 20");
  do_command("led 3 " "20 20 20");
  do_command("led 10 " "20 20 20");
  do_command("led 11 " "20 20 20");
  do_command("led 12 " "20 20 20");
  
}

void cmd_square(){
  do_command("tics 400 0");
  do_command("tics 0 400");
  do_command("tics -400 0");
  do_command("tics 0 -400");
}

