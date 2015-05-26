#include "AllMP4Box.h"

MP4HdlrBox::MP4HdlrBox(MP4FileClass &file)
        : MP4FullBox(file, "hdlr")
{
	m_pre_defined = 0;
	m_handler_type = 0;
	memset(m_reserved, 0, sizeof(m_reserved));
}

void MP4HdlrBox::ReadProperties()
{
	// read version and flags */
	MP4FullBox::ReadProperties();

	m_pre_defined = m_File.ReadUInt32();
	m_handler_type = m_File.ReadUInt32();
	m_File.ReadBytes((uint8_t*)m_reserved, sizeof(m_reserved));
	m_name.clear();

	//HANDLER_TYPE_VIDEO,			//¡®vide¡¯ Video track
	//HANDLER_TYPE_AUDIO,			//¡®soun¡¯ Audio track
	//HANDLER_TYPE_HINT,			//¡®hint¡¯ Hint track
	//HANDLER_TYPE_META,			//¡®meta¡¯ Timed Metadata track
	//HANDLER_TYPE_AUXV				//¡®auxv¡¯ Auxiliary Video track
	static uint32 type[5] = { 0 };
	if (type[0] == 0)
	{
		type[0] = STRTOINT32("vide");
		type[1] = STRTOINT32("soun");
		type[2] = STRTOINT32("hint");
		type[3] = STRTOINT32("meta");
		type[4] = STRTOINT32("auxv");
	}

	m_Tracktype = HANDLER_TYPE_UNKNOW;
	for (uint32 i = 0; i < 5; i++)
	{
		if (m_handler_type == type[i])	m_Tracktype = (HANDLER_TYPE)(i + 1);
	}
}
void MP4HdlrBox::Read()
{
    // read all the properties but the "name" field
    ReadProperties();

    uint64_t pos = m_File.GetPosition();
    uint64_t end = GetEnd();
    if (pos == end)
	{
        // A hdlr atom with missing "name".     
        // Apparently that's what some of the iTunes m4p files have.
        return;
    }

    // take a peek at the next byte
    uint8_t strLength;
    m_File.PeekBytes(&strLength, 1);
    // if the value matches the remaining box length
    if (pos + strLength + 1 == end)
	{
        // read a counted string
		char * pStr = m_File.ReadCountedString();
		osAssert(pStr);
		if (pStr)
		{
			m_name.append(pStr);
		}
		m_File.FreeString(pStr);
    } else
	{
        // read a null terminated string
		char * pStr = m_File.ReadString();
		osAssert(pStr);
		if (pStr)
		{
			m_name.append(pStr);
		}
		m_File.FreeString(pStr);
    }

    Skip(); // to end of atom
}


void MP4HdlrBox::DumpProperties(uint8_t indent, bool dumpImplicits)
{
	MP4FullBox::DumpProperties(indent, dumpImplicits);
	char type[5] = { 0 };
	INT32TOSTR(m_handler_type, type);
	osDump(indent, "Handler Type: %s(0x%x)\n", type, m_handler_type);

	if (false == m_name.empty())
	{
		osDump(indent, "Name: %s\n", m_name.c_str());
	}
}
