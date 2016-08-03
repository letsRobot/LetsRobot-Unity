


void Pos_DC_motor_loop ()
{
  Serial.println("POS");
  //Serial.println(pdistanceB);
  if (TickswantedA < 0)
  {
    setpointA = -1 * setpointA;
  }
  if (TickswantedB < 0)
  {
    setpointB = -1 * setpointB;
  }
  DC_control = true;
}

void Vel_DC_motor_loop ()
{
  Serial.println("Vel");
  if (V_control_A == false && V_control_B == false)
  {
    EncA.write(0);
    EncB.write(0);
  }
  DC_control = true;
  V_control_A = true;
  V_control_B = true;
}

void Pos_Control_Motor (int8_t PWMpin, int8_t Dir1pin, int8_t Dir2pin, int8_t ENC1Apin, int8_t ENC2Bpin, int16_t velocity, int32_t pdistance, int16_t Vel, int16_t* Int, int32_t newPos, uint8_t* stall, bool* posdone, bool* limit)
{ 
  *Int = *Int + velocity - Vel;
  //  Serial.println(*Int);
  //  Serial.println(velocity-Vel);
  //  Serial.println(" ");
  if (*stall >= cRate / 2)
  {
    *posdone = true;
    analogWrite(PWMpin, 0);

    Serial.println("stldone");
  }

  

  else if ((pdistance > 0 && (pdistance > newPos)) || (pdistance < 0 && pdistance < newPos) )
  {

    int16_t PWMact = (velocity - Vel) * KP / cRate + *Int * KI / cRate;
    //Serial.println(Vel);
    //Serial.println(velocity);
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


    if ((Vel >= 0 && Vel <= 300 / cRate) || (Vel <= 0 && Vel >= -300 / cRate))
    {
      *stall = *stall + 1;
      //Serial.println("stl");
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
    Serial.println("pdone");
   Serial.println(pdistance);
    Serial.println(newPos);
    //Serial.println("Finish");
  }
}

void Vel_Control_Motor (int8_t PWMpin, int8_t Dir1pin, int8_t Dir2pin, int16_t velocity, int16_t Vel, int16_t* Int, uint8_t* stall, bool* V_C_pt)
{
//  *Int = *Int + velocity - Vel;
//
//  int16_t PWMact = (velocity - Vel) * KP / cRate + *Int * KI / cRate;
//
//  if (*stall >= 5)
//  {
//    analogWrite(PWMpin, 0);
//    Serial.println("done");
//    *V_C_pt = false;
//    *stall = 0;
//  }
//  else
//  {
//    if (PWMact < 0)
//    {
//      digitalWrite(Dir1pin, LOW);
//      digitalWrite(Dir2pin, HIGH);
//      PWMact = -1 * PWMact;
//    }
//    else
//    {
//      digitalWrite(Dir1pin, HIGH);
//      digitalWrite(Dir2pin, LOW);
//    }
//    if (PWMact >= 255)
//    {
//      PWMact = 255;
//    }
//    //Serial.println(Vel);
//    //Serial.println(velocity);
//    analogWrite(PWMpin, PWMact);
//    if ((Vel >= 0 && Vel <= 150 / cRate) || (Vel <= 0 && Vel >= -150 / cRate))
//    {
//      *stall = *stall + 1;
//      Serial.println("stl");
//      //Serial.println(*stall);
//    }
//    else
//    {
//      *stall = 0;
//      //Serial.println(9);
//    }
//  }
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
  setpointA = setpointAdefault;   // Desired Motor Velocity (Ticks/Second)
  setpointB = setpointBdefault;   // Desired Motor Velocity (Ticks/Second)
  last_ticksA = EncA.read();
  last_ticksB = EncB.read();

}

void DC_vel_clean(void)
{
  IntA = 0;
  IntB = 0;
}



void DC_Rover (int16_t velocityA, int16_t velocityB)
{
  Control_Rover(PWMMR_PIN, INMR1_PIN, INMR2_PIN, velocityA);
  Control_Rover(PWMML_PIN, INML1_PIN, INML2_PIN,  velocityB);
}

void Control_Rover (int8_t PWMpin, int8_t Dir1pin, int8_t Dir2pin, int16_t velocity)
{ //Serial.println("MotorC");
  if (velocity < 0)
  {
    digitalWrite(Dir1pin, LOW);
    digitalWrite(Dir2pin, HIGH);
    velocity = -1 * velocity;
     //Serial.println(velocity);
  }
  else
  {
    digitalWrite(Dir1pin, HIGH);
    digitalWrite(Dir2pin, LOW);
  }
  if (velocity >= 255)
  {
    velocity = 255;
  }

  analogWrite(PWMpin, velocity);
}



