#ifndef OS_EXCEPTION_H
#define OS_EXCEPTION_H
#include "os.h"

class osException
{
public:
    explicit osException(const string&   what_,
                        const char      *file_,
                        int             line_,
                        const char      *function_);
    virtual ~osException();

    virtual string      msg() const;

public:
    const string        what;
    const string        file;
    const int           line;
    const string        function;
};

#endif

