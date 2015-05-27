#include "AllMP4Box.h"


MP4StblBox::MP4StblBox(MP4FileClass &file)
        : MP4Box(file, "stbl")
{
    ExpectChildBox("stsd", Required, OnlyOne);
    ExpectChildBox("stts", Required, OnlyOne);
    ExpectChildBox("ctts", Optional, OnlyOne);
    ExpectChildBox("stsz", Required, OnlyOne);
    ExpectChildBox("stz2", Optional, OnlyOne);
    ExpectChildBox("stsc", Required, OnlyOne);
    ExpectChildBox("stco", Optional, OnlyOne);
    ExpectChildBox("co64", Optional, OnlyOne);
    ExpectChildBox("stss", Optional, OnlyOne);
    ExpectChildBox("stsh", Optional, OnlyOne);
    ExpectChildBox("stdp", Optional, OnlyOne);
    ExpectChildBox("sdtp", Optional, OnlyOne);
}

void MP4StblBox::Generate()
{
    // as usual
    MP4Box::Generate();

    // but we also need one of the chunk offset atoms
    MP4Box* pChunkOffsetBox;
    if (m_File.Use64Bits(GetType())) {
        pChunkOffsetBox = CreateBox(m_File, this, "co64");
    } else {
        pChunkOffsetBox = CreateBox(m_File, this, "stco");
    }

    AddChildBox(pChunkOffsetBox);

    // and ask it to self generate
    pChunkOffsetBox->Generate();
}
