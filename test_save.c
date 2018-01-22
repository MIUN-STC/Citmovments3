//gcc test_save.c -std=gnu11 -fdiagnostics-color -Wall -Wno-missing-braces -o save

#include "../Lepton/Lepton.h"
#include "../Lepton/Lepton_Pixels.h"

//printf
#include <stdio.h>

//uint64_t
#include <stdint.h>

//timerfd_create
#include <sys/timerfd.h>

//read
#include <unistd.h>

//strerror
#include <string.h>

//errrno
#include <errno.h>

//assert
#include <assert.h>

#include <stdlib.h>


//Creates a file.
//The filename will be a timestamp.
//The data being written to the file is compressed by gzip.
//Prints the command executed by popen.
//Returns the newly created file object.
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


//Read an entire Pixmap from STDIN and write that data to the argument <File>.
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


//Converts string to long.
//Checks for error and prints them.
//Prints successful value.
//Returns converted value long range 1 .. LONG_MAX.
long User_Input1 (char * Input)
{
   char * End;
   long Value;
   errno = 0;
   Value = strtol (Input, &End, 10);
   if (errno != 0)
   {
      printf ("Conversion error, non-convertable part: %s\n", End);
   };
   assert (errno == 0);
   if (Value <= 0)
   {
      printf ("Invalid period value. Minimum 1 seconds allowed.\n");
   }
   assert (Value >= 1);
   printf ("Period: %i seconds\n", (int) Value);
   return Value;
}


int main (int argc, char * argv [])
{ 
   assert (argc == 2);
   
   int Timer;
   Timer = timerfd_create (CLOCK_MONOTONIC, TFD_NONBLOCK);
   assert (Timer > 0);
   
   struct itimerspec Spec;
   Spec.it_interval.tv_sec = User_Input1 (argv [1]);
   Spec.it_interval.tv_nsec = 0;
   Spec.it_value.tv_sec = Spec.it_interval.tv_sec;
   Spec.it_value.tv_nsec = Spec.it_interval.tv_nsec;
   
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


