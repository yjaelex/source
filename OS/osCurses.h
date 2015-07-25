#ifndef _OS_CURSES_H
#define _OS_CURSES_H

#include "os.h"
#include <curses.h>

typedef pvoid osCLIHandle;


osCLIHandle osCreateCmdLineInterface(uint32 width, uint32 height, bool bSplit);

bool osMainLoopCmdLineInterface(osCLIHandle cliHandle, int argc, char** argv);

void osDesrotyCmdLineInterface(osCLIHandle cliHandle);


#endif


