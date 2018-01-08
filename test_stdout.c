//gcc test_stdout.c -std=gnu11 -fdiagnostics-color -Wall -Wno-missing-braces -lwiringPi -o grab

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
size_t Segment_Per_Frame = 0;


void Lepton_App_Enable_Vsync (int Device, int Micro_Sleep, size_t Trial_Count)
{
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
      case Lepton_Stream_Shifting:
      case Lepton_Stream_Invalid_Row:
      case Lepton_Stream_Discard:
      case Lepton_Stream_Mismatch:
      case Lepton_Stream_Invalid_Segment:
      break;
   };
   
   Segment_Per_Frame ++;
   
   if (Result == 4)
   {
      //Should be 4 segments but it is 12 due to US Export restriction.
      Segment_Per_Frame = 0;
      
      int R = write (STDOUT_FILENO, Pixmap, sizeof (Pixmap));
      assert (R == sizeof (Pixmap));
   }
   
   //Check if the camera goes haywire.
   //There should only be 4 segment (*3 if US Export restriction) per frame in Lepton 3.
   if (Segment_Per_Frame > 300)
   {
      Segment_Per_Frame = 0;
      Lepton_I2C_Write_Command (I2C_Device, Lepton_I2C_Command_Reboot);
      sleep (3);
      Lepton_App_Enable_Vsync (I2C_Device, 10, 10);
   }
   
   
}


int main (int argc, char * argv [])
{ 
   I2C_Device = Lepton_I2C_Open ("/dev/i2c-1");
   SPI_Device = Lepton_SPI_Open ("/dev/spidev0.0");
   int Status = Lepton_I2C_Status (I2C_Device);
   Lepton_Strings_Base_printf (be16toh (Status), 10, 2, "Status: %10s\n");
   
   {
      wiringPiSetup ();
      piHiPri (99);
      int Pin = 0;
      int Edge = INT_EDGE_RISING;
      wiringPiISR (Pin, Edge, &Interrupt_Handle);
   }
   
   while (1)
   {
      //printf ("S/P: %d\n", Segment_Per_Frame);
      sleep (1);
   }
}



