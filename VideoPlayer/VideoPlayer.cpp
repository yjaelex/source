#include <stdlib.h>
#include <stdio.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include "VideoFileClass.h"
#include "MP4FileClass.h"
#include <os.h>

#include <curl/curl.h>

//Screen attributes
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

static bool g_bPause = false;


void playVideo(const char * file)
{

}

size_t my_write_func(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return fwrite(ptr, size, nmemb, stream);
}

void testCurl()
{
    const char * urlStr = "http://vod.cntv.lxdns.com/flash/live_back/nettv_cctv5/cctv5-2015-06-17-14-007.mp4";
    std::string str = string(urlStr);
    std::size_t found = str.find_last_of("/\\");
    std::string fileName = str.substr(found + 1);
    FILE *fp = NULL;
    fopen_s(&fp, fileName.c_str(), "ab+");

    CURLcode retCode = curl_global_init(CURL_GLOBAL_ALL);
    osAssert(retCode == CURLE_OK);
    CURL * curl = curl_easy_init();
    retCode = curl_easy_setopt(curl, CURLOPT_URL, urlStr);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    osAssert(retCode == CURLE_OK);
    retCode = curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    retCode = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    char errbuf[CURL_ERROR_SIZE];
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
    errbuf[0] = 0;

    retCode = curl_easy_perform(curl);

    fclose(fp);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

void dumpFileInfo(const char * name, bool isURL)
{
    FileProvider * provider = NULL;
    if (isURL)
    {
        std::string str = string(name);
        URLFile * urlFile = new URLFile(str, FileProvider::MODE_READ);
        provider = (FileProvider*)urlFile;
    }

    MP4FileClass * mp4File = new MP4FileClass();
    mp4File->Open(name, File::MODE_READ, provider);
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
    std::string fileName;
    if (isURL)
    {
        std::string str = string(name);
        std::size_t found = str.find_last_of("/\\");
        fileName = str.substr(found + 1);
    }
    else
    {
        fileName.assign(name);
    }
    sprintf_s(h264FileName, sizeof(h264FileName), "%s.264", fileName.c_str());
    mp4File->Extract264RawData(h264FileName);

    delete mp4File;
}

void testURL()
{
    const char * urlStr = "http://vod.cntv.lxdns.com/flash/live_back/nettv_cctv5/cctv5-2015-06-17-14-007.mp4";
    std::string str = string(urlStr);
    URLFile * urlFile = new URLFile(str, FileProvider::MODE_READ);

    FileProvider::Size nin = 0;
    uint8* buffer = (uint8*)osMalloc(1024 * 1024);
    urlFile->open();
    urlFile->read(buffer, 1024, nin);
    urlFile->close();
    delete urlFile;
}

bool transformVideoFile(vector<string> & urlStrVec)
{
    FileProvider * provider = NULL;
    for (uint32 i = 0; i < urlStrVec.size(); i++)
    {
        uint64 startTime, endTime = 0;
        URLFile * urlFile = new URLFile(urlStrVec[i], FileProvider::MODE_READ);
        urlFile->setAsync(true);
        provider = (FileProvider*)urlFile;

        osDump(0, "Transform Video File start!\n");
        osDump(4, "URL: %s \n", urlStrVec[i].c_str());
        startTime = osQueryNanosecondTimer();
        osDump(4, "START TIME: %lld ns \n", startTime);
        MP4FileClass * mp4File = new MP4FileClass();
        mp4File->Open(urlStrVec[i].c_str(), File::MODE_READ, provider);
        mp4File->ReadFromFile();

        std::string fileName;
        std::size_t found = urlStrVec[i].find_last_of("/\\");
        fileName = urlStrVec[i].substr(found + 1);
        char h264FileName[64] = { 0 };
        sprintf_s(h264FileName, sizeof(h264FileName), "%s.264", fileName.c_str());
        mp4File->Extract264RawData(h264FileName);
        mp4File->Close();

        endTime = osQueryNanosecondTimer();
        osDump(4, "END TIME  : %lld ns. Total: %f s. \n", endTime, ((float)(endTime-startTime)) / 1000000000);

        delete mp4File;
    }

    return true;
}

CmdOpt g_rgOptions[] =
{
    { OPT_HELP, ("-?"), OPT_TYPE_NONE },
    { OPT_HELP, ("-h"), OPT_TYPE_NONE },
    { OPT_HELP, ("-help"), OPT_TYPE_NONE },
    { OPT_HELP, ("--help"), OPT_TYPE_NONE },
    { 1, ("-o"), OPT_TYPE_REQ_SEP },
    { 2, ("--url"), OPT_TYPE_REQ_SEP },
    { 3, ("trans"), OPT_TYPE_NONE },
    { OPT_MULTI, ("--multiURL"), OPT_TYPE_MULTI },
    { OPT_STOP, ("--"), OPT_TYPE_NONE }
};

static void ShowUsage()
{
    osPrintf(
        "Usage: VideoPlayer [OPTIONS] [URLs]\n"
        "\n"
        "-o format          Set output video file format. (raw ts) \n"
        "--url ARG          URL of remote input video file\n"
        "--multi N ARG-1 ARG-2 ... ARG-N   Multiple urls.\n"
        "\n"
        "-?  -h  -help  --help                       Output this help.\n"
        "\n"
        );
}

int main(int argc, char** argv)
{
    SDL_Surface *screen = NULL;
    SDL_Surface *image = NULL;

    pvoid hCmdOpt = osCreateCmdLineOptHandler(argc, argv, g_rgOptions, sizeof(g_rgOptions) / sizeof(CmdOpt));

    //dumpFileInfo("sample.mp4");
    //testCurl();
    //testURL();
    const char * urlStr = "http://vod.cntv.lxdns.com/flash/live_back/nettv_cctv5/cctv5-2015-06-17-14-007.mp4";
    string url;
    url.assign(urlStr);
    vector<string> vecStr;
    vecStr.clear();
    vecStr.push_back(url);

    transformVideoFile(vecStr);
    return 0;

#if 0
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
#endif

    return 0;
}
