#include "AllMP4Box.h"

MP4SoundBox::MP4SoundBox(MP4FileClass &file, const char *boxid)
        : MP4Box(file, boxid)
{
	memset(&m_entry, 0, sizeof(m_entry));
	m_reserved1[0] = m_reserved1[1] = 0;
	m_channelcount = 0;
	m_samplesize = 0;
	m_pre_defined = 0;
	m_reserved2 = 0;
	m_samplerate = 0;

	m_soundVersion2Data.clear();

    if (BoxID(boxid) == BoxID("mp4a")) {
        ExpectChildBox("esds", Required, OnlyOne);
        ExpectChildBox("wave", Optional, OnlyOne);
    } else if (BoxID(boxid) == BoxID("alac")) {
        ExpectChildBox("alac", Optional, Optional);

    }
}

void MP4SoundBox::Generate()
{
    MP4Box::Generate();

	m_channelcount = 2;
	m_samplesize = 16;

    // property reserved2 has non-zero fixed values
    static const uint8_t reserved2[6] = {
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00,
    };
    m_pProperties[3]->SetReadOnly(false);
    ((MP4BytesProperty*)m_pProperties[3])->
    SetValue(reserved2, sizeof(reserved2));
    m_pProperties[3]->SetReadOnly(true);
    ((MP4Integer16Property*)m_pProperties[4])->SetValue(2);
    ((MP4Integer16Property*)m_pProperties[5])->SetValue(0x0010);
    ((MP4Integer16Property*)m_pProperties[6])->SetValue(0);

}

void MP4SoundBox::Read()
{
    MP4Box *parent = GetParentBox();
    if (ATOMID(parent->GetType()) != ATOMID("stsd")) {
        // Quicktime has an interesting thing - they'll put an mp4a box
        // which is blank inside a wave box, which is inside an mp4a box
        // we have a mp4a inside an wave inside an mp4a - delete all properties
        for(int i = 0; i < 9; ++ i)
            delete m_pProperties[i];	// make sure we delete the properties themselves, then remove from  m_pProperties

        m_pProperties.Delete(8);
        m_pProperties.Delete(7);
        m_pProperties.Delete(6);
        m_pProperties.Delete(5);
        m_pProperties.Delete(4);
        m_pProperties.Delete(3);
        m_pProperties.Delete(2);
        m_pProperties.Delete(1);
        m_pProperties.Delete(0);

        if (ATOMID(GetType()) == ATOMID("alac")) {
            AddProperty(new MP4BytesProperty(*this, "decoderConfig", m_size));
            ReadProperties();
        }
        if (m_pChildBoxInfos.Size() > 0) {
            ReadChildBoxs();
        }
    } else {
        ReadProperties(0, 3); // read first 3 properties
        AddProperties(((MP4IntegerProperty *)m_pProperties[2])->GetValue());
        ReadProperties(3); // continue
        if (m_pChildBoxInfos.Size() > 0) {
            ReadChildBoxs();
        }
    }
    Skip();
}
