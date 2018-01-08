//gcc test_save.c -std=gnu11 -fdiagnostics-color -Wall -Wno-missing-braces -lwiringPi -o save

#include "../Lepton/Lepton_SPI.h"
#include "../Lepton/Lepton_I2C.h"
#include "../Lepton/Lepton_Strings.h"
#include "../Lepton/Lepton_App.h"
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


/*
int main (int argc, char * argv [])
{ 
   Lepton_App_Init (&App, "/dev/i2c-1", "/dev/spidev0.0");
   Lepton_Strings_Base_printf (be16toh (App.Status), 10, 2, "Status: %10s\n");
   
   App->Handle_Pipe = popen ("gzip - > myfile.gz", "w");
   assert (App->Handle_Pipe != NULL);
   
   {
      wiringPiSetup ();
      piHiPri (99);
      int Pin = 0;
      int Edge = INT_EDGE_RISING;
      wiringPiISR (Pin, Edge, &Interrupt_Handle);
   }
   
   while (1)
   {
      App_Print_Time (App.Timestamp);
      sleep (1);
   }
}
*/


FILE * Opener ()
{
   char const Format [] = "gzip - > sample_%04d_%02d_%02d_%02d_%02d_%02d.gz\n";
   char Buffer [128];
   time_t Seconds = time (0);
   struct tm * Local = localtime (&Seconds);
   sprintf (Buffer, Format, 1900 + Local->tm_year, Local->tm_mon + 1, Local->tm_mday, Local->tm_hour, Local->tm_min, Local->tm_sec);
   printf (Buffer);
   FILE * Pipe;
   Pipe = popen (Buffer, "w");
   assert (Pipe != NULL);
   return Pipe;
}


void Delegate (FILE * File)
{
   struct Lepton_Pixel_Grayscale16 Pixmap [Lepton3_Width * Lepton3_Height];
   size_t const Size = sizeof (Pixmap);
   {
      int R = read (STDIN_FILENO, Pixmap, Size);
      assert (R == Size);
   }
   
   {
      int R = fwrite (Pixmap, Size, 1, File);
      assert (R == 1);
   }
}



int main (int argc, char * argv [])
{ 
   int Timer;
   Timer = timerfd_create (CLOCK_MONOTONIC, TFD_NONBLOCK);
   assert (Timer > 0);
   
   struct itimerspec Spec;
   Spec.it_interval.tv_sec = 10;
   Spec.it_interval.tv_nsec = 0;
   Spec.it_value.tv_sec = 10;
   Spec.it_value.tv_nsec = 0;
   
   {
      int R = timerfd_settime (Timer, 0, &Spec, NULL);
      assert (R == 0);
   }
   

   
   FILE * Pipe = NULL;
   
   Pipe = Opener ();
   
   while (1)
   {
      uint64_t N;
      int R = read (Timer, &N, sizeof (N));
      if (R == sizeof (N))
      { 
         //printf ("R:%i\n", R);
         //assert (R == sizeof (N));
         pclose (Pipe);
         Pipe = Opener ();
      }
      Delegate (Pipe);
      //printf (".");
   }
}


