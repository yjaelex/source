
#include "osGetOpt.h"
#include "Simpleopt.h"

#if defined(_MSC_VER)
# include <windows.h>
# include <tchar.h>
#else
# define TCHAR		char
# define _T(x)		x
# define _tprintf	printf
# define _tmain		main
# define _ttoi      atoi
#endif

static CSimpleOpt::SOption g_rgFlags[] =
{
    { SO_O_CLUMP, _T("--clump"), SO_NONE },
    { SO_O_ICASE_SHORT, _T("--icase-short"), SO_NONE },
    SO_END_OF_OPTIONS,
};

////////////////////////////////////////////////////////////////////////////
// Examples:
//static CSimpleOpt::SOption g_rgOptions[] =
//{
//    { OPT_HELP, _T("-?"), SO_NONE },
//    { OPT_HELP, _T("-h"), SO_NONE },
//    { OPT_HELP, _T("-help"), SO_NONE },
//    { OPT_HELP, _T("--help"), SO_NONE },
//};
//static void ShowUsage()
//{
//    _tprintf(
//        _T("Usage: fullSample [OPTIONS] [FILES]\n")
//        _T("\n")
//        _T("--exact         Disallow partial matching of option names\n")
//        _T("--noslash       Disallow use of slash as an option marker on Windows\n")
//        _T("--shortarg      Permit arguments on single letter options with no equals sign\n")
//        _T("--clump         Permit single char options to be clumped as long string\n")
//        _T("--noerr         Do not generate any errors for invalid options\n")
//        _T("--pedantic      Generate an error for petty things\n")
//        _T("--icase         Case-insensitive for all types\n")
//        _T("--icase-short   Case-insensitive for short args\n")
//        _T("--icase-long    Case-insensitive for long argsn")
//        _T("--icase-word    Case-insensitive for word args\n")
//        _T("\n")
//        _T("-d  -e  -E  -f  -F  -g  -flag  --flag       Flag (no arg)\n")
//        _T("-s ARG   -sep ARG  --sep ARG                Separate required arg\n")
//        _T("-S ARG   -SEP ARG  --SEP ARG                Separate required arg (uppercase)\n")
//        _T("-cARG    -c=ARG    -com=ARG    --com=ARG    Combined required arg\n")
//        _T("-o[ARG]  -o[=ARG]  -opt[=ARG]  --opt[=ARG]  Combined optional arg\n")
//        _T("-man     -mandy    -mandate                 Shortcut matching tests\n")
//        _T("--man    --mandy   --mandate                Shortcut matching tests\n")
//        _T("--multi0 --multi1 ARG --multi2 ARG1 ARG2    Multiple argument tests\n")
//        _T("--multi N ARG-1 ARG-2 ... ARG-N             Multiple argument tests\n")
//        _T("--                                          Stop argument processing\n")
//        _T("open read write close zip unzip UPCASE      Special words\n")
//        _T("\n")
//        _T("-?  -h  -help  --help                       Output this help.\n")
//        _T("\n")
//        _T("If a FILE is `-', read standard input.\n")
//        );
//}
//
//CSimpleOpt::SOption g_rgFlags[] =
//{
//    { SO_O_EXACT, _T("--exact"), SO_NONE },
//    { SO_O_NOSLASH, _T("--noslash"), SO_NONE },
//    { SO_O_SHORTARG, _T("--shortarg"), SO_NONE },
//    { SO_O_CLUMP, _T("--clump"), SO_NONE },
//    { SO_O_NOERR, _T("--noerr"), SO_NONE },
//    { SO_O_PEDANTIC, _T("--pedantic"), SO_NONE },
//    { SO_O_ICASE, _T("--icase"), SO_NONE },
//    { SO_O_ICASE_SHORT, _T("--icase-short"), SO_NONE },
//    { SO_O_ICASE_LONG, _T("--icase-long"), SO_NONE },
//    { SO_O_ICASE_WORD, _T("--icase-word"), SO_NONE },
//    SO_END_OF_OPTIONS
//};
//
//enum { OPT_HELP = 0, OPT_MULTI = 100, OPT_MULTI0, OPT_MULTI1, OPT_MULTI2, OPT_STOP };
//CSimpleOpt::SOption g_rgOptions[] =
//{
//    { OPT_HELP, _T("-?"), SO_NONE },
//    { OPT_HELP, _T("-h"), SO_NONE },
//    { OPT_HELP, _T("-help"), SO_NONE },
//    { OPT_HELP, _T("--help"), SO_NONE },
//    { 1, _T("-"), SO_NONE },
//    { 2, _T("-d"), SO_NONE },
//    { 3, _T("-e"), SO_NONE },
//    { 4, _T("-f"), SO_NONE },
//    { 5, _T("-g"), SO_NONE },
//    { 6, _T("-flag"), SO_NONE },
//    { 7, _T("--flag"), SO_NONE },
//    { 8, _T("-s"), SO_REQ_SEP },
//    { 9, _T("-sep"), SO_REQ_SEP },
//    { 10, _T("--sep"), SO_REQ_SEP },
//    { 11, _T("-c"), SO_REQ_CMB },
//    { 12, _T("-com"), SO_REQ_CMB },
//    { 13, _T("--com"), SO_REQ_CMB },
//    { 14, _T("-o"), SO_OPT },
//    { 15, _T("-opt"), SO_OPT },
//    { 16, _T("--opt"), SO_OPT },
//    { 17, _T("-man"), SO_NONE },
//    { 18, _T("-mandy"), SO_NONE },
//    { 19, _T("-mandate"), SO_NONE },
//    { 20, _T("--man"), SO_NONE },
//    { 21, _T("--mandy"), SO_NONE },
//    { 22, _T("--mandate"), SO_NONE },
//    { 23, _T("open"), SO_NONE },
//    { 24, _T("read"), SO_NONE },
//    { 25, _T("write"), SO_NONE },
//    { 26, _T("close"), SO_NONE },
//    { 27, _T("zip"), SO_NONE },
//    { 28, _T("unzip"), SO_NONE },
//    { 29, _T("-E"), SO_NONE },
//    { 30, _T("-F"), SO_NONE },
//    { 31, _T("-S"), SO_REQ_SEP },
//    { 32, _T("-SEP"), SO_REQ_SEP },
//    { 33, _T("--SEP"), SO_REQ_SEP },
//    { 34, _T("UPCASE"), SO_NONE },
//    { OPT_MULTI, _T("--multi"), SO_MULTI },
//    { OPT_MULTI0, _T("--multi0"), SO_MULTI },
//    { OPT_MULTI1, _T("--multi1"), SO_MULTI },
//    { OPT_MULTI2, _T("--multi2"), SO_MULTI },
//    { OPT_STOP, _T("--"), SO_NONE },
//    SO_END_OF_OPTIONS
//};

static CSimpleOpt::SOption g_endOpt = SO_END_OF_OPTIONS;

static const TCHAR * GetLastErrorText(int a_nError)
{
    switch (a_nError)
    {
    case SO_SUCCESS:            return _T("Success");
    case SO_OPT_INVALID:        return _T("Unrecognized option");
    case SO_OPT_MULTIPLE:       return _T("Option matched multiple strings");
    case SO_ARG_INVALID:        return _T("Option does not accept argument");
    case SO_ARG_INVALID_TYPE:   return _T("Invalid argument format");
    case SO_ARG_MISSING:        return _T("Required argument is missing");
    case SO_ARG_INVALID_DATA:   return _T("Invalid argument data");
    default:                    return _T("Unknown error");
    }
}

typedef struct CmdOptHandle
{
    char** myArgv = NULL;
    CSimpleOpt::SOption * pOptions = NULL;
    CSimpleOpt * pArgs;
}CmdOptHandle;

pvoid osCreateCmdLineOptHandler(int argc, char ** argv, CmdOpt * pCmdOpt, uint32 nNumOfOpts)
{
    int myArgc = argc + sizeof(g_rgFlags) / sizeof(CSimpleOpt::SOption) - 1;

    CmdOptHandle * handler = new CmdOptHandle();
    if (handler == NULL) return NULL;

    handler->myArgv = (char**)osMalloc(sizeof(char*)*(myArgc));
    uint32 i = 0;

    if (handler->myArgv == NULL) return NULL;
    memcpy(handler->myArgv, argv, sizeof(char*)*argc);
    while (g_rgFlags[i].nId != -1)
    {
        handler->myArgv[i + argc] = const_cast<char*>(g_rgFlags[i].pszArg);
        i++;
    }
    osAssert((i + argc) == myArgc);

    handler->pOptions = (CSimpleOpt::SOption*) osMalloc(sizeof(CSimpleOpt::SOption)*(nNumOfOpts + 1));
    if (handler->pOptions == NULL)
    {
        osFree(handler->myArgv);
        return NULL;
    }

    memcpy(handler->pOptions, pCmdOpt, sizeof(CSimpleOpt::SOption)*(nNumOfOpts));
    handler->pOptions[nNumOfOpts] = g_endOpt;

    int nFlags = SO_O_USEALL;
    handler->pArgs = new CSimpleOpt(myArgc, handler->myArgv, handler->pOptions, SO_O_NOERR | SO_O_EXACT);
    if (handler->pArgs == NULL)
    {
        osFree(handler->myArgv);
        osFree(handler->pOptions);
        handler->myArgv = NULL;
        handler->pOptions = NULL;
        return NULL;
    }

    return handler;
}

void osOptArgsStop(pvoid handler)
{
    CSimpleOpt * pArgs = ((CmdOptHandle *)handler)->pArgs;
    pArgs->Stop();
}

bool osGetOpt(pvoid handler, int * pID, bool * pErr)
{
    osAssert(handler && pID);
    CSimpleOpt * pArgs = ((CmdOptHandle *)handler)->pArgs;
    bool ret = pArgs->Next();
    if (!ret) return ret;

    *pID = pArgs->OptionId();
    *pErr = false;
    if (pArgs->LastError() != SO_SUCCESS)
    {
        osPrintf( _T("%s: '%s' (use --help to get command line help)\n"),
            GetLastErrorText(pArgs->LastError()), pArgs->OptionText());
        *pErr = true;
    }

    return ret;
}

bool osDoMultiArgs(pvoid handler, int  nMultiArgs, pfnDoMultiArgsCB pFunc)
{
    osAssert(handler && pFunc);
    CSimpleOpt * pArgs = ((CmdOptHandle *)handler)->pArgs;
    TCHAR ** rgpszArg = NULL;

    // get the number of arguments if necessary
    if (nMultiArgs == -1) 
    {
        // first arg is a count of how many we have
        rgpszArg = pArgs->MultiArg(1);
        if (!rgpszArg)
        {
            osPrintf( _T("%s: '%s' (use --help to get command line help)\n"),
                GetLastErrorText(pArgs->LastError()), pArgs->OptionText());
            return false;
        }

        nMultiArgs = _ttoi(rgpszArg[0]);
    }

    // get the arguments to follow
    rgpszArg = pArgs->MultiArg(nMultiArgs);
    if (!rgpszArg)
    {
        osPrintf( _T("%s: '%s' (use --help to get command line help)\n"),
            GetLastErrorText(pArgs->LastError()), pArgs->OptionText());
        return false;
    }

    for (int n = 0; n < nMultiArgs; ++n)
    {
        pFunc(pArgs->OptionId(), pArgs->OptionText(), n, rgpszArg[n]);
    }

    return true;
}

bool osDoArgs(pvoid handler, pfnDoArgsCB pFunc)
{
    osAssert(handler && pFunc);
    CSimpleOpt * pArgs = ((CmdOptHandle *)handler)->pArgs;
    char * pArgStr = pArgs->OptionArg();

    pFunc(pArgs->OptionId(), pArgs->OptionText(), pArgs->OptionArg());
    return true;
}

void osDestroyCmdLineOptHandler(pvoid handler)
{
    osAssert(handler);
    CSimpleOpt * pArgs = (CSimpleOpt *)((CmdOptHandle *)handler)->pArgs;
    osFree(((CmdOptHandle *)handler)->myArgv);
    osFree(((CmdOptHandle *)handler)->pOptions);
    delete pArgs;
}

