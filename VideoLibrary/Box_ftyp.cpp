#include "AllMP4Box.h"

MP4FtypBox::MP4FtypBox(MP4FileClass &file)
    : MP4Box( file, "ftyp" )
{
    m_majorBrand = 0;
	m_minorVersion = 0;
	m_compatibleBrands.clear();
}

void MP4FtypBox::Generate()
{
    MP4Box::Generate();

	m_majorBrand = STRTOINT32("mp42");
	m_minorVersion = 0;

	m_compatibleBrands.push_back(STRTOINT32("mp42"));
	m_compatibleBrands.push_back(STRTOINT32("isom"));
}

void MP4FtypBox::ReadProperties()
{
	m_majorBrand = m_File.ReadUInt32();
	m_minorVersion = m_File.ReadUInt32();

	uint32 compatibleBrandsCount = (m_nSize - 8) / 4;		// brands array fills rest of box
	for (uint32 i = 0; i < compatibleBrandsCount; i++)
	{
		m_compatibleBrands.push_back(m_File.ReadUInt32());
	}

	osAssert(GetEnd() == m_File.GetPosition());
}

void MP4FtypBox::Read()
{
    MP4Box::Read();
}
