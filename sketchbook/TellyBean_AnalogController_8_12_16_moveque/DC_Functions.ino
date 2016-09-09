

void Pos_DC_motor_loop ()
{
  Serial.println("POS");
  //Serial.println(pdistanceB);

  if (TickswantedA < 0)
  {
    setpointA = -1 * setpointAdefault;
  }
  if (TickswantedB < 0)
  {
    setpointB = -1 * setpointBdefault;
  }
  DC_control = true;
  pos_control = true;
}

void Vel_DC_motor_loop ()
{
  Serial.println("Vel");
  if ((V_control_A == false)&&(V_control_B == false))
  {
    EncA.write(0);
    EncB.write(0);
  }
  DC_control = true;
  V_control_A = true;
  V_control_B = true;
}

void Pos_Control_Motor (int8_t PWMpin, int8_t Dir1pin, int8_t Dir2pin, int8_t ENC1Apin, int8_t ENC2Bpin, int16_t velocity, int32_t pdistance, int16_t Vel, int16_t* Int, int32_t newPos, uint8_t* stall, bool* posdone)
{ 
  
if (((pdistance > 0 && pdistance > 500) || (pdistance < 0 && pdistance < 500)&& (abs(velocity)>=100)))
  {
    if ((pdistance > 0 && ((pdistance - cRate * velocity / ease) < newPos)) || (pdistance < 0 && ((pdistance - cRate * velocity / ease) > newPos) ))
    {
      int32_t thing = (pdistance - newPos) * ease / cRate;
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
    //*posdone = true;
    posAdone = true;
    posBdone = true;
    analogWrite(PWMApin, 0);
    analogWrite(PWMBpin, 0);
    Serial.println("stall");
  }

  else if ((pdistance > 0 && pdistance > newPos) || (pdistance < 0 && pdistance < newPos) )
  {

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

    analogWrite(PWMpin, PWMact);
    //Serial.println(PWMact);
    //Serial.println(newPos);
    if ((Vel >= 0 && Vel <= 300 / cRate) || (Vel <= 0 && Vel >= -300 / cRate))
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
     posAdone = true;
    posBdone = true;
    analogWrite(PWMApin, 0);
    analogWrite(PWMBpin, 0);
    //*posdone = true;
    Serial.println(newPos);
  }
}

void Vel_Control_Motor (int8_t PWMpin, int8_t Dir1pin, int8_t Dir2pin, int16_t velocity, int16_t Vel, int16_t* Int, uint8_t* stall,bool* V_C_pt)
{
  *Int = *Int + velocity - Vel;
  int16_t PWMact = (velocity - Vel) * KP / cRate + *Int * KI / cRate;
  //Serial.println(Vel);

  if (*stall >= 6)
  {
    analogWrite(PWMpin, 0);
    Serial.println("stall");
    *V_C_pt = false;
    *stall = 0;
  }
  else
  {
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
    //Serial.println(PWMact);
    analogWrite(PWMpin, PWMact);
    if ((Vel >= 0 && Vel <= 300 / cRate && velocity >= 0) || (Vel <= 0 && Vel >= -300 / cRate && velocity <= 0))
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
}


void Motor_Setup(int8_t PWMpin, int8_t Dir1pin, int8_t Dir2pin)
{
  pinMode(PWMpin, OUTPUT);
  pinMode(Dir1pin, OUTPUT);
  pinMode(Dir2pin, OUTPUT);
}

void DC_clean (void)
{
  IntA = 0;
  IntB = 0;
  stallA = 0;
  stallB = 0;
  posAdone = false;
  posBdone = false;
  oldPosA = 0;
  oldPosB = 0;
  setpointA = setpointAdefault;   // Desired Motor Velocity (Ticks/Second)
  setpointB = setpointBdefault;   // Desired Motor Velocity (Ticks/Second)
  TickswantedA = 0;
  TickswantedB = 0;
  EncA.write(0);
  EncB.write(0);
  motion_done = true;
  pos_control = false;
  if (move_cmd_rear <= move_cmd_front)
    {
      move_cmd_rear = 0;
      move_cmd_front = 0;
      Serial.println("buffclr");
    }

}

void DC_vel_clean(void)
{
  stallA = 0;
  stallB = 0;
  IntA = 0;
  IntB = 0;
  analogWrite(PWMApin, 0);
  analogWrite(PWMBpin, 0);
  oldPosA = 0;
  oldPosB = 0;
  setpointA = setpointAdefault;   // Desired Motor Velocity (Ticks/Second)
  setpointB = setpointBdefault;   // Desired Motor Velocity (Ticks/Second)
  EncA.write(0);
  EncB.write(0);
  DC_control = false;
  Serial.println("v clean");
}


