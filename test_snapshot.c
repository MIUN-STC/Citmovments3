//gcc test_saveimg.c -std=gnu11 -fdiagnostics-color -Wall -Wno-missing-braces `sdl2-config --cflags --libs` -lm -lSDL2_ttf -lSDL2_image

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


void Process
(
    uint16_t const * Source, 
    struct Pixel_ABGR8888 * Pixmap_ABGR8888,
    size_t Width,
    size_t Height,
    uint16_t Min,
    uint16_t Max
)
{
    float Pixmap_float [Width * Height];
    

    Map_Linear_u16v_floatv ((Width * Height), Source, Pixmap_float, Min, Max, 0.0f, 1.0f);
    
    for (size_t I = 0; I < 255; I = I + 1)
    {
		Pixmap_float [I] = ((float)I / 255.0f);
	}
    //Convert the scale 0 .. 1 to RGB cold .. warm color scale.
    Map_Pixel_float_ABGR8888 (Pixmap_float, Pixmap_ABGR8888, (Width * Height), 0.0, 1.0f, Map_Pixel_ABGR8888_Heat256, 256);
    
}


void Reciever (uint16_t * Destination, size_t Count)
{
    size_t const Size = sizeof (uint16_t) * Count;
    int R = read (STDIN_FILENO, Destination, Size);
    Assert (R == (int) Size, "%s", "read failed");
}


uint16_t Mean
(
	size_t Count,
	uint16_t const Source [Count]
)
{
	uint64_t R = 0;
	for (size_t I = 0; I < Count; I = I + 1)
	{
		R += Source [I];
	}
	R = R / Count;
	return R;
}


uint16_t SD
(
	size_t Count,
	uint16_t const Source [Count]
)
{
	uint16_t M = Mean (Count, Source);
	uint64_t R = 0;
	for (size_t I = 0; I < Count; I = I + 1)
	{
		uint16_t D = Source [I] - M;
		R += (D * D);
	}
	R = R / (Count - 1);
	return R;
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

	//Init ();

	size_t const Width = Lepton3_Width;
	size_t const Height = Lepton3_Height;
	uint16_t Pixmap [Width * Height];
    struct Pixel_ABGR8888 Pixmap_ABGR8888 [Width * Height];
	memset (Pixmap, 0, sizeof (Pixmap));
	uint16_t Min = UINT16_MAX;
	uint16_t Max = 0;



	for (size_t I = 0; I < 10; I = I + 1)
	{
		Reciever (Pixmap, Width * Height);
		printf ("SD: %d\n", SD (Width * Height, Pixmap));
	}
	
	//Normalize the entire pixmap to 0 .. 1.
	Min = UINT16_MAX;
	Max = 0;
	Find_Range_u16v (Pixmap, (Width * Height), &Min, &Max);
	Process (Pixmap, Pixmap_ABGR8888, Width, Height, Min, Max);

	SDL_Surface * Surface = SDL_CreateRGBSurfaceFrom 
	(
		Pixmap_ABGR8888, 
		Width, 
		Height, 
		32, 
		Width * sizeof (struct Pixel_ABGR8888), 
		0xFF000000,
		0x00FF0000,
		0x0000FF00,
		0x000000FF
	);
	SDL_SaveBMP (Surface, "snapshot.bmp");
	Assert (Surface != NULL, "Surface error. %s", "");
	SDL_FreeSurface (Surface);

	SDL_Quit ();

	return 0;
}


