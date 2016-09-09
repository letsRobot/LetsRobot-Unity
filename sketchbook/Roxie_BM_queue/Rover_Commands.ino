#define SERVO_BUFF_LENGTH 20
char   servo_command_buf[SERVO_BUFF_LENGTH];
uint8_t servo_distance_buf[SERVO_BUFF_LENGTH];
uint8_t servo_cmd_front = 0;
uint8_t servo_cmd_rear = 0;



void cmd_f(void)
{
  rover_command_buf[rover_cmd_rear] = 'f';
  increment_rover_rear();
}

void cmd_b(void)
{
  rover_command_buf[rover_cmd_rear] = 'b';
  increment_rover_rear();
}

void cmd_l(void)
{
  rover_command_buf[rover_cmd_rear] = 'l';
  increment_rover_rear();
}

void cmd_r(void)
{
  rover_command_buf[rover_cmd_rear] = 'r';
  increment_rover_rear();
}

void cmd_a(void)
{
  rover_command_buf[rover_cmd_rear] = 'a';
  increment_rover_rear();
}

void cmd_d(void)
{
  rover_command_buf[rover_cmd_rear] = 'd';
  increment_rover_rear();
}
void diagonalrightPos(){
   if (wheelPos != 4) { //Check current Wheel Position
       pwm.setPWM(srf, 0, 500);
       pwm.setPWM(srb, 0, 100);
       pwm.setPWM(slf, 0, 500);
       pwm.setPWM(slb, 0, 100);
       wheelPos = 4;
       delay(turnTime);
   }
}

void diagonalleftPos(){
   if (wheelPos != 5) { //Check current Wheel Position
       pwm.setPWM(srf, 0, 100);
       pwm.setPWM(srb, 0, 500);
       pwm.setPWM(slf, 0, 100);
       pwm.setPWM(slb, 0, 500);
       wheelPos = 5;
       rover_Millis = millis()+(turnTime);
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
        rover_Millis = millis()+(turnTime);
      }
      else
        rover_Millis = 1;
}

void turnPos() {
      if (wheelPos !=2) {
        pwm.setPWM(srf, 0, 400);
        pwm.setPWM(srb, 0, 170);
        pwm.setPWM(slf, 0, 200);
        pwm.setPWM(slb, 0, 400);
        wheelPos = 2;
        rover_Millis = millis()+(turnTime);
      }
      else
        rover_Millis = 1;
}

void strafePos() {
      if (wheelPos !=3) {  
        pwm.setPWM(srf, 0, 505);
        pwm.setPWM(srb, 0, 80);
        pwm.setPWM(slf, 0, 120);
        pwm.setPWM(slb, 0, 488);
        wheelPos = 3;
        rover_Millis = millis()+(turnTime);
      }
      else
        rover_Millis = 1;
}

void cmd_moveForward() {
      //Serial.println("Moving Forward");
      forwardPos();
      pwm.setPWM(wrf, 0, 400);
      pwm.setPWM(wrb, 0, 400);
      pwm.setPWM(wlf, 0, 200);
      pwm.setPWM(wlb, 0, 200);
      schedule_stop(driveTime);

}

void cmd_moveForwardDiagonally() {
      //Serial.println("Moving Forward");
      diagonalrightPos();
      pwm.setPWM(wrf, 0, 400);
      pwm.setPWM(wrb, 0, 400);
      pwm.setPWM(wlf, 0, 200);
      pwm.setPWM(wlb, 0, 200);
      schedule_stop(driveTime);

}

void cmd_moveBack () {
      pwm.setPWM(wrf, 0, 200);
      pwm.setPWM(wrb, 0, 200);
      pwm.setPWM(wlf, 0, 400);
      pwm.setPWM(wlb, 0, 400);
      schedule_stop(driveTime);
}

void cmd_turnRight () {
      //then drive
      pwm.setPWM(wrf, 0, 200);
      pwm.setPWM(wrb, 0, 200);
      pwm.setPWM(wlf, 0, 200);
      pwm.setPWM(wlb, 0, 200);
      schedule_stop(turnStep);
}

void cmd_turnLeft() {
      //change wheel position first
      pwm.setPWM(wrf, 0, 400);
      pwm.setPWM(wrb, 0, 400);
      pwm.setPWM(wlf, 0, 400);
      pwm.setPWM(wlb, 0, 400);
      schedule_stop(turnStep);
}

void cmd_strafeRight () {
      pwm.setPWM(wrf, 0, 200);
      pwm.setPWM(wrb, 0, 400);
      pwm.setPWM(wlf, 0, 200);
      pwm.setPWM(wlb, 0, 400);
      schedule_stop(driveTime);
}

void cmd_strafeLeft () {

      pwm.setPWM(wrf, 0, 400);
      pwm.setPWM(wrb, 0, 200);
      pwm.setPWM(wlf, 0, 400);
      pwm.setPWM(wlb, 0, 200);
      schedule_stop(driveTime);
}

void cmd_pan_buf () {
  int16_t panval = nextarg_int(0, 180, 150, -1);
  if (panval != -1)
  {
   servo_command_buf[servo_cmd_rear] = 'p';
   servo_distance_buf[servo_cmd_rear] = panval;
   inc_servo_rear();
  }
}
void cmd_pan(){
  pan_angle = 100+servo_distance_buf[servo_cmd_front]*38/18;
  Serial.println(format("panangle %d", pan_angle));
  pan_go = true;
  pan_desire = pan_angle_prev;
}

void cmd_tilt_buf () {
  int16_t tiltval = nextarg_int(0, 180, 150, -1);
  if (tiltval != -1)
  {
    servo_command_buf[servo_cmd_rear] = 't';
    servo_distance_buf[servo_cmd_rear] = tiltval;
    inc_servo_rear();
  }
}
void cmd_tilt(){
  tilt_angle = 100+servo_distance_buf[servo_cmd_front]*38/18;
  Serial.println(format("tiltangle %d", tilt_angle));
  tilt_go = true;
  tilt_desire = tilt_angle_prev;
}

void servo_buf_clr(){
      if (servo_cmd_rear == servo_cmd_front)
    {
      servo_cmd_rear = 0;
      servo_cmd_front = 0;
       a_step = 1;
      //Serial.println("buffclr");
    }
}
void cmd_nod(){
  a_step = 3;
  do_command("tilt 90");
  do_command("tilt 120");
  do_command("tilt 90");
  do_command("tilt 120");
  do_command("tilt 90");
  do_command("tilt 120");
}
void cmd_no(){
    a_step = 3;
  do_command("pan 110");
  do_command("pan 70");
  do_command("pan 110");
  do_command("pan 70");
  do_command("pan 110");
  do_command("pan 90");
}

void inc_servo_rear()
{
  if (servo_cmd_rear < SERVO_BUFF_LENGTH)
  servo_cmd_rear = servo_cmd_rear+1;
  else
  Serial.println("Servo_Overflow");
}
void increment_rover_rear()
{
  if (rover_cmd_rear < ROVER_BUFF_LENGTH)
  rover_cmd_rear = rover_cmd_rear+1;
  else
  Serial.println("Rover_Overflow");
}


void rover_buf_clr(){
      if (rover_cmd_rear <= rover_cmd_front)
    {
      rover_cmd_rear = 0;
      rover_cmd_front = 0;
      Serial.println("rbuffclr");
    }
}
