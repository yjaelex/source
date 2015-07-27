#include "osException.h"
#include <sstream>

osException::osException(const string&     what_,
    const char        *file_,
    int               line_,
    const char        *function_)
    : what(what_)
    , file(file_)
    , line(line_)
    , function(function_)
{
    osAssert(file_);
    osAssert(function_);
}

osException::~osException()
{
}

string
osException::msg() const
{
    ostringstream retval;

    retval << function << ": " << what << " (" << file << "," << line << ")";

    return retval.str();
}
