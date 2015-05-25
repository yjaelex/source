#include "AllMP4Box.h"

MP4RootBox::MP4RootBox(MP4FileClass &file)
    : MP4Box( file, NULL )
    , m_rewrite_ftyp         ( NULL )
    , m_rewrite_ftypPosition ( 0 )
    , m_rewrite_free         ( NULL )
    , m_rewrite_freePosition ( 0 )
{
    ExpectChildBox( "moov", Required, OnlyOne );
    ExpectChildBox( "ftyp", Optional, OnlyOne );
    ExpectChildBox( "mdat", Optional, Many );
    ExpectChildBox( "free", Optional, Many );
    ExpectChildBox( "skip", Optional, Many );
    ExpectChildBox( "udta", Optional, Many );
    ExpectChildBox( "moof", Optional, Many );
}

void MP4RootBox::BeginWrite(bool use64)
{
    m_rewrite_ftyp = (MP4FtypBox*)FindChildBox( "ftyp" );
    if( m_rewrite_ftyp )
	{
        m_rewrite_free = (MP4FreeBox*)MP4Box::CreateBox( m_File, NULL, "free" );
        m_rewrite_free->SetSize( 32*4 ); // room for 32 additional brands
        AddChildBox( m_rewrite_free );

        m_rewrite_ftypPosition = m_File.GetPosition();
        m_rewrite_ftyp->Write();

        m_rewrite_freePosition = m_File.GetPosition();
        m_rewrite_free->Write();
    }

    m_vChildBoxs[GetLastMdatIndex()]->BeginWrite( m_File.Use64Bits( "mdat" ));
}

void MP4RootBox::Write()
{
    // no-op
}

void MP4RootBox::FinishWrite(bool use64)
{
    if( m_rewrite_ftyp )
	{
        const uint64_t savepos = m_File.GetPosition();
        m_File.SetPosition( m_rewrite_ftypPosition );
        m_rewrite_ftyp->Write();

        const uint64_t newpos = m_File.GetPosition();
        if( newpos > m_rewrite_freePosition )
            m_rewrite_free->SetSize( m_rewrite_free->GetSize() - (newpos - m_rewrite_freePosition) ); // shrink
        else if( newpos < m_rewrite_freePosition )
            m_rewrite_free->SetSize( m_rewrite_free->GetSize() + (m_rewrite_freePosition - newpos) ); // grow

        m_rewrite_free->Write();
        m_File.SetPosition( savepos );
    }

    // finish writing last mdat box
    const uint32_t mdatIndex = GetLastMdatIndex();
    m_vChildBoxs[mdatIndex]->FinishWrite( m_File.Use64Bits( "mdat" ));

    // write all box after last mdat
    const uint32_t size = m_vChildBoxs.size();
    for ( uint32_t i = mdatIndex + 1; i < size; i++ )
		m_vChildBoxs[i]->Write();
}

void MP4RootBox::BeginOptimalWrite()
{
    WriteBoxType("ftyp", OnlyOne);
    WriteBoxType("moov", OnlyOne);
    WriteBoxType("udta", Many);

	m_vChildBoxs[GetLastMdatIndex()]->BeginWrite(m_File.Use64Bits("mdat"));
}

void MP4RootBox::FinishOptimalWrite()
{
    // finish writing mdat
	m_vChildBoxs[GetLastMdatIndex()]->FinishWrite(m_File.Use64Bits("mdat"));

    // find moov box
    uint32_t size = m_vChildBoxs.size();
    MP4Box* pMoovBox = NULL;

    uint32_t i;
    for (i = 0; i < size; i++) {
        if (!strcmp("moov", m_vChildBoxs[i]->GetType())) {
            pMoovBox = m_vChildBoxs[i];
            break;
        }
    }
    osAssert(i < size);
	osAssert(pMoovBox != NULL);

    // rewrite moov so that updated chunkOffsets are written to disk
    m_File.SetPosition(pMoovBox->GetStart());
    uint64_t oldSize = pMoovBox->GetSize();

    pMoovBox->Write();

    // sanity check
    uint64_t newSize = pMoovBox->GetSize();
	osAssert(oldSize == newSize);
}

uint32_t MP4RootBox::GetLastMdatIndex()
{
    for (int32_t i = m_vChildBoxs.size() - 1; i >= 0; i--) {
        if (!strcmp("mdat", m_vChildBoxs[i]->GetType())) {
            return i;
        }
    }
	osAssert(false);
    return (uint32_t)-1;
}

void MP4RootBox::WriteBoxType(const char* type, bool onlyOne)
{
    uint32_t size = m_vChildBoxs.size();

    for (uint32_t i = 0; i < size; i++) {
        if (!strcmp(type, m_vChildBoxs[i]->GetType())) {
            m_vChildBoxs[i]->Write();
            if (onlyOne) {
                break;
            }
        }
    }
}

