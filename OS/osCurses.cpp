#include "osCurses.h"
#include <sstream>

class osCLI
{

public:
    osCLI()
    {
        m_winSrc = NULL;
        m_upRow = 0;
        m_downRow = 0;
        m_maxRow = m_maxCol = m_nCol = 0;
        strBuf = NULL;
    }

    ~osCLI()
    {
        if (strBuf)     osFree(strBuf);
    }

    bool create(uint32 width, uint32 height, bool bSplit)
    {
        m_winSrc = initscr();				        /* start the curses mode */
        osAssert(m_winSrc);
        /* Resize the terminal to something larger than the physical screen */
        resize_term(2000, 2000);
        /* Get the largest physical screen dimensions */
        getmaxyx(m_winSrc, m_maxRow, m_maxCol);

        m_upRow = min(height, m_maxRow * 4 / 5);
        m_nCol = min(width, m_maxCol * 4 / 5);

        /* Resize so it fits */
        resize_term(m_upRow - 1, m_nCol - 1);
        /* Get the screen dimensions that fit */
        getmaxyx(m_winSrc, m_upRow, m_nCol);

        initcolor();
        if (bSplit)
        {
            m_downRow = m_upRow / 2;
            m_upRow = m_upRow - m_downRow - 1;
            m_nCol = m_nCol - 1;
            m_upWin = subwin(m_winSrc, m_upRow, m_nCol, 0, 1);
            m_downWin = subwin(m_winSrc, m_downRow, m_nCol, m_upRow + 1, 1);
            if (!m_upWin || !m_downWin)
            {
                m_upWin = m_downWin = NULL;
                endwin();
                return false;
            }
        }
        else
        {
            m_downRow = 0;
            m_downWin = m_winSrc;
            if (!m_downWin)
            {
                m_upWin = m_downWin = NULL;
                endwin();
                return false;
            }
            m_upWin = NULL;
        }

        keypad(m_downWin, TRUE);   /* enable cursor keys */
        scrollok(m_downWin, TRUE); /* enable scrolling in main window */

        if (bSplit)
        {
            //leaveok(m_upWin, TRUE);
            scrollok(m_upWin, TRUE);
            /* print the message at the center of the screen */
            strBuf = (char*)osMalloc(2 * m_nCol);
            memset(strBuf, '_', m_nCol);
            strBuf[m_nCol] = 0;
            mvprintw(m_upRow, 0, "%s", strBuf);
            refresh();
        }

        move(getbegy(m_downWin), getbegx(m_downWin));
        return true;
    }

    void close()
    {
        if (m_upWin)
        {
            delwin(m_downWin);
            delwin(m_upWin);
            endwin();
        }
        if (strBuf)     osFree(strBuf);
        strBuf = NULL;
        m_upWin = m_downWin = NULL;
        m_vecCallBack.clear();
    }

    bool mainLoop(int argc, char** argv)
    {
        strBuf[0] = 0;
        wmove(m_downWin, 0, 0);
        while (1)
        {
            curs_set(1);
            wgetstr(m_downWin, strBuf);

            if (strncmp(strBuf, "quit", 8) == 0)
            {
                break;
            }
            else if (strncmp(strBuf, "help", 8) == 0)
            {
                showHelpInfo();
            }
            else
            {
                std::vector<char*>  args;
                parseCmdArgs(strBuf, args);
                if (args.empty())     continue;

                uint32 i = 0;
                for (i = 0; i < m_vecCallBack.size(); i++)
                {
                    if (strncmp(args[0], m_vecCallBack[i].cmdName.c_str(), 8) == 0)
                    {
                        m_vecCallBack[i].pfnFunc(args.size(), &args[0]);
                        break;
                    }
                }
                freeCmdArgs(args);
                if (i == m_vecCallBack.size())
                {
                    print(CLI_DOWN_WIN, "Invalid command!");
                    showHelpInfo();
                }
            }

            wrefresh(m_downWin);
        }
        return true;
    }

    void pushCmd(osCLIFuncCB * pCB)
    {
        if (!pCB)   return;
        m_vecCallBack.push_back(*pCB);
    }

    void print(CLIArea area, const char * str, uint32 * curRow = NULL)
    {
        WINDOW * win = NULL;
        switch (area)
        {
        case CLI_UP_WIN:
            win = m_upWin;
            curs_set(0);
            break;
        case CLI_DOWN_WIN:
        case CLI_SRC:
            curs_set(1);
            win = m_downWin;
            break;
        default:
            win = m_downWin;
            break;
        }
        osAssert(win);
        waddstr(win, str);
        wrefresh(win);
        if (curRow)     *curRow = getcury(win);
    }

    void print(WINDOW * win, const char * str, uint32 * curRow = NULL)
    {
        osAssert(win);
        if (win == m_upWin)
        {
            curs_set(0);
        }
        else
        {
            curs_set(1);
        }
        waddstr(win, str);
        wrefresh(win);
        if (curRow)     *curRow = getcury(win);
    }

    WINDOW* getwin(CLIArea area)
    {
        WINDOW * win = NULL;
        switch (area)
        {
        case CLI_UP_WIN:
            win = m_upWin;
            break;
        case CLI_DOWN_WIN:
        case CLI_SRC:
            win = m_downWin;
            break;
        default:
            win = m_downWin;
            break;
        }
        return win;
    }

private:
    void initcolor(void)
    {
#ifdef A_COLOR
        if (has_colors())
            start_color();

        /* foreground, background */
        init_pair(0, COLOR_BLACK, COLOR_CYAN);
        init_pair(1, COLOR_WHITE, COLOR_CYAN);
        init_pair(2, COLOR_WHITE, COLOR_BLACK);
        init_pair(3, COLOR_WHITE, COLOR_CYAN);
        init_pair(4, COLOR_WHITE, COLOR_BLACK);
        init_pair(5, COLOR_WHITE, COLOR_BLUE);
        init_pair(6, COLOR_WHITE, COLOR_CYAN);
        init_pair(7, COLOR_BLACK, COLOR_CYAN);
        init_pair(8, COLOR_WHITE, COLOR_BLACK);
#endif
    }

    void showHelpInfo()
    {
        print(m_downWin, "\n   List of commands:\n");
        print(m_downWin, "   help   --   print this help info.\n");
        print(m_downWin, "   trans  --   Tranform video files.\n");
        print(m_downWin, "   abort  --   Abort current jobs.\n");
        print(m_downWin, "   quit   --   Quit this program.\n");
    }

    void parseCmdArgs(char * cmdStr, std::vector<char *> & args)
    {
        std::istringstream iss(cmdStr);
        std::string token;
        while (iss >> token)
        {
            char *arg = new char[token.size() + 1];
            std::copy(token.begin(), token.end(), arg);
            arg[token.size()] = '\0';
            args.push_back(arg);
        }

        // now exec with &args[0], and then:
    }

    void freeCmdArgs(std::vector<char *> & args)
    {
        for (size_t i = 0; i < args.size(); i++)
        {
            delete[] args[i];
            args[i] = NULL;
        }
    }

private:
    WINDOW*                     m_winSrc;
    WINDOW*                     m_upWin;
    WINDOW*                     m_downWin;
    uint32                      m_maxRow;
    uint32                      m_maxCol;
    uint32                      m_upRow;
    uint32                      m_downRow;
    uint32                      m_nCol;
    char *                      strBuf;
    vector<osCLIFuncCB>         m_vecCallBack;
};

osCLIHandle osCreateCmdLineInterface(uint32 width, uint32 height, bool bSplit)
{
    osCLI * pCLI = new osCLI();
    pCLI->create(width, height, bSplit);

    return (osCLIHandle)pCLI;
}

bool osMainLoopCmdLineInterface(osCLIHandle cliHandle, int argc, char** argv)
{
    osCLI * pCLI = (osCLI*)cliHandle;
    return pCLI->mainLoop(argc, argv);
}

void osDesrotyCmdLineInterface(osCLIHandle cliHandle)
{
    osCLI * pCLI = (osCLI*)cliHandle;
    pCLI->close();

    delete pCLI;
}

void osSetCallBackFuncCmdLineInterface(osCLIHandle cliHandle, uint32 num, osCLIFuncCB * pCB)
{
    osCLI * pCLI = (osCLI*)cliHandle;
    if (!pCB)   return;

    for (uint32 i = 0; i < num; i++)
    {
        pCLI->pushCmd(pCB);
        pCB++;
    }
}

pvoid osGetCmdWinCmdLineInterface(osCLIHandle cliHandle)
{
    osCLI * pCLI = (osCLI*)cliHandle;
    return (pvoid) pCLI->getwin(CLI_DOWN_WIN);
}
pvoid osGetInfoWinCmdLineInterface(osCLIHandle cliHandle)
{
    osCLI * pCLI = (osCLI*)cliHandle;
    return (pvoid)pCLI->getwin(CLI_UP_WIN);
}

void osPrintCmdLineInterface(osCLIHandle cliHandle, pvoid win, const char * str)
{
    osCLI * pCLI = (osCLI*)cliHandle;
    pCLI->print((WINDOW*)win, str);
}

