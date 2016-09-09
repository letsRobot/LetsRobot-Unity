#define MOVE_BUFF_LENGTH 20
char move_command_buf[MOVE_BUFF_LENGTH];
int16_t move_distance_buf[MOVE_BUFF_LENGTH];
uint8_t move_cmd_front = 0;
uint8_t move_cmd_rear = 0;

void cmd_toggle()
{
  analog_enable = !analog_enable;
  Serial.println(analog_enable);
}

void cmd_f()
{
  if (analog_enable == true)
    cmd_f_analog();
  else
    cmd_forward();
}

void cmd_r()
{
  if (analog_enable == true)
    cmd_r_analog();
  else
    cmd_right();
}

void cmd_l()
{
  if (analog_enable == true)
    cmd_l_analog();
  else
    cmd_left();
}

void cmd_b()
{
  if (analog_enable == true)
    cmd_b_analog();
  else
    cmd_back();
}

void cmd_grip_strength (void)
{
  grip_strength = nextarg_int(0, 255, 0, -1);
}

void cmd_xaxis(void)
{ if (analog_enable == true)
  {
    int16_t value = nextarg_int(-1200, 1200, 0, -1);
    if (value != 10)
    {
      X_val = value;
      analog_timer = millis() + 200;
      analog_good = true;
    }
  }
}

void cmd_yaxis(void)
{
  if (analog_enable == true)
  {
    int16_t value = nextarg_int(-1200, 1200, 0, -1);
    if (value != 10)
    {
      Y_val = value;
      analog_timer = millis() + 200;
      analog_good = true;
    }
  }
}

void cmd_r_analog(void)
{
  X_val = X_val;
  cmd_analog_control ();

}

void cmd_l_analog(void)
{
  X_val = -X_val;
  cmd_analog_control ();
}

void cmd_f_analog(void)
{
  Y_val = Y_val;
  cmd_analog_control ();
}

void cmd_b_analog(void)
{
  Y_val = -Y_val;
  cmd_analog_control ();
}

void cmd_analog_control (void)
{
  int16_t Left_V = 0;
  int16_t Right_V = 0;

  if (X_val >= 0)
  {
    if (Y_val >= 0)
    {
      if (Y_val >= X_val)
      {
        Left_V = Y_val;
        Right_V = Y_val - X_val;
      }
      else if (X_val > Y_val)
      {
        Left_V = X_val;
        Right_V = -X_val + Y_val;
      }
    }
    if (Y_val < 0)
    {
      if (-Y_val >= X_val)
      {
        Left_V = Y_val;
        Right_V = Y_val + X_val;
      }
      else if (X_val > -Y_val)
      {
        Left_V = X_val + Y_val;
        Right_V = -X_val;
      }
    }
  }
  else if (X_val < 0)
  {
    if (Y_val >= 0)
    {
      if (Y_val >= -X_val)
      {
        Left_V = Y_val + X_val;
        Right_V = Y_val;
      }
      else if (-X_val > Y_val)
      {
        Left_V = X_val + Y_val;
        Right_V = -X_val;
      }
    }
    if (Y_val < 0)
    {
      if (-Y_val >= -X_val)
      {
        Left_V = Y_val;
        Right_V = Y_val - X_val;
      }
      else if (-X_val > -Y_val)
      {
        Left_V = X_val;
        Right_V = -X_val + Y_val;
      }
    }

  }
  setpointA = Left_V * analog_speed / cRate;
  setpointB = Right_V * analog_speed / cRate;

  Vel_DC_motor_loop();

  //  setpointA = Left_V/cRate;
  //  setpointB = Right_V/cRate;
  //  Vel_DC_motor_loop();

}

void cmd_vel_control (void)
{
  int16_t Left_V = nextarg_int(-5000, 5000, 0, -1);
  setpointA = Left_V / cRate;
  int16_t Right_V = nextarg_int(-5000, 5000, 0, -1);
  setpointB = Right_V / cRate;
  Vel_DC_motor_loop();

}


void cmd_forward (void)
{
  move_command_buf[move_cmd_rear] = 'f';
  increment_move_rear();

}


void cmd_back    (void)
{
  move_command_buf[move_cmd_rear] = 'b';
  increment_move_rear();
}


void cmd_left    (void)
{
  move_command_buf[move_cmd_rear] = 'l';
  increment_move_rear();
}


void cmd_right   (void)
{
  move_command_buf[move_cmd_rear] = 'r';
  increment_move_rear();
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

void cmd_open(void) {
  digitalWrite(GRIP_INA, LOW);
  digitalWrite(GRIP_INB, HIGH);
  analogWrite(GRIP_PWM, 200);
  open_grip = true;
  OK();
}

void cmd_close(void) {
  digitalWrite(GRIP_INA, HIGH);
  digitalWrite(GRIP_INB, LOW);
  analogWrite(GRIP_PWM, 220);
  open_grip = false;
  schedule_stop(1500);
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

void cmd_ticks(void) {
  int32_t value = nextarg_int(-50000, 50000, 0, -1);
  move_command_buf[move_cmd_rear] = 't';
  move_distance_buf[move_cmd_rear] = value;
  move_cmd_rear = move_cmd_rear + 1;
}


void cmd_r_ticks(void) {
  int32_t value = nextarg_int(-360, 360, 0, -1);
  value = 1450 * value / 90;
  move_command_buf[move_cmd_rear] = 'a';
  move_distance_buf[move_cmd_rear] = value;
  move_cmd_rear = move_cmd_rear + 1;
}

void cmd_setv(void) {
  int8_t value = nextarg_int(1, 5, 3, -1);
  analog_speed = value;
  //  int32_t value = nextarg_int(1, 5000, 0, -1);
  //  Serial.println(format("Setting Motor Velocity %d", value));
  //  if (setpointA < 0)
  //  setpointA = -value / cRate;
  //  else
  //  setpointA = value / cRate;
  //  if (setpointB < 0)
  //  setpointB = -value / cRate;
  //  else
  //  setpointB = value/cRate;
  //
  //  setpointAdefault = value / cRate;
  //  setpointBdefault = value / cRate;

}

void cmd_pan(void) {
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
void cmd_pos(void)
{
  int32_t lastposA = EncA.read();
  int32_t lastposB = EncB.read();
  Serial.println("A Pos, B pos");
  Serial.println(lastposA);
  Serial.println(lastposB);
}


void increment_move_rear()
{
  if (move_cmd_rear < MOVE_BUFF_LENGTH)
    move_cmd_rear = move_cmd_rear + 1;
  else
  {
    Serial.println("Move_Overflow");
    move_cmd_rear = 0;
    move_cmd_front = 0;
    Serial.println("buffclr");
  }
}
