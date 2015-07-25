#include "osCurses.h"

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
            m_upWin = subwin(m_winSrc, m_upRow, m_nCol, 0, 0);
            m_downWin = subwin(m_winSrc, m_downRow, m_nCol, 0, 0);
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
        if (m_upWin)    leaveok(m_upWin, TRUE);
        keypad(m_downWin, TRUE);   /* enable cursor keys */
        scrollok(m_downWin, TRUE); /* enable scrolling in main window */

        if (bSplit)
        {
            /* print the message at the center of the screen */
            strBuf = (char*)osMalloc(2 * m_nCol);
            memset(strBuf, '_', m_nCol);
            strBuf[m_nCol] = 0;
            mvprintw(m_downRow - 1, 0, "%s", strBuf);
            refresh();
        }

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
    }

    bool mainLoop(int argc, char** argv)
    {
        strBuf[0] = 0;

        while (1)
        {
            getstr(strBuf);

            if (strncmp(strBuf, "quit", 8) == 0)
            {
                break;
            }
            else
            {
                printw("Invalid command!");
                showHelpInfo();
            }

            refresh();
        }
        return true;
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
        waddstr(m_downWin, "\n   List of commands:\n");
        waddstr(m_downWin, "   help   --   print this help info.\n");
        waddstr(m_downWin, "   trans  --   Tranform video files.\n");
        waddstr(m_downWin, "   abort  --   Abort current jobs.\n");
        waddstr(m_downWin, "   quit   --   Quit this program.\n");
        wrefresh(m_downWin);
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

