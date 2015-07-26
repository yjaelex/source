#ifndef _OS_CURSES_H
#define _OS_CURSES_H

#include "os.h"
#include <curses.h>

typedef pvoid osCLIHandle;

typedef enum CLIArea
{
    CLI_UP_WIN,
    CLI_DOWN_WIN,
    CLI_SRC
}CLIArea;

typedef bool(*pfnCLIFuncCB) (int argc, char** argv);

typedef struct osCLIFuncCB
{
    string              cmdName;
    string              helpInfo;
    pfnCLIFuncCB        pfnFunc;
}osCLIFuncCB;

osCLIHandle osCreateCmdLineInterface(uint32 width, uint32 height, bool bSplit);

bool osMainLoopCmdLineInterface(osCLIHandle cliHandle, int argc, char** argv);

void osSetCallBackFuncCmdLineInterface(osCLIHandle cliHandle, uint32 num, osCLIFuncCB * pCB);

pvoid osGetCmdWinCmdLineInterface(osCLIHandle cliHandle);
pvoid osGetInfoWinCmdLineInterface(osCLIHandle cliHandle);

void osPrintCmdLineInterface(osCLIHandle cliHandle, pvoid win, const char * str);

void osDesrotyCmdLineInterface(osCLIHandle cliHandle);


#endif


