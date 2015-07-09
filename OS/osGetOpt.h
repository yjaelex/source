#ifndef _OS_GETOPT_H
#define _OS_GETOPT_H
#include "os.h"

enum { OPT_HELP = 0, OPT_MULTI = 100, OPT_MULTI0, OPT_MULTI1, OPT_MULTI2, OPT_STOP };

typedef enum OptArgType {
    /*! No argument. Just the option flags.
    e.g. -o         --opt */
    OPT_TYPE_NONE,

    /*! Required separate argument.
    e.g. -o ARG     --opt ARG */
    OPT_TYPE_REQ_SEP,

    /*! Required combined argument.
    e.g. -oARG      -o=ARG      --opt=ARG  */
    OPT_TYPE_REQ_CMB,

    /*! Optional combined argument.
    e.g. -o[ARG]    -o[=ARG]    --opt[=ARG] */
    OPT_TYPE_OPT,

    /*! Multiple separate arguments. The actual number of arguments is
    determined programatically at the time the argument is processed.
    e.g. -o N ARG1 ARG2 ... ARGN    --opt N ARG1 ARG2 ... ARGN */
    OPT_TYPE_MULTI
} OptArgType;

typedef struct CmdOpt {
    /*! ID to return for this flag. Optional but must be >= 0 */
    int nId;

    /*! arg string to search for, e.g.  "open", "-", "-f", "--file"
    Note that on Windows the slash option marker will be converted
    to a hyphen so that "-f" will also match "/f". */
    const char * pszArg;

    /*! type of argument accepted by this option */
    int nArgType;
}CmdOpt;

pvoid osCreateCmdLineOptHandler(int argc, char ** argv, CmdOpt * pCmdOpt, uint32 nNumOfOpts);

#endif

