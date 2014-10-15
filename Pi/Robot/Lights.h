#ifndef Lights_h
#define Lights_h

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

class Lights
{
   public:
      Lights(const char * devName, int address)
      {
         if((file = open(devName, O_RDWR)) < 0)
         {
            fprintf(stderr, "I2C: Failed to access %s.\n", devName);
            exit(1);
         }

         if (ioctl(file, I2C_SLAVE, address) < 0)
         {
            fprintf(stderr, "I2C: Failed to acquire bus access/talk to slave 0x%x\n", address);
            exit(1);
         }

         printf("Connected to Arduino.\n");
      }

      ~Lights()
      {
         close(file);
      }

      void SetLight(int light, int r, int g, int b)
      {
         SendByte(light);
         SendByte(r);
         SendByte(g);
         SendByte(b);
      }

   private:
      void SendByte(char byte)
      {
         if (write(file, &byte, 1) == 1)
         {
            usleep(10000);

            char buf[1];
            if (read(file, buf, 1) == 1)
            {
               int temp = (int) buf[0];
//               printf("Received %d\n", temp);
            }
         }

         usleep(10000);
      }

      int file;
};

#endif
