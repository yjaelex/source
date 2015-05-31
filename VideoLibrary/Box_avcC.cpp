#include "AllMP4Box.h"


MP4AvcCBox::MP4AvcCBox(MP4FileClass &file)
        : MP4Box(file, "avcC")
{

}

//
// Clone - clone my properties to destination atom
//
// this method simplifies duplicating avcC atom properties from
// source to destination file using a single API rather than
// having to copy each property.  This API encapsulates the object
// so the application layer need not concern with each property
// thereby isolating any future changes to atom properties.
//
// ----------------------------------------
// property   description
// ----------------------------------------
//
// 0    configurationVersion
// 1    AVCProfileIndication
// 2    profile_compatibility
// 3    AVCLevelIndication
// 4    reserved
// 5    lengthSizeMinusOne
// 6    reserved
// 7    number of SPS
// 8    SPS entries
// 9    number of PPS
// 10   PPS entries
//
//


