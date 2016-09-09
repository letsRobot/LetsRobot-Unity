void heartbeat(void) {

  uint32_t currentMillis = millis();
  if (0 < stop_time && stop_time < currentMillis)
    stop();
  if ((motion_done == true) && (servo_cmd_rear > servo_cmd_front))
  {
    char mcmd = servo_command_buf[servo_cmd_front];
    switch (mcmd) {
      case 'p':
        cmd_pan();
        break;
      case 't':
        cmd_tilt();
        break;
    }
    motion_done = false;
    if (servo_cmd_front < SERVO_BUFF_LENGTH)
      servo_cmd_front = servo_cmd_front + 1;
  }


  if ((rover_cmd_done == true) && (rover_cmd_rear > rover_cmd_front))
  {
    char mcmd = rover_command_buf[rover_cmd_front];
    Serial.println("cmdread1");
    switch (mcmd) {
      case 'f':
        forwardPos();
        Serial.println("Fpos");
        break;
      case 'b':
        forwardPos();
        break;
      case 'l':
        turnPos();
        break;
      case 'r':
        turnPos();
        Serial.println("Rpos");
        break;
      case 'd':
        strafePos();
        break;
      case 'a':
        strafePos();
        break;
    }
    rover_cmd_done = false;
  }

  if (currentMillis >= rover_Millis)
  {
    if ((rover_cmd_done == false) && (rover_cmd_rear > rover_cmd_front))
    {
      char mcmd = rover_command_buf[rover_cmd_front];
       Serial.println("cmdread2");
      switch (mcmd) {
        case 'f':
          cmd_moveForward();
          break;
        case 'b':
          cmd_moveBack ();
          break;
        case 'l':
          cmd_turnLeft();
          break;
        case 'r':
          cmd_turnRight();
          break;
        case 'd':
          cmd_strafeRight ();
          break;
        case 'a':
          cmd_strafeLeft();
          break;
      }
      rover_Millis = 4294967295;
      
   
      
    }
  }

  if ((currentMillis - prevMillis) >= 10)
  {
    prevMillis = currentMillis;
    if (pan_go == true)
    {
      if (pan_angle >= pan_angle_prev)
      {
        pan_desire = pan_desire +  a_step;
        pwm.setPWM(pan, 0, pan_desire);
        if (pan_desire >= pan_angle)
        {
          pan_go = false;
          motion_done = true;
          Serial.println("Pandone");
          pan_angle_prev = pan_angle;
          Serial.println(pan_angle);
          servo_buf_clr();
        }
      }
      else if (pan_angle <= pan_angle_prev)
      {
        pan_desire = pan_desire -  a_step;
        pwm.setPWM(pan, 0, pan_desire);
        if (pan_desire <= pan_angle)
        {
          pan_go = false;
          motion_done = true;
          Serial.println("Pandone");
          pan_angle_prev = pan_angle;
          Serial.println(pan_angle);
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
        pwm.setPWM(tilt, 0, tilt_desire);
        if (tilt_desire >= tilt_angle)
        {
          tilt_go = false;
          motion_done = true;
          Serial.println("Tiltdone");
          tilt_angle_prev = tilt_angle;
          servo_buf_clr();
        }
      }
      else if (tilt_angle <= tilt_angle_prev)
      {
        tilt_desire = tilt_desire -  a_step;
        pwm.setPWM(tilt, 0, tilt_desire);
        //Serial.println(tilt_desire);
        if (tilt_desire <= tilt_angle)
        {
          tilt_go = false;
          motion_done = true;
          Serial.println("Tiltdone");
          tilt_angle_prev = tilt_angle;
          servo_buf_clr();
        }
      }
    }
  }
}
