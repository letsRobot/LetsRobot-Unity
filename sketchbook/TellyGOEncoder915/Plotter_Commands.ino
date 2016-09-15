#define Z_LOCATION_X 220
#define Z_LOCATION_Y 180

void cmd_test (void)
{
  int32_t value = nextarg_int(0, 500, 97, -1);
  Serial.println(value);
}

void cmd_type (void)
{
 int16_t x_pos = Z_LOCATION_X;
 int16_t y_pos = Z_LOCATION_Y;
 int32_t value = nextarg_int(97, 122, 97, -1);
 switch(value){
 case 113: //q
 x_pos = x_pos - 140;
 y_pos = y_pos + 160;
 break;
 case 122: //z
 break;
 
 }
  move_command_buf[move_cmd_rear] = 'o';
  move_distance_a_buf[move_cmd_rear] = x_pos;
  move_distance_b_buf[move_cmd_rear] = y_pos;
  inc_move_rear();
  move_command_buf[move_cmd_rear] = 'h';
  inc_move_rear();
}

void cmd_swipe_r_buf (void)
{
  move_command_buf[move_cmd_rear] = 'r';
  inc_move_rear();
  move_command_buf[move_cmd_rear] = 'o';
  move_distance_a_buf[move_cmd_rear] = EncA.read()*1000/A_limit;
  move_distance_b_buf[move_cmd_rear] = EncB.read()*1000/B_limit;
  inc_move_rear();
}

void cmd_pen_buf (void)
{
  int32_t value = nextarg_int(-1000, 1000, 0, -1);
  int32_t value2 = nextarg_int(-1000, 1000, 0, -1);
  move_command_buf[move_cmd_rear] = 'o';
  move_distance_a_buf[move_cmd_rear] = value;
  move_distance_b_buf[move_cmd_rear] = value2;
  inc_move_rear();
}

void cmd_swipe_l_buf (void)
{
  move_command_buf[move_cmd_rear] = 'l';
  move_cmd_rear = move_cmd_rear+1;
  move_command_buf[move_cmd_rear] = 'o';
  move_distance_a_buf[move_cmd_rear] = EncA.read()*1000/A_limit;
  move_distance_b_buf[move_cmd_rear] = EncB.read()*1000/B_limit;
  inc_move_rear();
}


void cmd_poke_buf (void)
{
  int32_t value = nextarg_int(0, 5000, 0, -1);
  move_distance_b_buf[move_cmd_rear] = value;
  move_command_buf[move_cmd_rear] = 'p';
  move_cmd_rear = move_cmd_rear+1;
  move_command_buf[move_cmd_rear] = 'o';
  int16_t ypos = EncB.read()*1000/B_limit;
  if (ypos < 0)
  ypos = 0; 
  move_distance_a_buf[move_cmd_rear] = EncA.read()*1000/A_limit;
  move_distance_b_buf[move_cmd_rear] = ypos;
  inc_move_rear();
}


void cmd_ticks_buf(void) {
  int32_t value = nextarg_int(-1500, 1500, 0, -1);
  int32_t value2 = nextarg_int(-1500, 1500, 0, -1);
  move_command_buf[move_cmd_rear] = 't';
  move_distance_a_buf[move_cmd_rear] = value;
  move_distance_b_buf[move_cmd_rear] = value2;
  inc_move_rear();
}


void cmd_spin (void)
{
 V_spin = true;
 setpointA = A_spinmax/2/cRate;
 setpointB = B_spinmax/2/cRate;
 Vel_DC_motor_loop ();
 velocity_time = millis()+100;
}

void cmd_swipe_right(void)
{
  scale_x = 2;
  setpointA = 3000 / cRate;;
  TickswantedA = A_limit /2 ;
  TickswantedB = 0;

  if (((A_limit < (TickswantedA + last_ticksA)) && TickswantedA > 0) && limitAfound == true)
  {
    TickswantedA = A_limit - last_ticksA;
    Serial.println("possata");
  }
  else if ((0 > TickswantedA + last_ticksA) && limitAfound == true)
  {
    TickswantedA = 50 - last_ticksA;
    Serial.println("negsata");
  }
  
  digitalWrite(solenoid, HIGH);
  pokeball = true;
  poketimer = millis() + 450;
  Pos_DC_motor_loop();
}

void cmd_swipe_left(void)
{
  scale_x = 2;
  setpointA = 3000 / cRate;;
  TickswantedA = -A_limit /2;
  TickswantedB = 0;

  if (((A_limit < (TickswantedA + last_ticksA)) && TickswantedA > 0) && limitAfound == true)
  {
    TickswantedA = A_limit - last_ticksA;
    Serial.println("possata");
  }
  else if ((0 > TickswantedA + last_ticksA) && limitAfound == true)
  {
    TickswantedA = 50 - last_ticksA;
    Serial.println("negsata");
  }

  digitalWrite(solenoid, HIGH);
  pokeball = true;
  poketimer = millis() + 450;
  Pos_DC_motor_loop();
}

void cmd_pokeball(void)
{
  int32_t value = move_distance_b_buf[move_cmd_front];
  setpointA = setpointAdefault;
  setpointB = value / cRate;
  TickswantedA = 0;
  TickswantedB = 600;
  
    if (((B_limit < (TickswantedB + last_ticksB)) && TickswantedB > 0) && limitBfound == true)
  {
    TickswantedB = B_limit - last_ticksB;
    Serial.println("possatb");
  }
  else if ((0 > TickswantedB + last_ticksB) && limitBfound == true)
  {
    TickswantedB = 50 - last_ticksB;
    Serial.println("negsatb");
  }
  
  digitalWrite(solenoid, HIGH);
  pokeball = true;
  poketimer = millis() + 300;
  scale_y = 2;
  Pos_DC_motor_loop();
}

void cmd_pokefun(void)
{
  int32_t value = nextarg_int(0, 1000, 0, -1);
  int32_t value2 = nextarg_int(0, 1000, 0, -1);
  setpointA = setpointAdefault;
  setpointB = 10000 / cRate;
  TickswantedA = 0;
  TickswantedB = value2-last_ticksB;
  digitalWrite(solenoid, HIGH);
  pokeball = true;
  poketimer = millis() + value;
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
  digitalWrite(solenoid, HIGH);
}

void cmd_tap (void)
{
  digitalWrite(solenoid, HIGH);
  pokeball = true;
  poketimer = millis() + 200;
}

void cmd_retract (void)
{
  digitalWrite(solenoid, LOW);
}

void cmd_ticks(void) {
  int32_t value1 = move_distance_a_buf[move_cmd_front];
  int32_t value2 = move_distance_b_buf[move_cmd_front];
  TickswantedA = value1 * A_limit/1000;
  TickswantedB = value2 * B_limit/1000;
  if (((A_limit < (TickswantedA + last_ticksA)) && TickswantedA > 0) && limitAfound == true)
  {
    TickswantedA = A_limit - last_ticksA;
    Serial.println("possata");
  }
  else if ((0 > TickswantedA + last_ticksA) && limitAfound == true)
  {
    TickswantedA = 50 - last_ticksA;
    Serial.println("negsata");
  }
  if (((B_limit < (TickswantedB + last_ticksB)) && TickswantedB > 0) && limitBfound == true)
  {
    TickswantedB = B_limit - last_ticksB;
    Serial.println("possatb");
  }
  else if ((0 > TickswantedB + last_ticksB) && limitBfound == true)
  {
    TickswantedB = 50 - last_ticksB;
    Serial.println("negsatb");
  }

  Serial.print("TicksA ");
  Serial.println(TickswantedA);
  Serial.print("TicksB ");
  Serial.println(TickswantedB);
  Pos_DC_motor_loop();
}

void cmd_atics(void) {
  int32_t value1 = move_distance_a_buf[move_cmd_front];
  int32_t value2 = move_distance_b_buf[move_cmd_front];
  if(value2<=100)
  value2 = B_limit_low;
  TickswantedA = value1 * A_limit/1000;
  TickswantedB = value2 * B_limit/1000;
  Serial.print("lastticks");
  Serial.println(last_ticksA);
    TickswantedA = TickswantedA - last_ticksA;
    TickswantedB = TickswantedB - last_ticksB;
    

  Serial.print("TicksA ");
  Serial.println(TickswantedA);
  Serial.print("TicksB ");
  Serial.println(TickswantedB);
  Pos_DC_motor_loop();
}


void cmd_pos(void)
{
  int32_t lastposA = EncA.read();
  int32_t lastposB = EncB.read();
  Serial.println("A Pos, B pos");
  Serial.println(lastposA*1000/A_limit);
  Serial.println(lastposB*1000/B_limit);
}

void cmd_zero(void)
{
  limitAfound = false;
  limitBfound = false;
  do_command("tics -1200 -1200");
}

void inc_move_rear()
{
  if (move_cmd_rear < MOVE_BUFF_LENGTH)
  move_cmd_rear = move_cmd_rear+1;
  else
  Serial.println("Move_Overflow");
}

