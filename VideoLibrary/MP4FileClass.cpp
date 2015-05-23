#include "MP4FileClass.h"

bool MP4FileClass::Use64Bits(const char *atomName)
{
    uint32_t atomid = BoxID(atomName);
    if (atomid == BoxID("mdat") || atomid == BoxID("stbl")) {
        return (m_createFlags & MP4_CREATE_64BIT_DATA) == MP4_CREATE_64BIT_DATA;
    }
    if (atomid == BoxID("mvhd") ||
        atomid == BoxID("tkhd") ||
        atomid == BoxID("mdhd")) {
        return (m_createFlags & MP4_CREATE_64BIT_TIME) == MP4_CREATE_64BIT_TIME;
    }
    return false;
}

void MP4FileClass::Check64BitStatus(const char *atomName)
{
    uint32_t atomid = BoxID(atomName);

    if (atomid == BoxID("mdat") || atomid == BoxID("stbl")) {
        m_createFlags |= MP4_CREATE_64BIT_DATA;
    }
    else if (atomid == BoxID("mvhd") ||
        atomid == BoxID("tkhd") ||
        atomid == BoxID("mdhd")) {
        m_createFlags |= MP4_CREATE_64BIT_TIME;
    }
}
