void heartbeat(void)
{
  currentMillis = millis();
  if ((currentMillis >= analog_timer) && analog_good == true)
  {
    X_val = 0;
    Y_val = 0;
    V_control_A = false;
    V_control_B = false;
    DC_vel_clean();
    analog_good = false;
  }

  if ((motion_done == true) && (move_cmd_rear > move_cmd_front))
  {
    Serial.println("buf_int");
    char mcmd = move_command_buf[move_cmd_front];
    if (mcmd == 'f')
    {
      TickswantedA = 3576 ; // Desired number of ticks for the motor to travel (1 rev = 3576)
      TickswantedB = 3576 ; // Desired number of ticks for the motor to travel (1 rev = 3576)
      Serial.println("movef");
      Pos_DC_motor_loop();
    }
    else if (mcmd == 'b')
    {
      TickswantedA = -3576 ; // Desired number of ticks for the motor to travel (1 rev = 3576)
      TickswantedB = -3576 ; // Desired number of ticks for the motor to travel (1 rev = 3576)
      Serial.println("moveb");
      Pos_DC_motor_loop();
    }
    else if (mcmd == 'l')
    {
      TickswantedA = -deg_360 / 8; // Desired number of ticks for the motor to travel (1 rev = 3576)
      TickswantedB = deg_360 / 8; // Desired number of ticks for the motor to travel (1 rev = 3576)
      Serial.println("movel");
      Pos_DC_motor_loop();
    }
    else if (mcmd == 'r')
    {
      TickswantedA = deg_360 / 8; // Desired number of ticks for the motor to travel (1 rev = 3576)
      TickswantedB = -deg_360 / 8; // Desired number of ticks for the motor to travel (1 rev = 3576)
      Serial.println("mover");
      Pos_DC_motor_loop();
    }
    else if (mcmd == 't')
    {
      TickswantedA = -move_distance_buf[move_cmd_front]; // Desired number of ticks for the motor to travel (1 rev = 3576)
      TickswantedB = -move_distance_buf[move_cmd_front]; // Desired number of ticks for the motor to travel (1 rev = 3576)
      Serial.println(TickswantedA);
      Pos_DC_motor_loop();
    }
    else if (mcmd == 'a')
    {
      TickswantedA = -move_distance_buf[move_cmd_front]; // Desired number of ticks for the motor to travel (1 rev = 3576)
      TickswantedB = move_distance_buf[move_cmd_front]; // Desired number of ticks for the motor to travel (1 rev = 3576)
      Serial.println(TickswantedA);
      Pos_DC_motor_loop();
    }
    motion_done = false;
    move_cmd_front = move_cmd_front + 1;

  }



  if (0 < stop_time && stop_time < millis())
  {
    stop();
  }
  if (open_grip == true)
  {
    int limit = digitalRead(GRIP_LIMIT);
    if (limit == false)
    {
      analogWrite(GRIP_PWM, 0);
    }
  }


  if (DC_control == true)
  {
    if (currentMillis - prevMillis >= tstep)
    {
      //Serial.println("DC_CON");
      prevMillis = currentMillis;
      int32_t newPosA = EncA.read();
      VelA = newPosA - oldPosA;
      //Serial.println(VelA*setpointA);
      oldPosA = newPosA;
      int32_t newPosB = EncB.read();
      VelB = newPosB - oldPosB;
      oldPosB = newPosB;
      if (pos_control == false)
      {
        if ((V_control_A == true) || (V_control_B == true))
        {
          if (V_control_A == true)
          {
            Vel_Control_Motor(PWMApin, INA1pin, INA2pin, setpointA, VelA, IntA_pt, stlA, V_A_pt);
          }
          if (V_control_B == true)
          {
            Vel_Control_Motor(PWMBpin, INB1pin, INB2pin, setpointB, VelB, IntB_pt, stlB, V_B_pt);
          }
        }
        else
        {
          DC_vel_clean();
        }
      }
      if (pos_control == true)
      {
        if (posAdone == false)
        {
          Pos_Control_Motor(PWMApin, INA1pin, INA2pin, ENC1Apin, ENC1Bpin, setpointA, TickswantedA, VelA, IntA_pt, newPosA, stlA, doneA);
        }
        if ( posBdone == false)
        {
          Pos_Control_Motor(PWMBpin, INB1pin, INB2pin, ENC2Apin, ENC2Bpin, setpointB, TickswantedB, VelB, IntB_pt, newPosB, stlB, doneB);
        }

        //Serial.println(VelA);
        //Serial.println(VelB);

        if (posAdone == true && posBdone == true)
        {
          DC_control = false;
          DC_clean();
          Serial.println("cleaned");
        }
      }
    }
  }
}

