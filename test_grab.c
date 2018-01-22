//gcc test_grab.c -std=gnu11 -fdiagnostics-color -Wall -Wno-missing-braces -lwiringPi -o grab

#include "util.h"

//#define Lepton_Log_Assert Log_Assert

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


void Lepton_App_Enable_Vsync (int Device, int Micro_Sleep, size_t Trial_Count)
{
   Log ("Enabling video syncronization pulse on FLIR Lepton GPIO3 pin.");
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


void Interrupt_Handle ()
{
   int Result;
   Result = Lepton_Stream_Accept (SPI_Device, Pixmap);
   switch (Result)
   {
      case Lepton_Stream_SPI_Error:
      Log ("Lepton_Stream_SPI_Error");
      break;
      case Lepton_Stream_Shifting:
      //Log ("Lepton_Stream_Shifting");
      break;
      case Lepton_Stream_Invalid_Row:
      //Log ("Lepton_Stream_Invalid_Row");
      break;
      case Lepton_Stream_Discard:
      //Log ("Lepton_Stream_Discard");
      break;
      case Lepton_Stream_Mismatch:
      //Log ("Lepton_Stream_Mismatch");
      break;
      case Lepton_Stream_Invalid_Segment:
      //Log ("Lepton_Stream_Invalid_Segment");
      break;
   };
   
   if (Result == 4)
   {
      Safe_Counter = 0;
      
      int R = write (STDOUT_FILENO, Pixmap, sizeof (Pixmap));
      Log_Assert (R == sizeof (Pixmap), "write");
   }
}


int main (int argc, char * argv [])
{ 
   I2C_Device = Lepton_I2C_Open ("/dev/i2c-1");
   //SPI_Device = Lepton_SPI_Open (Lepton_Debug_None, "/dev/spidev0.0");
   SPI_Device = Lepton_SPI_Open ("/dev/spidev0.0");
   
   {
      int Status = Lepton_I2C_Status (I2C_Device);
      Lepton_Strings_Base_printf (stderr, be16toh (Status), 10, 2, "Status: %10s\n");
   }
   
   Lepton_I2C_Write_Command (I2C_Device, Lepton_I2C_Command_Reboot);
   sleep (3);
   Lepton_App_Enable_Vsync (I2C_Device, 10, 10);
   
   {
      int Status = Lepton_I2C_Status (I2C_Device);
      Lepton_Strings_Base_printf (stderr, be16toh (Status), 10, 2, "Status: %10s\n");
   }
      
   {
      Log ("wiringPiSetup");
      wiringPiSetup ();
      piHiPri (99);
      int Pin = 0;
      int Edge = INT_EDGE_RISING;
      wiringPiISR (Pin, Edge, &Interrupt_Handle);
   }
   
   while (1)
   {
      sleep (1);
      //Check if the camera goes haywire.
      Safe_Counter ++;
      if (Safe_Counter > 3)
      {
         Safe_Counter = 0;
         Log ("Camera has gone haywire! Rebooting now to fix the problem.");
         Lepton_I2C_Write_Command (I2C_Device, Lepton_I2C_Command_Reboot);
         sleep (3);
         Lepton_App_Enable_Vsync (I2C_Device, 10, 10);
      }
   }
}



