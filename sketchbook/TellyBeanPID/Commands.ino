

void cmd_forward (void)
{

  int32_t TickswantedA = -3576 * 2 ; // Desired number of ticks for the motor to travel (1 rev = 3576)
  int32_t TickswantedB = -3576 * 2 ; // Desired number of ticks for the motor to travel (1 rev = 3576)
  DC_motor_loop(setpointA, TickswantedA, setpointB, TickswantedB);
}


void cmd_back    (void)
{
  int32_t TickswantedA = 3576 * 2; // Desired number of ticks for the motor to travel (1 rev = 3576)
  int32_t TickswantedB = 3576 * 2; // Desired number of ticks for the motor to travel (1 rev = 3576)
  DC_motor_loop(setpointA, TickswantedA, setpointB, TickswantedB);
}


void cmd_left    (void)
{
  int32_t TickswantedA = -1450; // Desired number of ticks for the motor to travel (1 rev = 3576)
  int32_t TickswantedB = 1450; // Desired number of ticks for the motor to travel (1 rev = 3576)
  DC_motor_loop(setpointA, TickswantedA, setpointB, TickswantedB);
}


void cmd_right   (void)
{
  int32_t TickswantedA = 1450; // Desired number of ticks for the motor to travel (1 rev = 3576)
  int32_t TickswantedB = -1450; // Desired number of ticks for the motor to travel (1 rev = 3576)
  DC_motor_loop(setpointA, TickswantedA, setpointB, TickswantedB);
}


void cmd_stop(void) {
  if (stop_time)
    schedule_stop(1);
  heartbeat();
  OK();
}

void cmd_setdt(void) {
  int time = nextarg_int(1, 0x7fff, 3000, -1);
  //Serial.println(format("Drive time set: %d", spd));
  default_drive_time = time;
  OK();
}

void cmd_settt(void) {
  int time = nextarg_int(1, 0x7fff, 250, -1);
  //Serial.println(format("Turn time set: %d", spd));
  default_turn_time = time;
  OK();
}

void cmd_setds(void) {
  int spd = nextarg_int(1, 20, 10, -1);
  Serial.println(format("Drive speed set: %d", spd));
  dspeed = spd;
  OK();
}

void cmd_setts(void) {
  int spd = nextarg_int(1, 20, 10, -1);
  Serial.println(format("Turn speed set: %d", spd));
  tspeed = spd;
  OK();
}

void cmd_setgo(void) {
  int pos = nextarg_int(-360, 360, 180, -1);
  //Serial.println(format("Gripper open position set: %d", pos));
  gripper_open = pos;
  OK();
}

void cmd_setgc(void) {
  int pos = nextarg_int(-360, 360, 0, -1);
  //Serial.println(format("Gripper close position set: %d", pos));
  gripper_close = pos;
  OK();
}
void cmd_grip_attach(void)
{
    gripper_servo.attach(GRIPPER_PIN);
}

void cmd_grip_detach(void)
{
  gripper_servo.detach();
}

void cmd_open(void) 
{
  uint16_t wantpos = 600;
  uint16_t pos = 2200;
  gripper_servo.attach(GRIPPER_PIN);
  if (grip_open == true)
  {
    gripper_servo.writeMicroseconds(wantpos);
  }
  else
  {
    while (pos >= wantpos)
    {
      gripper_servo.writeMicroseconds(pos);
      delay(12);
      pos = pos - 25;
    
  }
gripper_servo.detach();
grip_open = true;
}
}

void cmd_close(void) 
{
  uint16_t wantpos = 2200;
  uint16_t pos = 600;
 gripper_servo.attach(GRIPPER_PIN);
  if (grip_open == false)
  {
    gripper_servo.writeMicroseconds(wantpos);
  }
  else
  {
    while (pos <= wantpos)
    {
      gripper_servo.writeMicroseconds(pos);
      delay(12);
      pos = pos + 25;
    }
  }
  gripper_servo.detach();
  grip_open = false;
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

void cmd_fticks(void) {
  int32_t value = nextarg_int(1, 10000, 0, -1);
  TickswantedA = -value;
  TickswantedB = -value;
  Serial.println(format("Moving Forward %d", value));
  DC_motor_loop(setpointA, TickswantedA, setpointB, TickswantedB);
}
void cmd_bticks(void) {
  int32_t value = nextarg_int(1, 10000, 0, -1);
  TickswantedA = value;
  TickswantedB = value;
  Serial.println(format("Moving Backward %d", TickswantedA));
  DC_motor_loop(setpointA, TickswantedA, setpointB, TickswantedB);
}

void cmd_ticks(void) {
  int32_t value1 = nextarg_int(-10000, 10000, 0, -1);
  int32_t value2 = nextarg_int(-10000, 10000, 0, -1);
  TickswantedA = -value1;
  TickswantedB = -value2;
  Serial.println(format("Moving Backward %d", TickswantedA));
  DC_motor_loop(setpointA, TickswantedA, setpointB, TickswantedB);
}


void cmd_l_ticks(void) {
  int32_t value = nextarg_int(-10000, 10000, 0, -1);
  TickswantedA = value;
  TickswantedB = -value;
  Serial.println(format("Turning Left %d", TickswantedA));
  DC_motor_loop(setpointA, TickswantedA, setpointB, TickswantedB);
}

void cmd_r_ticks(void) {
  int32_t value = nextarg_int(-10000, 10000, 0, -1);
  TickswantedA = -value * 290/18 ;
  TickswantedB = value * 290/18 ;
  Serial.println(format("Turning Right %d", TickswantedA));
  DC_motor_loop(setpointA, TickswantedA, setpointB, TickswantedB);
}

void cmd_setv(void) {
  int32_t value = nextarg_int(1, 10000, 0, -1);
  Serial.println(format("Setting Motor Velocity %d", value));
  setpointA = value / cRate;
  setpointB = value / cRate;

}

void cmd_pan(void) {
  gripper_servo.attach(GRIPPER_PIN);
  int32_t value = nextarg_int(0, 2400, 0, -1);
  Serial.println(format("close %d", value));
  gripper_servo.writeMicroseconds(value);

}
void cmd_grab(void)
{
  cmd_open();
  delay(200);
  do_command("pan 150");
  delay(1000);
  cmd_close();
  delay(400);
  do_command("pan 30");
  delay(200);
  cmd_open();


}


