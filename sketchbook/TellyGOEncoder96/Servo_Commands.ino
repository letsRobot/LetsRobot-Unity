//Creates a buffer and a front and back variable
char servo_command_buf[SERVO_BUFF_LENGTH];
uint8_t servo_distance_buf[SERVO_BUFF_LENGTH];
uint8_t servo_cmd_front = 0;
uint8_t servo_cmd_rear = 0;

//Puts a pan command at the rear of the queue
void cmd_pan_buf () {
  int16_t panval = nextarg_int(0, 180, 150, -1);
  if (panval != -1)
  {
   servo_command_buf[servo_cmd_rear] = 'p';
   servo_distance_buf[servo_cmd_rear] = panval;
   inc_servo_rear();
  }
}
//Excecutes a pan command
void cmd_pan(){
  pan_angle = servo_distance_buf[servo_cmd_front];
  //Serial.println(format("panangle %d", pan_angle));
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
  tilt_angle = servo_distance_buf[servo_cmd_front];
  //Serial.println(format("tiltangle %d", tilt_angle));
  tilt_go = true;
  tilt_desire = tilt_angle_prev;
}

void cmd_nod(){
  a_step = 3;
  do_command("tilt 160");
  do_command("tilt 120");
  do_command("tilt 160");
  do_command("tilt 120");
  do_command("tilt 160");
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

void servo_buf_clr(){
      servo_done = true;
      if (servo_cmd_rear == servo_cmd_front)
    {
      servo_cmd_rear = 0;
      servo_cmd_front = 0;
      a_step = 2;
      //Serial.println("buffclr");
    }
}

//Incrementally pans or tilts the head
void servo_loop(){
      prevMillisServo = currentMillis;
    
    if (pan_go == true)
    { 
      if (pan_angle >= pan_angle_prev)
      {
        pan_desire = pan_desire +  a_step;
        pan_servo.write(pan_desire);
        if (pan_desire >= pan_angle)
        {
          pan_go = false;
          pan_angle_prev = pan_angle;
          servo_buf_clr();
        }
      }
      else if (pan_angle <= pan_angle_prev)
      {
        pan_desire = pan_desire -  a_step;
        pan_servo.write(pan_desire); 
        if (pan_desire <= pan_angle)
        {
          pan_go = false;
          pan_angle_prev = pan_angle;
          servo_buf_clr();
        }
      }
    }

    if (tilt_go == true)
    {
      if (tilt_angle >= tilt_angle_prev)
      {
        tilt_desire = tilt_desire +  a_step;
        //Serial.println(tilt_desire);
        tilt_servo.write(tilt_desire);
        if (tilt_desire >= tilt_angle)
        {
          tilt_go = false;
           tilt_angle_prev = tilt_angle;
           servo_buf_clr();
        }
      }
      else if (tilt_angle <= tilt_angle_prev)
      {
        tilt_desire = tilt_desire -  a_step;
        tilt_servo.write(tilt_desire);
        //Serial.println(tilt_desire);
        if (tilt_desire <= tilt_angle)
        {
          tilt_go = false;
           tilt_angle_prev = tilt_angle;
           servo_buf_clr();
        }
      }
    }
}
void inc_servo_rear()
{
  if (servo_cmd_rear < SERVO_BUFF_LENGTH)
  servo_cmd_rear = servo_cmd_rear+1;
  else
  Serial.println("Servo_Overflow");
}

