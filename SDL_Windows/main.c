/*
gcc main.c -I/c/msys64/mingw64/include -L/c/msys64/mingw64/lib -lmingw32 -lSDL2main -lSDL2 -mwindows -Wl,--no-undefined -Wl,--dynamicbase -Wl,--nxcompat -Wl,--high-entropy-va -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lsetupapi -lversion -luuid -static-libgcc
*/

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "../demo1/csc/csc_debug.h"

#define WIN_X SDL_WINDOWPOS_UNDEFINED
#define WIN_Y SDL_WINDOWPOS_UNDEFINED
#define WIN_W 640
#define WIN_H 480
#define WIN_TITLE "An SDL2 windows"
#define WIN_N 10

#define APP_QUIT 0x01


struct Application
{
	uint32_t n;
	SDL_Window * window [WIN_N];
};

void XSDL_DestroyWindows (SDL_Window * window [], uint32_t n)
{
	for (uint32_t i = 0; i < n; i ++)
	{
		SDL_DestroyWindow (window [i]);
	}
}

#define XSDL_Init(x) ASSERTF(SDL_Init((x)) == 0, "There was an error initializing the SDL library: %s\n", SDL_GetError())
#define XSDL_WaitEvent(x) ASSERTF(SDL_WaitEvent((x)) == 1, "There was an error while waiting for events: %s\n", SDL_GetError())
#define XSDL_ASSERT_CreateWindow(x) ASSERTF((x) != NULL, "Could not create window: %s\n", SDL_GetError())

int main (int argc, char * argv[])
{
	setbuf (stdout, NULL);

	struct Application a = {0};
	a.n = 1;

	uint32_t flags = 0;
	XSDL_Init (SDL_INIT_VIDEO);


	a.window [0] = SDL_CreateWindow (WIN_TITLE, WIN_X, WIN_Y, WIN_W, WIN_H, SDL_WINDOW_OPENGL);
	XSDL_ASSERT_CreateWindow (a.window [0]);

	SDL_Event event;
	while (1)
	{
		if (flags & APP_QUIT) {break;}
		//printf ("SDL_WaitEvent: %i\n", SDL_WaitEvent (&event));
		XSDL_WaitEvent (&event);
		SDL_Delay(100);
		switch (event.type)
		{
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				flags |= APP_QUIT;
				break;

			case SDLK_c:
				printf ("SDL_CreateWindow\n");
				if (a.n < WIN_N)
				{
					a.window [a.n] = SDL_CreateWindow (WIN_TITLE, WIN_X, WIN_Y, WIN_W, WIN_H, SDL_WINDOW_OPENGL);
					a.n ++;
				}
				break;

			default:
				break;
			}
			break;

		case SDL_QUIT:
			flags |= APP_QUIT;
			break;

		case SDL_MOUSEMOTION:
			//printf ("%i %i\n", event.motion.x, event.motion.y);
			break;
		}
	}

	XSDL_DestroyWindows (a.window, a.n);
	SDL_Quit();

	return 0;
}
