void cmd_pokeball(void)
{
  setpointA = setpointAdefault;
  setpointB = 8000/cRate;
  TickswantedA = 0;
  TickswantedB = 2000;
  digitalWrite(solenoid,HIGH);
  Serial.println("o");
  pokeball = true;
  poketimer = millis()+500;
  Pos_DC_motor_loop();
}

void cmd_lastpos (void)
{
  Serial.println("A,BN");
  Serial.println(last_ticksA);
  Serial.println(last_ticksB);
}
void cmd_push (void)
{
 digitalWrite(solenoid,HIGH);
}

void cmd_tap (void)
{
 digitalWrite(solenoid,HIGH);
  pokeball = true;
  poketimer = millis()+200;
}

void cmd_retract (void)
{
 digitalWrite(solenoid,LOW);
}

void cmd_rover (void)
{
 int32_t value = nextarg_int(-500, 500, 0, -1);
 int32_t value2 = nextarg_int(-500, 500, 0, -1);
 uint32_t time_stop = nextarg_int(0, 10000, 0, -1);
 schedule_stop(time_stop);
 DC_Rover(value,value2);
}

void cmd_forward (void)
{
 schedule_stop(default_stop_time);
 DC_Rover(rover_speed,-rover_speed);
}


void cmd_back    (void)
{
  schedule_stop(default_stop_time);
  DC_Rover(-rover_speed,rover_speed);
}


void cmd_left    (void)
{
  schedule_stop(default_stop_time/2);
  DC_Rover(-rover_speed,-rover_speed);
}


void cmd_right   (void)
{
  schedule_stop(default_stop_time/2);
  DC_Rover(rover_speed,rover_speed);
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
    
}

void cmd_close(void) 
{
    gripper_servo.writeMicroseconds(600);
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
  int32_t value1 = nextarg_int(-50000, 50000, 0, -1);
  int32_t value2 = nextarg_int(-50000, 50000, 0, -1);
  TickswantedA = value1;
  TickswantedB = value2;
  if (((A_limit<(TickswantedA+last_ticksA)) && TickswantedA > 0)&& limitAfound==true)
  {
    TickswantedA=A_limit-last_ticksA;
    Serial.println("possata");
  }
  else if ((0>TickswantedA+last_ticksA)&& limitAfound==true)
  {
    TickswantedA=50-last_ticksA;
    Serial.println("negsata");
  }
    if (((B_limit<(TickswantedB+last_ticksB)) && TickswantedB > 0)&& limitBfound==true)
  {
    TickswantedB=B_limit-last_ticksB;
    Serial.println("possatb");
  }
    else if ((0>TickswantedB+last_ticksB)&& limitBfound==true)
  {
    TickswantedB=50-last_ticksB;
    Serial.println("negsatb");
  }
  
  Serial.print("TicksA ");
  Serial.println(TickswantedA);
  Serial.print("TicksB ");
  Serial.println(TickswantedB);
   Pos_DC_motor_loop();
}

void cmd_atics(void) {
//  int32_t value1 = nextarg_int(-50000, 50000, 0, -1);
//  int32_t value2 = nextarg_int(-50000, 50000, 0, -1);
//  TickswantedA = value1-last_ticksA;
//  TickswantedB = value2-last_ticksB;
//  if (A_limit<TickswantedA+last_ticksA)
//  {
//    TickswantedA=A_limit-last_ticksA;
//  }
//    if (B_limit<TickswantedB+last_ticksB)
//  {
//    TickswantedB=B_limit-last_ticksB;
//  }
//  Serial.println(format("TicksA", TickswantedA));
//  Serial.println(format("TicksB", TickswantedB));
//   Pos_DC_motor_loop();
}


void cmd_setv(void) {
  int32_t value = nextarg_int(1, 20000, 0, -1);
  Serial.println(format("Setting Motor Velocity %d", value));
  setpointA = value / cRate;
  setpointB = value /2/ cRate;
  setpointAdefault = value / cRate;
  setpointBdefault = value /2/ cRate;

}

void cmd_rover_speed(void)
{
  int32_t value = nextarg_int(1, 160, 0, -1);
  Serial.println(format("Setting Rover Velocity %d", value));
  rover_speed = value;
}
void cmd_pan(void) {
  pan_angle = nextarg_int(0, 180, 0, -1);
  Serial.println(format("close %d", pan_angle));
  pan_go = true;
  pan_desire = pan_angle_prev;
}

void cmd_tilt(void)
{
  tilt_angle = nextarg_int(0,180, 0, -1);
  Serial.println(format("close %d", tilt_angle));
  tilt_go = true;
  tilt_desire = tilt_angle_prev;  
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



