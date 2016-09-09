uint8_t speed_counter = 0;

//Starts the control of the Encoded DC Motors and sets desired velocity in direction of wanted position
void Pos_DC_motor_loop ()
{
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

// Starts the control of encoded motors based on current setpoints
void Vel_DC_motor_loop ()
{
  Serial.println("Vel");
  DC_control = true;
  V_control_A = true;
  V_control_B = true;
  V_control = true;
}

//PI controller with stall detection for encoded motors.
void Pos_Control_Motor (int8_t PWMpin, int8_t Dir1pin, int8_t Dir2pin, int8_t ENC1Apin, int8_t ENC2Bpin, int16_t velocity, int32_t pdistance, int16_t Vel, int16_t* Int, int32_t newPos, uint8_t* stall, bool* posdone, bool* limit, uint8_t scale)
{ 
  *Int = *Int + velocity - Vel;
  if (*stall >= cRate/2)
  {
    *posdone = true;
    analogWrite(PWMpin, 0);

    Serial.println("stalled");
  }
  else if ((pdistance > 0 && (pdistance > newPos)) || (pdistance < 0 && pdistance < newPos) )
  {

    int16_t PWMact = (velocity - Vel) * KP* scale / cRate + *Int * KI *scale / cRate;
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
    //Serial.println(PWMact);
    //Serial.println(PWMact);
    analogWrite(PWMpin, PWMact);


    if ((Vel >= 0 && Vel <= 80 / cRate) || (Vel <= 0 && Vel >= -80 / cRate))
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
  }
}

void Vel_Control_Motor (int8_t PWMpin, int8_t Dir1pin, int8_t Dir2pin, int16_t velocity, int16_t Vel, int16_t* Int, uint8_t* stall, bool* V_C_pt, uint8_t scale)
{
  *Int = *Int + velocity - Vel;

  int16_t PWMact = (velocity - Vel) * KP / cRate * scale + *Int * KI / cRate * scale;

  if (*stall >= 10)
  {
    analogWrite(PWMpin, 0);
    Serial.println("done");
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
    Serial.println("OP");
    Serial.println(Vel);
    Serial.println(velocity);
    analogWrite(PWMpin, PWMact);
    if ((Vel >= 0 && Vel <= 150 / cRate) || (Vel <= 0 && Vel >= -150 / cRate))
    {
      //*stall = *stall + 1;
      //Serial.println("stl");
      //Serial.println(*stall);
    }
    else
    {
      *stall = 0;
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
  DC_control = false;
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
  scale_x = 1;
  scale_y = 2;
  motion_done = true;
    if (move_cmd_rear <= move_cmd_front)
    {
      move_cmd_rear = 0;
      move_cmd_front = 0;
      //Serial.println("buffclr");
    }

}

void DC_vel_clean(void)
{
  DC_control = false;
  IntA = 0;
  IntB = 0;
  stallA = 0;
  stallB = 0;
  V_control = false;
  last_ticksA = EncA.read();
  last_ticksB = EncB.read();
  setpointA = setpointAdefault;   // Desired Motor Velocity (Ticks/Second)
  setpointB = setpointBdefault;   // Desired Motor Velocity (Ticks/Second)
  V_spin = false;
  Serial.println("Vel_clean");
}



void DC_Rover (int16_t velocityA, int16_t velocityB)
{
  Control_Rover(PWMMR_PIN, INMR1_PIN, INMR2_PIN, velocityA);
  Control_Rover(PWMML_PIN, INML1_PIN, INML2_PIN,  velocityB);
}

void Control_Rover (int8_t PWMpin, int8_t Dir1pin, int8_t Dir2pin, int16_t velocity)
{ 
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

void dc_loop (){
  prevMillis = currentMillis;
    if (DC_control == true)
    {
      

      int32_t newPosA = EncA.read();
      VelA = newPosA - oldPosA;
      //Serial.println(VelA*setpointA);
      oldPosA = newPosA;
      int32_t newPosB = EncB.read();
      VelB = newPosB - oldPosB;
      oldPosB = newPosB;
      //Serial.println(VelA);
      //Serial.println(last_ticksA);
      int16_t velposA = newPosA;
      int16_t velposB = newPosB;
      newPosA = newPosA - last_ticksA;
      newPosB = newPosB - last_ticksB;
      speed_counter = speed_counter + 1;
      if (speed_counter >=9);
      {
      Serial.println(VelB);
      speed_counter = 0;
      }
      if (V_control == true)
      {
        if (V_control_A == true)
        {
          
          if (((((velposA) >= -100) ) && ((velposA) <= A_limit)))
              Vel_Control_Motor(PWMApin, INA1pin, INA2pin, setpointA, VelA, IntA_pt, stlA, V_A_pt, scale_x);
           else
            {
              V_control_A = false;
              analogWrite(PWMApin, 0);
              Serial.println("AlimitV");
            }
        }
        if (V_control_B == true)
        {
          
            if ((((velposB) >= -100 ) && ((velposB) <= B_limit)))
              Vel_Control_Motor(PWMBpin, INB1pin, INB2pin, setpointB, VelB, IntB_pt, stlB, V_B_pt, scale_y);
            else
            {
              V_control_B = false;
              analogWrite(PWMBpin, 0);
              Serial.println("BlimitV");
              Serial.println(TickswantedB);
            }
        }
        if((V_control_A == false) &&(V_control_B == false))
        {
          DC_vel_clean();
        }
      }
      else if ((V_control == false))
      {
        if (posAdone == false || posBdone == false)
        {
        //Serial.println("DC_CON");
          
          if (limitAfound == false && limitA == true)
          {
            limitA = digitalRead(LIMIT_A_PIN);
            if (limitA == false )
            {
              EncA.write(0);
              posAdone = true;
              analogWrite(PWMApin, 0);
              oldPosA = 0;
              limitAfound = true;
              Serial.println("lmtAfd");
            }
          }
          if (posAdone == false)
          {
            if ((((TickswantedA + last_ticksA) >= 0) && ((TickswantedA + last_ticksA) <= A_limit)) || limitAfound == false)
              Pos_Control_Motor(PWMApin, INA1pin, INA2pin, ENC1Apin, ENC1Bpin, setpointA, TickswantedA, VelA, IntA_pt, newPosA, stlA, doneA, lmtA,scale_x);
            else
            {
              posAdone = true;
              analogWrite(PWMApin, 0);
              Serial.println("Afail");
              Serial.println(TickswantedA);
            }
          }
          if (limitBfound == false && limitB == true)
          {
            limitB = digitalRead(LIMIT_B_PIN);
            if (limitB == false)
            {
              EncB.write(0); 
              posBdone = true;
              analogWrite(PWMBpin, 0);
              oldPosB = 0;
              limitBfound = true;
              Serial.println("lmtBfd");
            }
          }
          if (posBdone == false)
          {
            if ((((TickswantedB + last_ticksB) >= 0) && ((TickswantedB + last_ticksB) <= B_limit)) || limitBfound == false)
            {
              Pos_Control_Motor(PWMBpin, INB1pin, INB2pin, ENC2Apin, ENC2Bpin, setpointB, TickswantedB, VelB, IntB_pt, newPosB, stlB, doneB, lmtB,scale_y);
               //Serial.println("bcontrol");
            }
            else
            {
              posBdone = true;
              analogWrite(PWMBpin, 0);
              Serial.println("bdone");
              Serial.println(TickswantedB);
              Serial.println(last_ticksB);
            }
          }
        }

        //Serial.println(VelA);
        //Serial.println(VelB);

        else if (posAdone == true && posBdone == true)
        {
          DC_clean();
          Serial.println("pos_cleaned");
        }
      }
    }
}

