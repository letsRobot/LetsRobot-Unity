#include <SerialCommand.h>
#include <SoftwareSerial.h>

SerialCommand sCmd;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  sCmd.addCommand("PING", pingHandler);
  sCmd.addCommand("ECHO", echoHandler);
  //sCmd.setDefaultHandler(errorHandler);
}

void loop() {
  
  // Your operations here

  if (Serial.available() > 0)
    sCmd.readSerial();

  delay(50);  
}

void pingHandler ()
{
  Serial.println("PONG");
}

void echoHandler ()
{
  char *arg;
  arg = sCmd.next();
  if (arg != NULL)
    Serial.println(arg);
  else
    Serial.println("nothing to echo");
}

//arg errorHandler (const char *command)
//{
  // Error handling
//}
