

#include "AllMP4Box.h"





MP4StszBox::MP4StszBox(MP4FileClass &file)
        : MP4Box(file, "stsz")
{
    AddVersionAndFlags(); /* 0, 1 */

    AddProperty( /* 2 */
        new MP4Integer32Property(*this, "sampleSize"));

    MP4Integer32Property* pCount =
        new MP4Integer32Property(*this, "sampleCount");
    AddProperty(pCount); /* 3 */

    MP4TableProperty* pTable = new MP4TableProperty(*this, "entries", pCount);
    AddProperty(pTable); /* 4 */

    pTable->AddProperty( /* 4/0 */
        new MP4Integer32Property(pTable->GetParentBox(), "entrySize"));
}

void MP4StszBox::Read()
{
    ReadProperties(0, 4);

    uint32_t sampleSize =
        ((MP4Integer32Property*)m_pProperties[2])->GetValue();

    // only attempt to read entries table if sampleSize is zero
    // i.e sample size is not constant
    m_pProperties[4]->SetImplicit(sampleSize != 0);

    ReadProperties(4);

    Skip(); // to end of atom
}

void MP4StszBox::Write()
{
    uint32_t sampleSize =
        ((MP4Integer32Property*)m_pProperties[2])->GetValue();

    // only attempt to write entries table if sampleSize is zero
    // i.e sample size is not constant
    m_pProperties[4]->SetImplicit(sampleSize != 0);

    MP4Box::Write();
}



}
} // namespace mp4v2::impl
