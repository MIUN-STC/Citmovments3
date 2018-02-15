//gcc render.c -std=gnu11 -fdiagnostics-color -Wall -Wno-missing-braces `sdl2-config --cflags --libs` -lm -lSDL2_ttf -lSDL2_image -o render && ./grab | ./render

#include "util.h"
#include "pixel.h"
#include "map.h"

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
#include <stdlib.h>
#include <math.h>


#include <SDL2/SDL.h>


struct Point2_float
{
	float X;
	float Y;
};

struct Rectangle2_float
{
	float X;
	float Y;
	float W;
	float H;
};


void Random_Point2v_float
(
	size_t Count, 
	struct Point2_float Point [Count],
	size_t Width,
	size_t Height
)
{
	for (size_t I = 0; I < Count; I = I + 1)
	{
		Point [I].X = Random_float (0, Width-1);
		Point [I].Y = Random_float (0, Height-1);
	}
}


void Random_Point2_Delta_float
(
	struct Point2_float * Point,
	float Width,
	float Height,
	float Delta
)
{
	Point->X = Point->X + Random_float (-Delta, Delta);
	Point->Y = Point->Y + Random_float (-Delta, Delta);
	Point->X = Crop_float (Point->X, 0, Width-1);
	Point->Y = Crop_float (Point->Y, 0, Height-1);
}


void Process3
(
    uint16_t const * Source, 
    SDL_Texture * Texture, 
    size_t Width,
    size_t Height,
    size_t Count,
    struct Point2_float Pointv [Count]
)
{
	float M1 [Width * Height];
	struct Pixel_ABGR8888 M2 [Width * Height];
	uint16_t Min = UINT16_MAX;
	uint16_t Max = 0;
	Find_Range_u16v (Source, (Width * Height), &Min, &Max);
	Map_Linear_u16v_floatv ((Width * Height), Source, M1, Min, Max, 0.0f, 1.0f);
	Map_Pixel_float_ABGR8888 (M1, M2, (Width * Height), 0.0f, 1.0f, Map_Pixel_ABGR8888_Heat256, 256);

	for (size_t I = 0; I < Count; I = I + 1)
	{
		size_t Index1;
		size_t Index2;
		size_t Index;
		float V1;
		float V2;
		Index1 = rand () % Count;
		Index2 = rand () % Count;
		Index = Width * (int) Pointv [Index1].Y + (int) Pointv [Index1].X;
		V1 = M1 [Index];
		Index = Width * (int) Pointv [Index2].Y + (int) Pointv [Index2].X;
		V2 = M1 [Index];
		if (V1 > V2)
		{
			Pointv [Index2].Y = Pointv [Index1].Y;
			Pointv [Index2].X = Pointv [Index1].X;
		}
		else
		{
			Pointv [Index1].Y = Pointv [Index2].Y;
			Pointv [Index1].X = Pointv [Index2].X;
		}
	}

	for (size_t I = 0; I < Count; I = I + 1)
	{
		size_t Index = Width * (int) Pointv [I].Y + (int) Pointv [I].X;
		assert (Index < (Width * Height));
		Random_Point2_Delta_float (Pointv + I, Width, Height, (1.0f - M1 [I])*Random_float (1.0f, 20.0f));
	}
	
	for (size_t I = 0; I < Count; I = I + 1)
	{
		size_t Index = Width * (int) Pointv [I].Y + (int) Pointv [I].X;
		assert (Index < (Width * Height));
		M2 [Index] = (struct Pixel_ABGR8888){255, 0, 255, 0};
	}
	
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
        1920, 
        1080, 
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
    //int R = SDL_SetHint (SDL_HINT_RENDER_SCALE_QUALITY, "1");
    //assert (R == SDL_TRUE);
    Texture = SDL_CreateTexture (Renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, Width, Height);
    Assert (Texture != NULL, "SDL_CreateTexture failed. %s", SDL_GetError ());
    SDL_SetTextureBlendMode (Texture, SDL_BLENDMODE_BLEND);
    
	size_t const Particles_Count = 200;
	struct Point2_float Particles1 [Particles_Count];
	Random_Point2v_float (Particles_Count, Particles1, Width, Height);
    
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
        Process3 (Pixmap, Texture, Width, Height, Particles_Count, Particles1);
        
        SDL_RenderCopy (Renderer, Texture, NULL, NULL);
        SDL_RenderPresent (Renderer);
    }
    Loop_Exit:
    
    SDL_DestroyTexture (Texture);
    SDL_DestroyRenderer (Renderer);
    SDL_DestroyWindow (Window);
    
    return 0;
}


