#include <stdlib.h>
#include <stdio.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include "VideoFileClass.h"
#include "MP4FileClass.h"
#include <os.h>

//Screen attributes
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

static bool g_bPause = false;


void playVideo(const char * file)
{

}

void dumpFileInfo(const char * name)
{
    MP4FileClass * mp4File = new MP4FileClass();
    mp4File->Open(name, File::MODE_READ, NULL);
    mp4File->ReadFromFile();
    mp4File->Dump(false);

    delete mp4File;
}

int main(int, char**)
{
    SDL_Surface *screen = NULL;
    SDL_Surface *image = NULL;

    dumpFileInfo("sample.mp4");
    return 0;

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        osLog(LOG_ERROR, "SDL_Init Error: %s. \n", SDL_GetError());
		return 1;
	}

    SDL_Window *window = SDL_CreateWindow("Video Player", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        osLog(LOG_ERROR, "CreateWindow");
        SDL_Quit();
        return 1;
    }

    screen = SDL_GetWindowSurface(window);
    image = SDL_LoadBMP("background.bmp"); // loads image

    //Our event structure
    SDL_Event e;
    const char * key = 0;
    //For tracking if we want to quit
    bool quit = false;
    while (!quit)
    {
        //Read any events that occured, for now we'll just quit if any event occurs
        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                key = SDL_GetKeyName(e.key.keysym.sym);
                printf("The %s key was pressed!\n", key);
                if (e.key.keysym.sym == SDLK_ESCAPE)                    //quit if ¡¯ESC¡¯
                {
                    quit = true;
                }
                else if (key[0] == 'q')                                 //quit if ¡¯q¡¯ pressed
                {
                    quit = true;
                }
                break;
                //If user clicks the mouse
            case SDL_MOUSEBUTTONDOWN:
                g_bPause = !g_bPause;
                break;
            default:
                break;
            }

        }
        //Rendering
        SDL_BlitSurface(image, NULL, screen, NULL);
        SDL_UpdateWindowSurface(window);
        SDL_Delay(30);
    }

    SDL_FreeSurface(image);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}