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
#include <stdbool.h>


#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>


void Process
(
    uint16_t const * Source, 
    SDL_Texture * Texture, 
    size_t Width,
    size_t Height,
    uint16_t Min,
    uint16_t Max
)
{
    float Pixmap_float [Width * Height];
    struct Pixel_ABGR8888 Pixmap_ABGR8888 [Width * Height];
    

    Map_Linear_u16v_floatv ((Width * Height), Source, Pixmap_float, Min, Max, 0.0f, 1.0f);
    
    //Convert the scale 0 .. 1 to RGB cold .. warm color scale.
    Map_Pixel_float_ABGR8888 (Pixmap_float, Pixmap_ABGR8888, (Width * Height), 0.0, 1.0f, Map_Pixel_ABGR8888_Heat256, 256);
    SDL_UpdateTexture (Texture, NULL, Pixmap_ABGR8888, Width * sizeof (struct Pixel_ABGR8888));
}


void Process2
(
	SDL_Renderer * Renderer, 
    TTF_Font * Font, 
    uint16_t const * Source, 
    SDL_Texture * Texture, 
    size_t Width,
    size_t Height,
    uint16_t Min,
    uint16_t Max
)
{
	assert (Font != NULL);
    float Pixmap_float [Width * Height];
    struct Pixel_ABGR8888 Pixmap_ABGR8888 [Width * Height];
    Map_Linear_u16v_floatv ((Width * Height), Source, Pixmap_float, Min, Max, 0.0f, 1.0f);
    //Convert the scale 0 .. 1 to RGB cold .. warm color scale.
    Map_Pixel_float_ABGR8888 (Pixmap_float, Pixmap_ABGR8888, (Width * Height), 0.0, 1.0f, Map_Pixel_ABGR8888_Heat256, 256);
    
    size_t Max_Index [2];
    uint16_t Max_Value = 0;
    Find_Range_Index2_u16v (Source, Width, Height, NULL, &Max_Value, NULL, Max_Index);
    
    Pixmap_ABGR8888 [Max_Index [1] * Width + Max_Index [0]] = (struct Pixel_ABGR8888){255, 0, 200, 0};
    
    SDL_UpdateTexture (Texture, NULL, Pixmap_ABGR8888, Width * sizeof (struct Pixel_ABGR8888));
    
    char Buf [10] = {0};
    snprintf (Buf, 6, "05%i", Source [Max_Index [1] * Width + Max_Index [0]]);
    SDL_Surface * Surface = TTF_RenderText_Solid (Font, Buf, (SDL_Color) {000, 255, 000, 255});
	SDL_Texture * Texture_Font = SDL_CreateTextureFromSurface (Renderer, Surface);
	SDL_FreeSurface (Surface);
	SDL_RenderCopy (Renderer, Texture, NULL, NULL);
	SDL_RenderCopy (Renderer, Texture_Font, NULL, (SDL_Rect []) {Max_Index [0] * (1920/Width), Max_Index [1] * (1080/Height), 200, 100});
	SDL_DestroyTexture (Texture_Font);
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
    TTF_Init ();
}


int main (int argc, char * argv [])
{ 
	Assert (argc == 1, "argc %i.", argc);
	Assert (argv[0] != NULL, "argv0 %p.", argv[0]);
	
	Init ();

	SDL_Window * Window = NULL;
	SDL_Renderer * Renderer = NULL;
	SDL_Texture * Texture = NULL;
	SDL_Event Event;
	TTF_Font * Font = TTF_OpenFont ("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 24);
	assert (Font != NULL);
	
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
	uint16_t Min = UINT16_MAX;
	uint16_t Max = 0;
	bool Auto_Gain = true;
	bool Freeze = false;
					
	//int R = SDL_SetHint (SDL_HINT_RENDER_SCALE_QUALITY, "1");
	//assert (R == SDL_TRUE);
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
				printf ("SDL_KEYDOWN %i\n", Event.key.keysym.sym);
				if (Event.key.keysym.sym == SDLK_ESCAPE)
				{goto Loop_Exit;}
				if (Event.key.keysym.sym == SDLK_c)
				{Auto_Gain = !Auto_Gain;}
				if (Event.key.keysym.sym == SDLK_SPACE)
				{Freeze = !Freeze;}
				if (Event.key.keysym.sym == SDLK_f)
				{
					//Process2 (Pixmap, Texture, Width, Height, Min, Max);
				}
				break;
				
			case SDL_MOUSEBUTTONDOWN:
				printf ("SDL_MOUSEBUTTONDOWN\n");
				Pixmap [0] = 0;
				Pixmap [1] = 255;
				break;
			}
		}

		if (Auto_Gain == true)
		{
			//Normalize the entire pixmap to 0 .. 1.
			Min = UINT16_MAX;
			Max = 0;
			Find_Range_u16v (Pixmap, (Width * Height), &Min, &Max);
		}

		Reciever (Pixmap, Width * Height);

		if (!Freeze)
		{
			Process2 (Renderer, Font, Pixmap, Texture, Width, Height, Min, Max);
			SDL_RenderPresent (Renderer);
		}

		
	}
	Loop_Exit:

	SDL_DestroyTexture (Texture);
	SDL_DestroyRenderer (Renderer);
	SDL_DestroyWindow (Window);

	return 0;
}

