
void heartbeat(void)
{
  
  currentMillis = millis();

  if (currentMillis>=distance_timer && DC_control == true)
  {
    distance_timer = currentMillis + 5;
    int32_t newPosA = EncA.read()- last_ticksA;
    int32_t newPosB = EncB.read()- last_ticksB; 
    if (((TickswantedA > 0 && (TickswantedA  < newPosA)) || (TickswantedA < 0 && TickswantedA > newPosA) )&& posAdone==false)
    {
     analogWrite(PWMApin, 0);
      posAdone = true;
      Serial.println("adone");
    }
     if (((TickswantedB > 0 && (TickswantedB  < newPosB)) || (TickswantedB < 0 && TickswantedB > newPosB) )&& posBdone==false)
    {
     analogWrite(PWMBpin, 0);
      posBdone = true;
      Serial.println("bdone");
      Serial.println(TickswantedB);
      Serial.println(newPosB);
    }
  }
    if ((servo_done == true) && (servo_cmd_rear > servo_cmd_front))
    {
    char mcmd = servo_command_buf[servo_cmd_front];
    switch (mcmd){
    case 'p':
    cmd_pan();
    break;
    case 't':
    cmd_tilt();
    break;
    }
    servo_done = false;
    if (servo_cmd_front < SERVO_BUFF_LENGTH)
    servo_cmd_front = servo_cmd_front + 1;
 
  }
   if ((currentMillis - prevMillisServo) >= 20)
  {
    servo_loop();
  }
   if ((motion_done == true) && (move_cmd_rear > move_cmd_front))
  {
    //Serial.println("buf_int");
    char mcmd = move_command_buf[move_cmd_front];
    switch (mcmd){
    case'r':
    cmd_swipe_right();
    break;
    case'l':
    cmd_swipe_left();
    break;
    case'o':
    cmd_atics();
    break;
    case'p':
    cmd_pokeball();
    break;
    case't':
    cmd_ticks();
    break;
    case'h':
    cmd_tap();
    break;
    }

    motion_done = false;
    if (move_cmd_front < MOVE_BUFF_LENGTH)
    move_cmd_front = move_cmd_front + 1;

  }
  
  if (V_spin == true)
  {
    if (currentMillis >= velocity_time)
    {
    velocity_time = millis()+30;


    if (setpointA >= A_spinmax/cRate)
    spin_valA = -A_spinmax/5/cRate;
    else if (setpointA <= -A_spinmax/cRate)
    spin_valA = A_spinmax/5/cRate;
    
    if (setpointB >= B_spinmax/cRate)
    spin_valB = -B_spinmax/5/cRate;
    else if (setpointB <= -B_spinmax/cRate)
    spin_valB = B_spinmax/5/cRate;
    
    setpointB = setpointB + spin_valB;
    setpointA = setpointA + spin_valA;
    }
  }
  
  if (0 < stop_time && stop_time < millis())
  {stop();}
  
  if (pokeball == true)
  {
    if (currentMillis >= poketimer)
    {
      Serial.println("Pika");
      pokeball = false;
      digitalWrite(solenoid, LOW);
      if(move_command_buf[move_cmd_front-1]=='h')
      motion_done = true;
      Serial.println("tapclr");
      if (move_cmd_rear <= move_cmd_front)
    {
      move_cmd_rear = 0;
      move_cmd_front = 0;
      //Serial.println("buffclr");
    }
    }
  }
  if ((currentMillis - prevMillis) >= tstep)
  {
    dc_loop();
    
  }
}

