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


void Process2
(
   uint16_t const * Source, 
   SDL_Texture * Texture, 
   size_t Width,
   size_t Height
)
{
   float M1 [Width * Height];
   struct Pixel_ABGR8888 M2 [Width * Height];
   uint16_t Min = UINT16_MAX;
   uint16_t Max = 0;
   Find_Range_u16v (Source, Width * Height, &Min, &Max);
   Map_Linear_u16v_float (Source, M1, Width * Height, Min, Max, 0, 255);
   Map_Pixel_float_ABGR8888 (M1, M2, Width * Height, Map_Pixel_ABGR8888_Heat256, 256);
   SDL_UpdateTexture (Texture, NULL, M2, Width * sizeof (struct Pixel_ABGR8888));
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
      SDL_WINDOW_FULLSCREEN |
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
      //usleep (10000);
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

