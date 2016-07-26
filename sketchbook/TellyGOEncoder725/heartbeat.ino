
void heartbeat(void)
{ 
  currentMillis = millis();
   if (0 < stop_time && stop_time < millis())
        stop();
   if (pokeball == true)
   {
    if (currentMillis >= poketimer)
    {
      Serial.println("Pika");
      pokeball = false;
      digitalWrite(solenoid,LOW);
    }
   }
  if ((currentMillis - prevMillis) >= tstep)
  {
    prevMillis = currentMillis;
    if (pan_go == true)
    {
      if (pan_angle > pan_angle_prev)
      {
        pan_desire = pan_desire +  a_step;
        //Serial.println(pan_desire);
        pan_servo.write(pan_desire);
        if (pan_desire >= pan_angle)
        {
          pan_go = false;
        }
      }
      else if (pan_angle < pan_angle_prev)
      {
        pan_desire = pan_desire -  a_step;
        pan_servo.write(pan_desire);
        //Serial.println(pan_desire);
        if (pan_desire <= pan_angle)
        {
          pan_go = false;
        }
      }
    }
    else
    {
      pan_angle_prev = pan_angle;
    }
    if (tilt_go == true)
    {
      if (tilt_angle > tilt_angle_prev)
      {
        tilt_desire = tilt_desire +  a_step;
        //Serial.println(tilt_desire);
        tilt_servo.write(tilt_desire);
        if (tilt_desire >= tilt_angle)
        {
          tilt_go = false;
        }
      }
      else if (tilt_angle < tilt_angle_prev)
      {
        tilt_desire = tilt_desire -  a_step;
        tilt_servo.write(tilt_desire);
        //Serial.println(tilt_desire);
        if (tilt_desire <= tilt_angle)
        {
          tilt_go = false;
        }
      }
    }
    else
    {
      tilt_angle_prev = tilt_angle;
    }

    if (DC_control == true)
    {
      //Serial.println("DC_CON");

      int32_t newPosA = EncA.read();
      VelA = newPosA - oldPosA;
      //Serial.println(VelA*setpointA);
      oldPosA = newPosA;
      int32_t newPosB = EncB.read();
      VelB = newPosB - oldPosB;
      oldPosB = newPosB;
      //Serial.println(VelA);
      //Serial.println(last_ticksA);
      newPosA = newPosA - last_ticksA;
      newPosB = newPosB - last_ticksB;
      //      Serial.println(newPosA);
      if (V_control_A == true || V_control_B == true)
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
        if (posAdone == false || posBdone == false)
        {
          limitA = digitalRead(LIMIT_A_PIN);
          limitB = digitalRead(LIMIT_B_PIN);
          if (limitA == false && limitAfound == false)
          {
            EncA.write(0);
            posAdone = true;
            analogWrite(PWMApin, 0);
            oldPosA = 0;
            limitAfound = true;
          }
          else if (posAdone == false)
          {
            if ((((TickswantedA+last_ticksA) >= 0)&&((TickswantedA+last_ticksA) <= 3576)) || limitAfound == false)
              Pos_Control_Motor(PWMApin, INA1pin, INA2pin, ENC1Apin, ENC1Bpin, setpointA, TickswantedA, VelA, IntA_pt, newPosA, stlA, doneA, lmtA);
            else
            {
              posAdone = true;
              analogWrite(PWMApin, 0);
            }
          }

          if (limitB == false && limitBfound == false)
          {
            EncB.write(0);
            posBdone = true;
            analogWrite(PWMBpin, 0);
            oldPosB = 0;
            limitBfound = true;
          }
          else if (posBdone == false)
          {
            if ((((TickswantedB+last_ticksB) >= 0)&&((TickswantedB+last_ticksB) <= 6500)) || limitBfound == false)
              Pos_Control_Motor(PWMBpin, INB1pin, INB2pin, ENC2Apin, ENC2Bpin, setpointB, TickswantedB, VelB, IntB_pt, newPosB, stlB, doneB, lmtB);
          
          else
          {
            posBdone = true;
            analogWrite(PWMBpin, 0);
          }
          }
        }

        //Serial.println(VelA);
        //Serial.println(VelB);

        else if (posAdone == true && posBdone == true)
        {
          DC_control = false;
          DC_clean();
          Serial.println("cleaned");
        }
      }
    }
  }
}

