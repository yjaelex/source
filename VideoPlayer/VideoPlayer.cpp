#include <stdlib.h>
#include <stdio.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include "VideoFileClass.h"
#include "MP4FileClass.h"
#include "TransformVideoFile.h"
#include <os.h>

#include <curl/curl.h>

#include <osCurses.h>

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
    { OPT_MULTI3,("--urls"), OPT_TYPE_MULTI },
    { OPT_STOP, ("--"), OPT_TYPE_NONE }
};

static void ShowUsage()
{
    osPrintf(
        "Usage: VideoPlayer [OPTIONS] [URLs]\n"
        "\n"
        "-o format          Set output video file format. (raw ts) \n"
        "--url ARG          URL of remote input video file\n"
        "--multiURL N ARG-1 ARG-2 ... ARG-N   Multiple urls.\n"
        "           N:          number of urls.\n"
        "           ARG-(1~N) : urls list\n"
        "--urls urlFormatStr beginNum  numberOfurls   a formatted url string, just like printf.\n"
        "\n"
        "-?  -h  -help  --help                        Output this help.\n"
        "\n"
        );
}

static vector<string> urlStrings;
static uint32 outPutForamt = 0;     // 0 : raw; 1 : ts

bool DoMultiArgsCB(int Id, const char * pOptText, uint32 nArgIndex, char * pArgsText)
{
    osAssert(pOptText && pArgsText);

    if (Id == OPT_MULTI3)
    {
        static string tempStr;
        static int beginNum = 0;
        char buffer[256] = { 0 };
        if (nArgIndex == 0)
        {
            urlStrings.clear();
            beginNum = 0;
            // formatted url string "http://.../.../.../...%d.mp4"
            string s;
            s.assign(pArgsText);
            std::size_t found = s.find_last_of("%");
            tempStr = s.substr(0, found + 1);
            tempStr.append("03");
            tempStr.append((s.substr(found + 1)).c_str());
        }
        else if (1 == nArgIndex)
        {
            beginNum = atoi(pArgsText);
        }
        else if (2 == nArgIndex)
        {
            int num = atoi(pArgsText);
            for (int i = beginNum; i < beginNum + num; i++)
            {
                buffer[0] = 0;
                sprintf_s(buffer, sizeof(buffer), tempStr.c_str(), i);
                urlStrings.push_back(string(buffer));
            }
        }
    }
    else if (Id == OPT_MULTI)
    {
        if (nArgIndex == 0)
        {
            urlStrings.clear();
        }
        urlStrings.push_back(string(pArgsText));
    }
    else
    {
        return false;
    }

    return true;
}

bool DoArgsCB(int Id, const char * pOptText, char * pArgsText)
{
    switch (Id)
    {
    case 1:
        // Set output video file format. (raw ts)
        osAssert(pArgsText);
        outPutForamt = (uint32)atoi(pArgsText);
        break;
    case 2:
        // Single URL of remote input video file.
        osAssert(pArgsText);
        urlStrings.clear();
        urlStrings.push_back(string(pArgsText));
        break;
    case 3:
        // Video Cmd to use. transform only currently.
        break;
    default:
        break;
    }

    return true;
}

#if 0
int curses_row, curses_col;				/* to store the number of rows and *
                                         * the number of colums of the screen */
char * strBuf = NULL;

void initCurses()
{
    char mesg[] = "***  Video Library Demo Interface  ***";
    WINDOW* _window = initscr();				/* start the curses mode */
    getmaxyx(stdscr, curses_row, curses_col);   /* get the number of rows and columns */

    /* Resize the terminal to something larger than the physical screen */
    resize_term(2000, 2000);
    /* Get the largest physical screen dimensions */
    getmaxyx(_window, curses_row, curses_col);

    curses_row = curses_row * 3 / 4;
    curses_col = curses_col * 3 / 4;

    /* Resize so it fits */
    resize_term(curses_row - 1, curses_col - 1);
    /* Get the screen dimensions that fit */
    getmaxyx(_window, curses_row, curses_col);

    if (has_colors() == TRUE)
    {
        start_color();			                    /* Start color 			*/
        init_pair(1, COLOR_RED, COLOR_BLACK);
        attron(COLOR_PAIR(1));
    }

    mvprintw((curses_row / 2), (curses_col - strlen(mesg)) / 2, "%s", mesg);
    /* print the message at the center of the screen */
    strBuf = (char*)osMalloc(2*curses_col);
    memset(strBuf, '_', curses_col);
    strBuf[curses_col] = 0;
    mvprintw((curses_row / 2) - 1, 0, "%s", strBuf);
    refresh();
}

void showHelpInfo()
{
    printw("\n   List of commands:\n");
    printw("   help   --   print this help info.\n");
    printw("   trans  --   Tranform video files.\n");
    printw("   abort  --   Abort current jobs.\n");
    printw("   quit   --   Quit this program.\n");
}

int doCurses(int argc, char** argv)
{
    strBuf[0] = 0;

    while (1)
    {
        getstr(strBuf);

        if (strncmp(strBuf, "quit", 8) == 0)
        {
            break;
        }
        else if (strncmp(strBuf, "help", 8) == 0)
        {
            showHelpInfo();
        }
        else if (strncmp(strBuf, "abort", 8) == 0)
        {
            //abortJobs();
        }
        else if (strncmp(strBuf, "abort", 8) == 0)
        {
            //abortJobs();
        }
        else
        {
            printw("Invalid command!");
            showHelpInfo();
        }

        refresh();
    }

    return 0;
}

void exitCurses()
{
    if (strBuf)     osFree(strBuf);
    endwin();			            /* End curses mode		  */
    curses_row = curses_col = 0;
}
#endif

int main(int argc, char** argv)
{
    //SDL_Surface *screen = NULL;
    //SDL_Surface *image = NULL;

    osCLIHandle hcli = osCreateCmdLineInterface(800, 600, true);
    osMainLoopCmdLineInterface(hcli, argc, argv);
    osDesrotyCmdLineInterface(hcli);
    return 0;

    int optID = 0;
    bool bError = false;
    pvoid hCmdOpt = osCreateCmdLineOptHandler(argc, argv, g_rgOptions, sizeof(g_rgOptions) / sizeof(CmdOpt));
    while (osGetOpt(hCmdOpt, &optID, &bError))
    {
        if (bError)
        {
            continue;
        }

        switch (optID)
        {
        case OPT_HELP:
            ShowUsage();
            return 0;
        case OPT_MULTI:
            osDoMultiArgs(hCmdOpt, -1, DoMultiArgsCB);
            break;
        case OPT_MULTI3:
            osDoMultiArgs(hCmdOpt, 3, DoMultiArgsCB);
            break;
        case OPT_STOP:
            osOptArgsStop(hCmdOpt);
        default:
            osDoArgs(hCmdOpt, DoArgsCB);
            break;
        }
    }

    VideoLibraryJobMgr jobMgr;
    for (uint32 i = 0; i < urlStrings.size(); i++)
    {
        char * pStr = const_cast<char*>(urlStrings[i].c_str());
        jobMgr.PushJobRequest(VP_JOB_TRANSFORM_FILES, 1, &(pStr));
        osThreadSuspend(1000);
    }
    jobMgr.WaitAll();
    return 0;

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
