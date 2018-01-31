//gcc test_grab.c -std=gnu11 -fdiagnostics-color -Wall -Wno-missing-braces -lwiringPi -o grab

#include "util.h"

#define Lepton_Assert Assert3arg

#include "../Lepton/Lepton_SPI.h"
#include "../Lepton/Lepton_I2C.h"
#include "../Lepton/Lepton_Strings.h"
#include "../Lepton/Lepton_Stream.h"

//printf
#include <stdio.h>

//timerfd_create
#include <sys/timerfd.h>

//read
#include <unistd.h>

//strerror
#include <string.h>

//errrno
#include <errno.h>

//INT_EDGE_RISING
//wiringPiISR
//wiringPiSetup
#include <wiringPi.h>





int I2C_Device;
int SPI_Device;
struct Lepton_Pixel_Grayscale16 Pixmap [Lepton3_Width * Lepton3_Height];
_Atomic size_t Safe_Counter = 0;


void Enable_Vsync (int Device, int Micro_Sleep, size_t Trial_Count)
{
   Log ("%s", "Enabling video syncronization pulse at GPIO3 pin.");
   struct Lepton_I2C_GPIO_Mode Mode;
   Mode.Value = htobe16 (Lepton_I2C_GPIO_Mode_Vsync);
   int Stage = 0;
   uint16_t Status;
   for (size_t I = 0; I < Trial_Count; I = I + 1)
   {
      Lepton_I2C_Execute 
      (
         Device, 
         Lepton_I2C_Command_GPIO_Mode_Set, 
         (void *) &(Mode),
         sizeof (struct Lepton_I2C_GPIO_Mode),
         &(Status),
         &(Stage)
      );
      if (Stage == 0) {break;}
      usleep (Micro_Sleep);
   }
}


void Reboot (int Device)
{
   int Status;
   Status = Lepton_I2C_Status (Device);
   Lepton_Strings_Base_printf (stderr, be16toh (Status), 10, 2, "Status: %10s\n");
   Lepton_I2C_Write_Command (Device, Lepton_I2C_Command_Reboot);
   sleep (3);
   Enable_Vsync (Device, 10, 10);
   Status = Lepton_I2C_Status (Device);
   Lepton_Strings_Base_printf (stderr, be16toh (Status), 10, 2, "Status: %10s\n");
}


void Interrupt_Handle ()
{
   int32_t Result;
   Result = Lepton_Stream_Accept (SPI_Device, Pixmap);
   if (Result != 4) {return;}
   Safe_Counter = 0;
   Result = write (STDOUT_FILENO, Pixmap, sizeof (Pixmap));
   Assert (Result == sizeof (Pixmap), "wrote %i of %i to STDOUT_FILENO", Result, sizeof (Pixmap));
}


void Setup_GPIO3 ()
{
   int Result;
   Result = wiringPiSetup ();
   Assert (Result >= 0, "wiringPiSetup returned %i", Result);
   piHiPri (99);
   int Pin = 0;
   int Edge = INT_EDGE_RISING;
   wiringPiISR (Pin, Edge, &Interrupt_Handle);
}


int main (int argc, char * argv [])
{ 
   Assert (argc == 1, "argc %i.", argc);
   Assert (argv[0] != NULL, "argv0 %p.", argv[0]);
   
   I2C_Device = Lepton_I2C_Open ("/dev/i2c-1");
   Reboot (I2C_Device);
   SPI_Device = Lepton_SPI_Open ("/dev/spidev0.0");
   Setup_GPIO3 ();
   while (1)
   {
      sleep (1);
      //Check if the camera goes haywire.
      Safe_Counter ++;
      if (Safe_Counter > 3)
      {
         Safe_Counter = 0;
         Log ("%s", "Camera has gone haywire! Rebooting now to fix the problem.");
         Reboot (I2C_Device);
      }
   }
}



