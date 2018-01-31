//gcc render.c -std=gnu11 -fdiagnostics-color -Wall -Wno-missing-braces `sdl2-config --cflags --libs` -lm -lSDL2_ttf -lSDL2_image -o render && ./grab | ./render

#include "util.h"
#include "pixel.h"

#include "../Lepton/Lepton.h"
#include "../Lepton/Lepton_Pixels.h"

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


#include <assert.h>
#include <float.h>


#include <SDL2/SDL.h>


float Map_Linear_float 
(
   float X, 
   float A0, 
   float A1, 
   float B0, 
   float B1
)
{
   //cropping
   if (X < A0) {return B0;}
   if (X > A1) {return B1;}
   //calculate delta
   float DA;
   float DB;
   DA = A1 - A0;
   DB = B1 - B0;
   //move to zero
   X = X - A0;
   //new scale
   X = X * DB;
   //zero division protection
   if (DA == 0) {return B1;};
   X = X / DA;
   //new offset
   X = X + B0;
   return X;
}

void Map_Linear_floatv
(
   float const * Source, 
   float * Destination, 
   size_t Count, 
   float A0, 
   float A1, 
   float B0, 
   float B1
)
{
   for 
   (size_t I = 0; I < Count; I = I + 1)
   {Destination [I] = Map_Linear_float (Source [I], A0, A1, B0, B1);}
}


void Find_Range_float (float Data, float * Min, float * Max)
{
   if (Data > *Max) {*Max = Data;}
   if (Data < *Min) {*Min = Data;}
}


void Find_Range_floatv 
(
   float const * Data, 
   size_t Count, 
   float * Min, 
   float * Max
)
{
   assert ((Count > 0 && Data != NULL) || (Count == 0));
   for 
   (size_t I = 0; I < Count; I = I + 1) 
   {Find_Range_float (Data [I], Min, Max);}
   assert (*Max >= *Min);
}


void Map_u16v_floatv 
(
   uint16_t const * Source, 
   float * Destination, 
   size_t Count
)
{
   for 
   (size_t I = 0; I < Count; I = I + 1)
   {Destination [I] = (float) Source [I];}
}


void Process
(
   uint16_t const * Source, 
   SDL_Texture * Texture, 
   size_t Width,
   size_t Height
)
{
   float M1 [Width * Height];
   struct Pixel_ABGR8888 M2 [Width * Height];
   float Min = FLT_MAX;
   float Max = FLT_MIN;
   Map_u16v_floatv (Source, M1, Width * Height);
   Find_Range_floatv (M1, Width * Height, &Min, &Max);
   Map_Linear_floatv (M1, M1, Width * Height, Min, Max, 0, 255);
   Map_Pixel_float_ABGR8888 (M1, M2, Width * Height, Map_Pixel_ABGR8888_Heat256, 256);
   SDL_UpdateTexture (Texture, NULL, M2, Width * sizeof (struct Pixel_ABGR8888));
}

void Reciever (uint16_t * Destination, size_t Count)
{
   size_t const Size = sizeof (uint16_t) * Count;
   int R = read (STDIN_FILENO, Destination, Size);
   Assert (R == (int) Size, "%s", "read failed");
}


void Init ()
{
   int Result;
   Result = SDL_Init (SDL_INIT_VIDEO);
   Assert (Result == 0, "%s", "SDL_Init failed");
}


int main (int argc, char * argv [])
{ 
   Assert (argc == 1, "argc %i.", argc);
   Assert (argv[0] != NULL, "argv0 %p.", argv[0]);
   
   SDL_Window * Window = NULL;
   SDL_Renderer * Renderer = NULL;
   SDL_Texture * Texture = NULL;
   SDL_Event Event;
   
   errno = 0;
   Window = SDL_CreateWindow 
   (
      "Pixmap Renderer", 
      SDL_WINDOWPOS_UNDEFINED, 
      SDL_WINDOWPOS_UNDEFINED, 
      640, 
      480, 
      SDL_WINDOW_OPENGL | 
      SDL_WINDOW_SHOWN | 
      //SDL_WINDOW_FULLSCREEN |
      0
   );
   Assert (Window != NULL, "SDL_CreateWindow failed. %s", SDL_GetError ());
   
   Renderer = SDL_CreateRenderer (Window, -1, SDL_RENDERER_ACCELERATED);
   Assert (Renderer != NULL, "SDL_CreateRenderer failed. %s", SDL_GetError ());
   
   size_t const Width = Lepton3_Width;
   size_t const Height = Lepton3_Height;
   uint16_t Pixmap [Width * Height];
   memset (Pixmap, 0, sizeof (Pixmap));
   Texture = SDL_CreateTexture (Renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, Width, Height);
   Assert (Texture != NULL, "SDL_CreateTexture failed. %s", SDL_GetError ());
   SDL_SetTextureBlendMode (Texture, SDL_BLENDMODE_BLEND);
   
   while (1)
   {
      usleep (10000);
      while (SDL_PollEvent (&Event))
      {
         switch (Event.type)
         {
            case SDL_QUIT:
            printf ("SDL_QUIT\n");
            goto Loop_Exit;
            break;
            
            case SDL_KEYDOWN:
            printf ("SDL_KEYDOWN\n");
            goto Loop_Exit;
            break;
            
            case SDL_MOUSEBUTTONDOWN:
            printf ("SDL_MOUSEBUTTONDOWN\n");
            Pixmap [0] = 0;
            Pixmap [1] = 255;
            
            break;
         }
      }
      
      Reciever (Pixmap, Width * Height);
      Process (Pixmap, Texture, Width, Height);
      
      SDL_RenderCopy (Renderer, Texture, NULL, NULL);
      SDL_RenderPresent (Renderer);
   }
   Loop_Exit:
   
   SDL_DestroyTexture (Texture);
   SDL_DestroyRenderer (Renderer);
   SDL_DestroyWindow (Window);
   
   return 0;
}

