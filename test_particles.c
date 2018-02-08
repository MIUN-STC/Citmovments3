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

void Point2_float_Random (struct Point2_float * P, size_t Count, struct Rectangle2_float * Area)
{
	for (size_t I = 0; I < Count; I = I + 1)
	{
		P [I].X = (rand () % (int)Area->W) + Area->X;
		P [I].Y = (rand () % (int)Area->H) + Area->Y;
	}
}

void Redistribution 
(
	float * M1,
	size_t Width,
	size_t Height,
	struct Point2_float const * P1, 
	struct Point2_float * P2, 
	size_t Count
)
{
	size_t N = 0;
	for (size_t I = 0; I < Count; I = I + 1)
	{
		size_t Index = Width * (int)P1 [I].Y + (int)P1 [I].X;
		assert (Index < (Width * Height));
		float A = M1 [Index] * Random_Float (0.0f, 1.0f) * (float) Count;
		for (size_t J = 0; J < (size_t) A; J = J + 1)
		{
			if (N >= Count) {return;}
			float DX = M1 [Index] * Random_Float (-0.2f, 0.2f) * Width;
			float DY = M1 [Index] * Random_Float (-0.2f, 0.2f) * Height;
			P2 [N].X += DX;
			P2 [N].Y += DY;
			P2 [N].X = Crop_float (P2 [I].X, 0, Width - 1);
			P2 [N].Y = Crop_float (P2 [I].Y, 0, Height - 1);
			N = N + 1;
		}
	}
}


void Process3
(
    uint16_t const * Source, 
    SDL_Texture * Texture, 
    size_t Width,
    size_t Height,
    struct Point2_float * P1,
    struct Point2_float * P2,
    size_t Count
)
{
	float M1 [Width * Height];
	struct Pixel_ABGR8888 M2 [Width * Height];
	uint16_t Min = UINT16_MAX;
	uint16_t Max = 0;
	Find_Range_u16v (Source, (Width * Height), &Min, &Max);
	Map_Linear_u16v_float (Source, M1, (Width * Height), Min, Max, 0.0f, 1.0f);
	Map_Pixel_float_ABGR8888 (M1, M2, (Width * Height), 0.0f, 1.0f, Map_Pixel_ABGR8888_Heat256, 256);
	
	Redistribution (M1, Width, Height, P1, P2, Count);

	for (size_t I = 0; I < Count; I = I + 1)
	{
		size_t Index = Width * (int)P1 [I].Y + (int)P1 [I].X;
		assert (Index < (Width * Height));
		M2 [Index] = (struct Pixel_ABGR8888){255, 255, 255, 0};
	}

	for (size_t I = 0; I < Count; I = I + 1)
	{
		size_t Index = Width * (int)P2 [I].Y + (int)P2 [I].X;
		assert (Index < (Width * Height));
		M2 [Index] = (struct Pixel_ABGR8888){255, 0, 255, 0};
	}
	
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
    uint16_t Min = UINT16_MAX;
    uint16_t Max = 0;
    Find_Range_u16v (Source, (Width * Height), &Min, &Max);
    Map_Linear_u16v_float (Source, M1, (Width * Height), Min, Max, 0.0f, 1.0f);
    Map_Pixel_float_ABGR8888 (M1, M2, (Width * Height), 0.0f, 1.0f, Map_Pixel_ABGR8888_Heat256, 256);
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
    
	size_t const Particles_Count = 30;
	struct Point2_float Particles1 [Particles_Count];
	struct Point2_float Particles2 [Particles_Count];
	struct Rectangle2_float Rect = {0.0f, 0.0f, (float) Width, (float) Height};
	Point2_float_Random (Particles1, Particles_Count, &Rect);
	
    
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
        //Process (Pixmap, Texture, Width, Height);
        
        Process3 (Pixmap, Texture, Width, Height, Particles1, Particles2, Particles_Count);
        
        SDL_RenderCopy (Renderer, Texture, NULL, NULL);
        SDL_RenderPresent (Renderer);
    }
    Loop_Exit:
    
    SDL_DestroyTexture (Texture);
    SDL_DestroyRenderer (Renderer);
    SDL_DestroyWindow (Window);
    
    return 0;
}


