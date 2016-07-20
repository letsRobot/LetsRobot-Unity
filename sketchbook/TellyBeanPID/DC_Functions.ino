

void DC_motor_loop (int16_t velocityA, int32_t pdistanceA, int16_t velocityB, int32_t pdistanceB)
{
  //Serial.println(pdistanceA);
  //Serial.println(pdistanceB);
  uint32_t currentMillis = 0;         // Current Time in ms
  uint16_t tstep = 1000 / cRate;      // Timestep between each control update
  int16_t VelA = 0;                   // Current Motor Velocity A
  int16_t IntA = 0;                   // Integral of Errors in Velocity A
  int16_t VelB = 0;                   // Current Motor Velocity B
  int16_t IntB = 0;                   // Integral of Errors in Velocity A
  int16_t *IntA_pt = &IntA;           // Pointer to INTA
  int16_t *IntB_pt = &IntB;           // Pointer to INTB
  stallA = 0;                         // Sets StallA to 0
  stallB = 0;                         // Sets StallB to 0
  bool *doneA = &posAdone;
  bool *doneB = &posBdone;
  uint8_t *stlA = &stallA;
  uint8_t *stlB = &stallB;
  posAdone = false;
  posBdone = false;
  oldPosA = 0;
  oldPosB = 0;

  // Resets the encoder counts to zero
  EncA.write(0);
  EncB.write(0);

  // Sets the motor sign to match the position 
  if (pdistanceA < 0)
  {
    velocityA = -1 * velocityA;
  }
  if (pdistanceB < 0)
  {
    velocityB = -1 * velocityB;
  }


  while (posAdone == false || posBdone == false)
  {
    currentMillis = millis();
    if (currentMillis - prevMillis >= tstep)
    {
      prevMillis = currentMillis;
      int32_t newPosA = EncA.read();
      VelA = newPosA - oldPosA;
      //Serial.println(VelA*setpointA);
      oldPosA = newPosA;
      int32_t newPosB = EncB.read();

      VelB = newPosB - oldPosB;
      oldPosB = newPosB;
      Control_Motor(PWMApin, INA1pin, INA2pin, ENC1Apin, ENC1Bpin, velocityA, pdistanceA, VelA, IntA_pt, newPosA, stlA, doneA);
      Control_Motor(PWMBpin, INB1pin, INB2pin, ENC2Apin, ENC2Bpin, velocityB, pdistanceB, VelB, IntB_pt, newPosB, stlB, doneB);

      //Serial.println(VelA);
      //Serial.println(VelB);
    }
  }
}

void Control_Motor (int8_t PWMpin, int8_t Dir1pin, int8_t Dir2pin, int8_t ENC1Apin, int8_t ENC2Bpin, int16_t velocity, int32_t pdistance, int16_t Vel, int16_t* Int, int32_t newPos, uint8_t* stall, bool* posdone)
{
  if ((pdistance > 0 && pdistance > velocity*5)||(pdistance < 0 && pdistance < velocity*5))
  {
    if ((pdistance > 0 && ((pdistance - cRate * velocity / ease) < newPos)) || (pdistance < 0 && ((pdistance - cRate * velocity / ease) > newPos) ))
    {
      int32_t thing = (pdistance-newPos)/(cRate/ease);
      //Serial.println(thing);
      //Serial.println(thing);
      //      velocity = thing;
      if (velocity < 0)
      {
        velocity = thing;
      }
      else if (velocity > 0)
      {
        velocity = thing;
      }

    }
  }
  *Int = *Int + velocity - Vel;

  if (*stall >= cRate / 3)
  {
    *posdone = true;
    analogWrite(PWMpin, 0);
    Serial.println("done");
  }

  else if ((pdistance > 0 && pdistance > newPos) || (pdistance < 0 && pdistance < newPos) )
  {
    //    Serial.println(velocity);
    //          Serial.println(Vel);
    int16_t PWMact = (velocity - Vel) * KP / cRate + *Int * KI / cRate;


    if (PWMact < 0)
    {
      digitalWrite(Dir1pin, LOW);
      digitalWrite(Dir2pin, HIGH);
      PWMact = -1 * PWMact;
    }
    else
    {
      digitalWrite(Dir1pin, HIGH);
      digitalWrite(Dir2pin, LOW);
    }
    if (PWMact >= 255)
    {
      PWMact = 255;
    }
    //    if (PWMact<50)
    //    {
    //    PWMact = 50;
    //    }

    analogWrite(PWMpin, PWMact);
    //Serial.println(velocity);
    //Serial.println(PWMact);
    //Serial.println(PWMact);
    //Serial.println(newPos);
    if ((Vel >= 0 && Vel <= 200 / cRate) || (Vel <= 0 && Vel >= -200 / cRate))
    {
      *stall = *stall + 1;
      //Serial.println(*stall);
    }
    else
    {
      *stall = 0;
      //Serial.println(9);
    }
  }
  else
  {
    analogWrite(PWMpin, 0);
    *posdone = true;
    Serial.println(newPos);

  }
}

void Motor_Setup(int8_t PWMpin, int8_t Dir1pin, int8_t Dir2pin)
{
  pinMode(PWMpin, OUTPUT);
  pinMode(Dir1pin, OUTPUT);
  pinMode(Dir2pin, OUTPUT);
}



