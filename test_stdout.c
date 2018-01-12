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

/*
enum Log_Type 
{
   Log_Info,
   Log_Cricital
};

void Log (enum Log_Type Type, FILE * File)
{
   switch (Type)
   {
      case Log_Info:
      fprintf ();
   };
}
*/

FILE * Logfile = NULL;
int I2C_Device;
int SPI_Device;
struct Lepton_Pixel_Grayscale16 Pixmap [Lepton3_Width * Lepton3_Height];
_Atomic size_t Safe_Counter = 0;


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
         Lepton_Debug_None,
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
      fprintf (Log, "Lepton_Stream_SPI_Error\n");break;
      case Lepton_Stream_Shifting:
      fprintf (Log, "Lepton_Stream_Shifting\n");break;
      case Lepton_Stream_Invalid_Row:
      fprintf (Log, "Lepton_Stream_Invalid_Row\n");break;
      case Lepton_Stream_Discard:
      fprintf (Log, "Lepton_Stream_Discard\n");break;
      case Lepton_Stream_Mismatch:
      fprintf (Log, "Lepton_Stream_Mismatch\n");break;
      case Lepton_Stream_Invalid_Segment:
      fprintf (Log, "Lepton_Stream_Invalid_Segment\n");break;
      break;
   };
   
   if (Result == 4)
   {
      Safe_Counter = 0;
      
      int R = write (STDOUT_FILENO, Pixmap, sizeof (Pixmap));
      assert (R == sizeof (Pixmap));
   }
}


int main (int argc, char * argv [])
{ 
   I2C_Device = Lepton_I2C_Open (Lepton_Debug_None, "/dev/i2c-1");
   //SPI_Device = Lepton_SPI_Open (Lepton_Debug_None, "/dev/spidev0.0");
   SPI_Device = Lepton_SPI_Open ("/dev/spidev0.0");
   
   Logfile = fopen ("file.txt", "w+");
   assert (Logfile != NULL);
   setbuf (Logfile, NULL);
   fprintf (Logfile, "Test\n");
   
   {
      int Status = Lepton_I2C_Status (Lepton_Debug_None, I2C_Device);
      Lepton_Strings_Base_printf (stderr, be16toh (Status), 10, 2, "Status: %10s\n");
   }
   
   Lepton_I2C_Write_Command (Lepton_Debug_None, I2C_Device, Lepton_I2C_Command_Reboot);
   sleep (3);
   Lepton_App_Enable_Vsync (I2C_Device, 10, 10);
   
   {
      int Status = Lepton_I2C_Status (Lepton_Debug_None, I2C_Device);
      Lepton_Strings_Base_printf (stderr, be16toh (Status), 10, 2, "Status: %10s\n");
   }
      
   {
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
         fprintf (stderr, "\nCamera has gone haywire! Rebooting now to fix the problem.");
         Lepton_I2C_Write_Command (Lepton_Debug_None, I2C_Device, Lepton_I2C_Command_Reboot);
         sleep (3);
         Lepton_App_Enable_Vsync (I2C_Device, 10, 10);
      }
   }
}



