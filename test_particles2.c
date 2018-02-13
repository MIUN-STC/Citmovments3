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




struct Particle
{
	float Position [2];
	float Weight;
	float Energy;
};


struct Atom
{
	struct Particle Proton;
	size_t Cloud_Size;
	struct Particle Cloud [10];
};


void Atoms_Init
(
    size_t Width,
    size_t Height,
	size_t Atoms_Count,
	struct Atom Atoms [Atoms_Count]
)
{
	size_t const Dim = 2;
	float Min [2] = {0, 0};
	float Max [2] = {Width - 1, Height - 1};
	for (size_t I = 0; I < Atoms_Count; I = I + 1)
	{
		Atoms [I].Cload_Size = 10;
		Random_Rectangle_floatv (Dim, Atoms [I].Proton.Position, Min, Max);
		for (size_t J = 0; J < Atoms_Count; J = J + 1)
		{
			Atoms [I].Cloud [J].Energy = (float) J;
		}
	}
}



void Atoms_Calc
(
	size_t Atoms_Count,
	struct Atom Atoms [Atoms_Count]
)
{
	size_t const Dim = 2;
	for (size_t I = 0; I < Atoms_Count; I = I + 1)
	{
		for (size_t J = 0; J < Atoms_Count; J = J + 1)
		{
			Random_Delta_Square_floatv (Dim, Atoms [I].Proton.Position, Atoms [I].Proton.Energy);
		}
	}
}


void Draw
(
    uint16_t const * Source, 
    SDL_Texture * Texture, 
    size_t Width,
    size_t Height,
    size_t Atoms_Count,
    struct Atom Atoms [Atoms_Count]
)
{
	size_t const Dim = 2;
	float Min_Rect [2] = {0, 0};
	float Max_Rect [2] = {Width - 1, Height - 1};
	uint16_t M1 [Width * Height];
	struct Pixel_ABGR8888 M2 [Width * Height];
	uint16_t Min = UINT16_MAX;
	uint16_t Max = 0;
	Find_Range_u16v (Source, (Width * Height), &Min, &Max);
	Map_Linear_u16v_u16v ((Width * Height), Source, M1, Min, Max, 0, 255);
	
	Copy_u16_ABGR8888 ((Width * Height), M1, M2);
	
	for (size_t I = 0; I < Atoms_Count; I = I + 1)
	{
		float P [Dim];
		Crop_Rectangle_floatv (Dim, Atoms [I].Proton.Position, P, Min_Rect, Max_Rect);
		size_t Index = Width * (int) P [1] + (int) P [0];
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


int main (int argc, char * argv [argc])
{ 
    Assert (argc == 1, "argc %i", argc);
    Assert (argv != NULL, "argv %p", argv);
    
    Init ();
    
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
    
	size_t const Atoms_Count = 10;
	struct Atom Atoms [Atoms_Count];
	
	Atoms_Init (Width, Height, Atoms_Count, Atoms);
    
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
        Atoms_Calc (Atoms_Count, Atoms);
        Draw (Pixmap, Texture, Width, Height, Atoms_Count, Atoms);
        
        SDL_RenderCopy (Renderer, Texture, NULL, NULL);
        SDL_RenderPresent (Renderer);
    }
    Loop_Exit:
    
    SDL_DestroyTexture (Texture);
    SDL_DestroyRenderer (Renderer);
    SDL_DestroyWindow (Window);
    
    return 0;
}


