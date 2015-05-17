#include <stdlib.h>
#include <stdio.h>

#include <SDL.h>
#include <os.h>

int main(int, char**){
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        osLog(LOG_ERROR, "SDL_Init Error: %s. \n", SDL_GetError());
		return 1;
	}


    SDL_Quit();
    return 0;
}