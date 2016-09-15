void cmd_rover (void)
{
  int32_t value = nextarg_int(-500, 500, 0, -1);
  int32_t value2 = nextarg_int(-500, 500, 0, -1);
  uint32_t time_stop = nextarg_int(0, 10000, 0, -1);
  schedule_stop(time_stop);
  DC_Rover(value, value2);
}

void cmd_forward (void)
{
  schedule_stop(default_stop_time);
  DC_Rover(rover_speed, -rover_speed);
}


void cmd_back    (void)
{
  schedule_stop(default_stop_time);
  DC_Rover(-rover_speed, rover_speed);
}


void cmd_left    (void)
{
  schedule_stop(default_stop_time );
  DC_Rover(-rover_speed-70, -rover_speed-70);
}


void cmd_right   (void)
{
  schedule_stop(default_stop_time );
  DC_Rover(rover_speed+70, rover_speed+70);
}


void cmd_stop(void) {
  if (stop_time)
    schedule_stop(1);
  heartbeat();
  OK();
}

void cmd_rover_speed(void)
{
  int32_t value = nextarg_int(1, 160, 0, -1);
  Serial.println(format("Setting Rover Velocity %d", value));
  rover_speed = value;
}
