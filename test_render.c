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


struct Application
{
	SDL_Window * Window;
	SDL_Renderer * Renderer;
	SDL_Texture * Texture;
	size_t Width;
	size_t Height;
	bool AG;
	uint16_t Pixmap [Lepton3_Width * Lepton3_Height];
	float Pixmap_float [Lepton3_Width * Lepton3_Height];
	struct Pixel_ABGR8888 Pixmap_ABGR8888 [Lepton3_Width * Lepton3_Height];
	SDL_Event Event;
	uint16_t Min;
	uint16_t Max;
};


void Application_Init (struct Application * App)
{
	App->Width = Lepton3_Width;
	App->Height = Lepton3_Height;
	App->AG = true;
	memset (App->Pixmap, 0, App->Width * App->Height * sizeof (uint16_t));
	App->Min = UINT16_MAX;
	App->Max = 0;
	
	Log ("SDL_Init %s", "");
    Assert (SDL_Init (SDL_INIT_VIDEO) == 0, "%s", "SDL_Init failed");
	
	Log ("SDL_CreateWindow %s", "");
	App->Window = SDL_CreateWindow 
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
	Assert (App->Window != NULL, "SDL_CreateWindow failed. %s", SDL_GetError ());
	
	App->Renderer = SDL_CreateRenderer (App->Window, -1, SDL_RENDERER_ACCELERATED);
	Assert (App->Renderer != NULL, "SDL_CreateRenderer failed. %s", SDL_GetError ());
	

	Assert (SDL_SetHint (SDL_HINT_RENDER_SCALE_QUALITY, "1") == SDL_TRUE, "SDL_Error %s", SDL_GetError ());
	App->Texture = SDL_CreateTexture (App->Renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, App->Width, App->Height);
	Assert (App->Texture != NULL, "SDL_CreateTexture failed. %s", SDL_GetError ());
	SDL_SetTextureBlendMode (App->Texture, SDL_BLENDMODE_BLEND);
}


void Application_Quit (struct Application * App)
{
	SDL_DestroyTexture (App->Texture);
	SDL_DestroyRenderer (App->Renderer);
	SDL_DestroyWindow (App->Window);
	SDL_Quit ();
}


void Application_Update (struct Application * App)
{
	//Recieve camera frames from standard in
	{
		size_t const Size = sizeof (uint16_t) * App->Width * App->Height;
		int R = read (STDIN_FILENO, App->Pixmap, Size);
		Assert (R == (int) Size, "%s", "read failed");
	}
	
	//If auto gain then update min max value.
	//min max is used later to map pixel values to diffrent scale. 
	if (App->AG == true)
	{
		App->Min = UINT16_MAX;
		App->Max = 0;
		Find_Range_u16v (App->Pixmap, (App->Width * App->Height), &(App->Min), &(App->Max));
	}
	
	//Map pixmap from: (min .. max) --> (0 .. 1)
    Map_Linear_u16v_floatv 
    (
		(App->Width * App->Height), 
		App->Pixmap, 
		App->Pixmap_float, 
		App->Min, 
		App->Max, 
		0.0f, 
		1.0f
	);
	
    //Map pixmap to a set of RGB values: (0 .. 1) --> (Cold .. Warm).
    Map_Pixel_float_ABGR8888 
    (
		App->Pixmap_float, 
		App->Pixmap_ABGR8888, 
		(App->Width * App->Height), 
		0.0, 
		1.0f, 
		Map_Pixel_ABGR8888_Heat256, 
		256
	);
	
	//Update GPU texture memory
    SDL_UpdateTexture 
    (
		App->Texture, 
		NULL, 
		App->Pixmap_ABGR8888, 
		App->Width * sizeof (struct Pixel_ABGR8888)
	);
	
	SDL_RenderCopy (App->Renderer, App->Texture, NULL, NULL);
	SDL_RenderPresent (App->Renderer);
}






int main (int argc, char * argv [])
{ 
	Assert (argc == 1, "argc %i.", argc);
	Assert (argv[0] != NULL, "argv0 %p.", argv[0]);

	struct Application App;
	Application_Init (&App);

	while (1)
	{
		//usleep (10000);
		while (SDL_PollEvent (&App.Event))
		{
			switch (App.Event.type)
			{
			case SDL_QUIT:
				printf ("SDL_QUIT\n");
				goto Loop_Exit;
				break;

			case SDL_KEYDOWN:
				printf ("SDL_KEYDOWN %i\n", App.Event.key.keysym.sym);
				if (App.Event.key.keysym.sym == SDLK_ESCAPE)
				{goto Loop_Exit;}
				if (App.Event.key.keysym.sym == SDLK_SPACE)
				{App.AG = !App.AG;}
				break;
				
			case SDL_MOUSEBUTTONDOWN:
				printf ("SDL_MOUSEBUTTONDOWN\n");
				App.Pixmap [0] = 0;
				App.Pixmap [1] = 255;
				break;
			}
		}
		Application_Update (&App);
	}
	Loop_Exit:

	Application_Quit (&App);
	
	return 0;
}

