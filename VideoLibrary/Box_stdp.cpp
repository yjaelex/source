

#include "AllMP4Box.h"





MP4StdpBox::MP4StdpBox(MP4FileClass &file)
        : MP4Box(file, "stdp")
{
    AddVersionAndFlags();

    MP4Integer32Property* pCount =
        new MP4Integer32Property(*this, "entryCount");
    pCount->SetImplicit();
    AddProperty(pCount);

    MP4TableProperty* pTable = new MP4TableProperty(*this, "entries", pCount);
    AddProperty(pTable);

    pTable->AddProperty(
        new MP4Integer16Property(pTable->GetParentBox(), "priority"));
}

void MP4StdpBox::Read()
{
    // table entry count computed from atom size
    ((MP4Integer32Property*)m_pProperties[2])->SetReadOnly(false);
    ((MP4Integer32Property*)m_pProperties[2])->SetValue((m_size - 4) / 2);
    ((MP4Integer32Property*)m_pProperties[2])->SetReadOnly(true);

    MP4Box::Read();
}



}
} // namespace mp4v2::impl
