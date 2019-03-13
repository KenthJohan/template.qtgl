#include <SDL2/SDL.h>
#include <stdio.h>

#define WIN_X SDL_WINDOWPOS_UNDEFINED
#define WIN_Y SDL_WINDOWPOS_UNDEFINED
#define WIN_W 640
#define WIN_H 480
#define WIN_TITLE "An SDL2 windows"

int main (int argc, char * argv[])
{
	SDL_Window * window1;
	SDL_Window * window2;
	SDL_Init (SDL_INIT_VIDEO);
	window1 = SDL_CreateWindow (WIN_TITLE, WIN_X, WIN_Y, WIN_W, WIN_H, SDL_WINDOW_OPENGL);
	window2 = SDL_CreateWindow (WIN_TITLE, WIN_X, WIN_Y, WIN_W, WIN_H, SDL_WINDOW_OPENGL);
	if (window1 == NULL || window2 == NULL)
	{
		printf("Could not create window: %s\n", SDL_GetError());
		return 1;
	}
	SDL_Delay(3000);
	SDL_DestroyWindow (window1);
	SDL_DestroyWindow (window2);
	SDL_Quit();
	return 0;
}
