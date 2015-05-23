#include "AllMP4Box.h"

MP4Ac3Box::MP4Ac3Box(MP4FileClassClass &file)
        : MP4Box(file, "ac-3")
{
    AddReserved(*this, "reserved1", 6); /* 0 */

    AddProperty( /* 1 */
        new MP4Integer16Property(*this,"dataReferenceIndex"));

    AddReserved(*this,"reserved2", 8); /* 2 */

    AddProperty( /* 3 */
        new MP4Integer16Property(*this,"channelCount"));

    AddProperty( /* 4 */
        new MP4Integer16Property(*this,"sampleSize"));

    AddReserved(*this,"reserved3", 4); /* 5 */

    AddProperty( /* 6 */
        new MP4Integer16Property(*this,"samplingRate"));

    AddReserved(*this,"reserved4", 2); /* 7 */

    ExpectChildBox("dac3", Required, OnlyOne);
}

void MP4Ac3Box::Generate()
{
    MP4Box::Generate();

    ((MP4Integer16Property*)m_pProperties[1])->SetValue(1); // data-reference-index
    ((MP4Integer16Property*)m_pProperties[3])->SetValue(2); // channelCount - ignored
    ((MP4Integer16Property*)m_pProperties[4])->SetValue(0x0010); // sampleSize - ignored

    // The user should set the samplingRate as appropriate - and create the dac3 atom
}



}
} // namespace mp4v2::impl
