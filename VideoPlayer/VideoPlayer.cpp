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

    static bool bDumpSamples = false;
    if (bDumpSamples)
    {
        uint32 maxSize = mp4File->GetTrackMaxSampleSize(1);
        uint32 sampleCount = mp4File->GetNumOfSamples(1);
        uint32 sampleSize = 0;
        uint8 * pBuffer = (uint8 *)osMalloc(maxSize + 8);
        osDump(0, "\n Dumping MP4 file Sample Data...  \n");
        for (uint32 i = 0; i < sampleCount; i++)
        {
            memset(pBuffer, 0, maxSize + 8);
            sampleSize = 0;
            sampleSize = mp4File->ReadSample(1, i + 1, pBuffer, maxSize + 1, NULL, NULL, NULL, NULL);
            osAssert(sampleSize && (sampleSize != (uint32)-1));
            osDump(0, "SampleID : %d \n", i + 1);
            for (uint32 j = 0; j < sampleSize; j = j + 8)
            {
                osDump(4, "%x  %x  %x  %x  %x  %x  %x  %x\n", pBuffer[j], pBuffer[j + 1], pBuffer[j + 2], pBuffer[j + 3],
                    pBuffer[j + 4], pBuffer[j + 5], pBuffer[j + 6], pBuffer[j + 7]);
            }
        }
    }

    char h264FileName[64] = { 0 };
    sprintf_s(h264FileName, sizeof(h264FileName), "%s.264", name);
    mp4File->Extract264RawData(h264FileName);

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