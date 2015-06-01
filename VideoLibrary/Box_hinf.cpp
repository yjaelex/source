#include "AllMP4Box.h"


MP4HinfBox::MP4HinfBox(MP4FileClass &file)
        : MP4Box(file, "hinf")
{
    ExpectChildBox("trpy", Optional, OnlyOne);
    ExpectChildBox("nump", Optional, OnlyOne);
    ExpectChildBox("tpyl", Optional, OnlyOne);
    ExpectChildBox("maxr", Optional, Many);
    ExpectChildBox("dmed", Optional, OnlyOne);
    ExpectChildBox("dimm", Optional, OnlyOne);
    ExpectChildBox("drep", Optional, OnlyOne);
    ExpectChildBox("tmin", Optional, OnlyOne);
    ExpectChildBox("tmax", Optional, OnlyOne);
    ExpectChildBox("pmax", Optional, OnlyOne);
    ExpectChildBox("dmax", Optional, OnlyOne);
    ExpectChildBox("payt", Optional, OnlyOne);
}

void MP4HinfBox::Generate()
{
    // hinf is special in that although all it's child atoms
    // are optional (on read), if we generate it for writing
    // we really want all the children

    for (uint32_t i = 0; i < m_vChildBoxInfos.size(); i++)
    {
        MP4Box* pChildBox =
            CreateBox(m_File, this, m_vChildBoxInfos[i]->m_name);

        AddChildBox(pChildBox);

        // and ask it to self generate
        pChildBox->Generate();
    }
}
