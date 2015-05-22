#include "MP4Box.h"
#include "os.h"
#include "AllMP4Box.h"
#include <list>

MP4BoxInfo::MP4BoxInfo(const char* name, bool mandatory, bool onlyOne)
{
    m_name = name;
    m_mandatory = mandatory;
    m_onlyOne = onlyOne;
    m_count = 0;
}

MP4Box::MP4Box(MP4FileClass& file, const char* type)
    : m_File(file)
{
    SetType(type);
    m_unknownType = false;
    m_nStart = 0;
    m_nEnd = 0;
    m_largesizeMode = false;
    m_nSize = 0;
    m_pParentBox = NULL;
    m_nDepth = 0xFF;
}

MP4Box::~MP4Box()
{
    uint32_t i;

    for (i = 0; i < m_vChildBoxInfos.size(); i++) {
        delete m_vChildBoxInfos[i];
    }

    for (i = 0; i < m_vChildBoxs.size(); i++)
	{
		delete m_vChildBoxs[i];
    }
}

MP4Box* MP4Box::CreateBox( MP4FileClass &file, MP4Box* parent, const char* type )
{
    MP4Box* box = factory( file, parent, type );
	osAssert(box);
	return box;
}

// generate a skeletal self

void MP4Box::Generate()
{
    uint32_t i;

    // for all mandatory, single child Box types
	for (i = 0; i < m_vChildBoxInfos.size(); i++)
	{
		if (m_vChildBoxInfos[i]->m_mandatory && m_vChildBoxInfos[i]->m_onlyOne)
		{

            // create the mandatory, single child Box
            MP4Box* pChildBox =
				CreateBox(m_File, this, m_vChildBoxInfos[i]->m_name);

            AddChildBox(pChildBox);

            // and ask it to self generate
            pChildBox->Generate();
        }
    }
}

MP4Box* MP4Box::ReadBox(MP4FileClass& file, MP4Box* pParentBox)
{
    uint8_t hdrSize = 8;
    uint8_t extendedType[16];

    uint64_t pos = file.GetPosition();

    osLog(LOG_INFO, "\"%s\": pos = 0x%" PRIx64, file.GetFilename().c_str(), pos);

    uint64_t dataSize = file.ReadUInt32();

    char type[5];
    file.ReadBytes((uint8_t*)&type[0], 4);
    type[4] = '\0';

    // extended size
    const bool largesizeMode = (dataSize == 1);
    if (dataSize == 1) {
        dataSize = file.ReadUInt64();
        hdrSize += 8;
        file.Check64BitStatus(type);
    }

    // extended type
    if (BoxID(type) == BoxID("uuid")) {
        file.ReadBytes(extendedType, sizeof(extendedType));
        hdrSize += sizeof(extendedType);
    }

    if (dataSize == 0) {
        // extends to EOF
        dataSize = file.GetSize() - pos;
    }

    dataSize -= hdrSize;

    osLog(LOG_INFO, "\"%s\": type = \"%s\" data-size = %" PRIu64 " (0x%" PRIx64 ") hdr %u",
                  file.GetFilename().c_str(), type, dataSize, dataSize, hdrSize);

    if (pos + hdrSize + dataSize > pParentBox->GetEnd())
	{
        osLog(LOG_ERROR, "%s: \"%s\": invalid Box size, extends outside parent Box - skipping to end of \"%s\" \"%s\" %" PRIu64 " vs %" PRIu64,
                   __FUNCTION__, file.GetFilename().c_str(), pParentBox->GetType(), type,
                   pos + hdrSize + dataSize,
                   pParentBox->GetEnd());
        osLog(LOG_INFO, "\"%s\": parent %s (%" PRIu64 ") pos %" PRIu64 " hdr %d data %" PRIu64 " sum %" PRIu64,
                      file.GetFilename().c_str(), pParentBox->GetType(),
                      pParentBox->GetEnd(),
                      pos,
                      hdrSize,
                      dataSize,
                      pos + hdrSize + dataSize);

        // skip to end of Box
        dataSize = pParentBox->GetEnd() - pos - hdrSize;
    }

    MP4Box* pBox = CreateBox(file, pParentBox, type);
    pBox->SetStart(pos);
    pBox->SetEnd(pos + hdrSize + dataSize);
    pBox->SetLargesizeMode(largesizeMode);
    pBox->SetSize(dataSize);

    if (BoxID(type) == BoxID("uuid")) {
        pBox->SetExtendedType(extendedType);
    }
    if (pBox->IsUnknownType())
	{
        if (!IsReasonableType(pBox->GetType()))
		{
			osLog(LOG_WARN, "%s: \"%s\": Box type %s is suspect", __FUNCTION__, file.GetFilename().c_str(),
                         pBox->GetType());
        } else
		{
            osLog(LOG_INFO, "\"%s\": Info: Box type %s is unknown", file.GetFilename().c_str(),
                          pBox->GetType());
        }
    }

    pBox->SetParentBox(pParentBox);

	try {
		pBox->Read();
	}
	catch (Exception* x) {
		// delete Box and rethrow so we don't leak memory.
		delete pBox;	
		throw x;
	}


    return pBox;
}

bool MP4Box::IsReasonableType(const char* type)
{
    // Unwound this.  Pricy when called a lot.
    if( isalnum((unsigned char) type[0])) {
        if( isalnum((unsigned char) type[1])) {
            if( isalnum((unsigned char) type[2])) {
                if( isalnum((unsigned char) type[3]) || type[3] == ' ' ) {
                    return true;
                }
            }
        }
    }

    return false;
}

// generic read
void MP4Box::Read()
{
    if (BoxID(m_type) != 0 && m_nSize > 1000000)
	{
        osLog(LOG_INFO, "%s: \"%s\": %s Box size %" PRIu64 " is suspect", __FUNCTION__,
                     m_File.GetFilename().c_str(), m_type, m_size);
    }

    ReadProperties();

    // read child Boxs, if we expect there to be some
    if (m_vChildBoxInfos.size() > 0) {
        ReadChildBoxs();
    }

    Skip(); // to end of Box
}

void MP4Box::Skip()
{
	if (m_File.GetPosition() != m_nEnd)
	{
        osLog(LOG_INFO, "\"%s\": Skip: %" PRIu64 " bytes",
                      m_File.GetFilename().c_str(), m_end - m_File.GetPosition());
    }
    m_File.SetPosition(m_nEnd);
}

MP4Box* MP4Box::FindBox(const char* name)
{
    if (!IsMe(name)) {
        return NULL;
    }

    if (!IsRootBox()) {
        osLog(LOG_INFO, "\"%s\": FindBox: matched %s", 
                      GetFile().GetFilename().c_str(), name);

        name = MP4NameAfterFirst(name);

        // I'm the sought after Box
        if (name == NULL) {
            return this;
        }
    }

    // else it's one of my children
    return FindChildBox(name);
}

bool MP4Box::IsMe(const char* name)
{
    if (name == NULL) {
        return false;
    }

    // root Box always matches
    if (!strcmp(m_type, "")) {
        return true;
    }

    // check if our Box name is specified as the first component
    if (!MP4NameFirstMatches(m_type, name)) {
        return false;
    }

    return true;
}

MP4Box* MP4Box::FindChildBox(const char* name)
{
    uint32_t BoxIndex = 0;

    // get the index if we have one, e.g. moov.trak[2].mdia...
    (void)MP4NameFirstIndex(name, &BoxIndex);

    // need to get to the index'th child Box of the right type
    for (uint32_t i = 0; i < m_pChildBoxs.Size(); i++) {
        if (MP4NameFirstMatches(m_vChildBoxs[i]->GetType(), name)) {
            if (BoxIndex == 0) {
                // this is the one, ask it to match
				return m_vChildBoxs[i]->FindBox(name);
            }
            BoxIndex--;
        }
    }

    return NULL;
}

void MP4Box::ReadChildBoxs()
{
    bool this_is_udta = BoxID(m_type) == BoxID("udta");

    osLog(LOG_INFO, "\"%s\": of %s", m_File.GetFilename().c_str(), m_type[0] ? m_type : "root");
    for (uint64_t position = m_File.GetPosition(); position < m_nEnd; position = m_File.GetPosition())
	{
        // make sure that we have enough to read at least 8 bytes
        // size and type.
		if (m_nEnd - position < 2 * sizeof(uint32_t)) {
            // if we're reading udta, it's okay to have 4 bytes of 0
            if (this_is_udta && (m_nEnd - position) == sizeof(uint32_t)) {
                uint32_t mbz = m_File.ReadUInt32();
                if (mbz != 0) {
					osLog(LOG_WARN, "%s: \"%s\": In udta Box, end value is not zero %x", __FUNCTION__,
                                 m_File.GetFilename().c_str(), mbz);
                }
                continue;
            }
            // otherwise, output a warning, but don't care
			osLog(LOG_WARN, "%s: \"%s\": In %s Box, extra %" PRId64 " bytes at end of Box", __FUNCTION__,
                         m_File.GetFilename().c_str(), m_type, (m_end - position));
            for (uint64_t ix = 0; ix < m_nEnd - position; ix++) {
                (void)m_File.ReadUInt8();
            }
            continue;
        }
        MP4Box* pChildBox = MP4Box::ReadBox(m_File, this);

        AddChildBox(pChildBox);

        MP4BoxInfo* pChildBoxInfo = FindBoxInfo(pChildBox->GetType());

        // if child Box is of known type
        // but not expected here print warning
        if (pChildBoxInfo == NULL && !pChildBox->IsUnknownType())
		{
            osLog(LOG_INFO, "%s: \"%s\": In Box %s unexpected child Box %s", __FUNCTION__,
                          m_File.GetFilename().c_str(), GetType(), pChildBox->GetType());
        }

        // if child Boxs should have just one instance
        // and this is more than one, print warning
        if (pChildBoxInfo) {
            pChildBoxInfo->m_count++;

            if (pChildBoxInfo->m_onlyOne && pChildBoxInfo->m_count > 1) {
				osLog(LOG_WARN, "%s: \"%s\": In Box %s multiple child Boxs %s", __FUNCTION__,
                             m_File.GetFilename().c_str(), GetType(), pChildBox->GetType());
            }
        }

    }

    // if mandatory child Box doesn't exist, print warning
    uint32_t numBoxInfo = m_pChildBoxInfos.Size();
    for (uint32_t i = 0; i < numBoxInfo; i++) {
        if (m_pChildBoxInfos[i]->m_mandatory
                && m_pChildBoxInfos[i]->m_count == 0) {
            osLog(LOG_WARN, "%s: \"%s\": In Box %s missing child Box %s", __FUNCTION__,
                         m_File.GetFilename().c_str(), GetType(), m_pChildBoxInfos[i]->m_name);
        }
    }

    osLog(LOG_INFO, "\"%s\": finished %s", m_File.GetFilename().c_str(), m_type);
}

MP4BoxInfo* MP4Box::FindBoxInfo(const char* name)
{
	uint32_t numBoxInfo = m_vChildBoxInfos.size();
    for (uint32_t i = 0; i < numBoxInfo; i++) {
		if (BoxID(m_vChildBoxInfos[i]->m_name) == BoxID(name)) {
            return m_vChildBoxInfos[i];
        }
    }
    return NULL;
}

// generic write
void MP4Box::Write()
{
    BeginWrite();

    // WriteProperties();

    WriteChildBoxs();

    FinishWrite();
}

void MP4Box::Rewrite()
{
    if (!m_nEnd) {
        // This Box hasn't been written yet...
        return;
    }

    uint64_t fPos = m_File.GetPosition();
    m_File.SetPosition(GetStart());
    Write();
    m_File.SetPosition(fPos);
}

void MP4Box::BeginWrite(bool use64)
{
    m_nStart = m_File.GetPosition();
    //use64 = m_File.Use64Bits();
    if (use64) {
        m_File.WriteUInt32(1);
    } else {
        m_File.WriteUInt32(0);
    }
    m_File.WriteBytes((uint8_t*)&m_type[0], 4);
    if (use64) {
        m_File.WriteUInt64(0);
    }
    if (BoxID(m_type) == BoxID("uuid")) {
        m_File.WriteBytes(m_extendedType, sizeof(m_extendedType));
    }
}

void MP4Box::FinishWrite(bool use64)
{
    m_nEnd = m_File.GetPosition();
	m_nSize = (m_nEnd - m_nStart);

    osLog(LOG_INFO, "end: type %s %" PRIu64 " %" PRIu64 " size %" PRIu64,
                       m_type,m_start, m_end, m_size);
    //use64 = m_File.Use64Bits();
    if (use64) {
        m_File.SetPosition(m_start + 8);
        m_File.WriteUInt64(m_size);
    } else {
        ASSERT(m_size <= (uint64_t)0xFFFFFFFF);
        m_File.SetPosition(m_start);
        m_File.WriteUInt32(m_size);
    }
    m_File.SetPosition(m_end);

    // adjust size to just reflect data portion of Box
	m_nSize -= (use64 ? 16 : 8);
    if (BoxID(m_type) == BoxID("uuid")) {
		m_nSize -= sizeof(m_extendedType);
    }
}

void MP4Box::WriteChildBoxs()
{
    uint32_t size = m_vChildBoxs.size();
    for (uint32_t i = 0; i < size; i++) {
		m_vChildBoxs[i]->Write();
    }

    osLog(LOG_INFO, "Write: \"%s\": finished %s", m_File.GetFilename().c_str(), m_type);
}

void MP4Box::AddVersionAndFlags()
{
    //AddProperty(new MP4Integer8Property(*this, "version"));
    //AddProperty(new MP4Integer24Property(*this, "flags"));
}

void MP4Box::AddReserved(MP4Box& parentBox, const char* name, uint32_t size)
{
    //MP4BytesProperty* pReserved = new MP4BytesProperty(parentBox, name, size);
    //pReserved->SetReadOnly();
    //AddProperty(pReserved);
}

void MP4Box::ExpectChildBox(const char* name, bool mandatory, bool onlyOne)
{
    m_vChildBoxInfos.push_back(new MP4BoxInfo(name, mandatory, onlyOne));
}

uint8_t MP4Box::GetVersion()
{
    if (strcmp("version", m_pProperties[0]->GetName())) {
        return 0;
    }
    return ((MP4Integer8Property*)m_pProperties[0])->GetValue();
}

void MP4Box::SetVersion(uint8_t version)
{
    if (strcmp("version", m_pProperties[0]->GetName())) {
        return;
    }
    ((MP4Integer8Property*)m_pProperties[0])->SetValue(version);
}

uint32_t MP4Box::GetFlags()
{
    if (strcmp("flags", m_pProperties[1]->GetName())) {
        return 0;
    }
    return ((MP4Integer24Property*)m_pProperties[1])->GetValue();
}

void MP4Box::SetFlags(uint32_t flags)
{
    if (strcmp("flags", m_pProperties[1]->GetName())) {
        return;
    }
    ((MP4Integer24Property*)m_pProperties[1])->SetValue(flags);
}

void MP4Box::Dump(uint8_t indent, bool dumpImplicits)
{
    if ( m_type[0] != '\0' ) {
        // create list of ancestors
        list<string> tlist;
        for( MP4Box* Box = this; Box; Box = Box->GetParentBox() ) {
            const char* const type = Box->GetType();
            if( type && type[0] != '\0' )
                tlist.push_front( type );
        }

        // create contextual Box-name
        string can;
        const list<string>::iterator ie = tlist.end();
        for( list<string>::iterator it = tlist.begin(); it != ie; it++ )
            can += *it + '.';
        if( can.length() )
            can.resize( can.length() - 1 );

        osDump(indent, "\"%s\": type %s (%s)\n", GetFile().GetFilename().c_str(),
                 m_type, can.c_str() );
    }

    uint32_t i;
    uint32_t size;

    // dump our properties
        /* skip details of tables unless we're told to be verbose */
        //if (m_pProperties[i]->GetType() == TableProperty
        //        && (log.verbosity < MP4_LOG_VERBOSE2)) {
            //osDump(indent + 1, MP4_LOG_VERBOSE1, "\"%s\": <table entries suppressed>",
            //         GetFile().GetFilename().c_str() );

	this->DumpProperties(indent + 4, dumpImplicits);

    // dump our children
    size = m_vChildBoxs.size();
    for (i = 0; i < size; i++) {
		m_vChildBoxs[i]->Dump(indent + 2, dumpImplicits);
    }
}

void MP4Box::DumpDumpProperties(uint8_t indent, bool dumpImplicits)
{
	// Start offset
	osDump(indent, "Start offset: %d(0x%x)\n", m_nStart, m_nStart);
	// Full Size
	osDump(indent, "Full Size   : %d(0x%x)\n", m_nSize, m_nSize);
	// Type
	osDump(indent, "Box Type    : %s(0x%x)\n", m_type, STRTOINT32(m_type));
}

uint8_t MP4Box::GetDepth()
{
    if (m_nDepth < 0xFF) {
		return m_nDepth;
    }

    MP4Box *pBox = this;
	m_nDepth = 0;

    while ((pBox = pBox->GetParentBox()) != NULL) {
		m_nDepth++;
		osAssert(m_nDepth < 255);
    }
	return m_nDepth;
}

bool MP4Box::GetLargesizeMode()
{
    return m_largesizeMode;
}

void MP4Box::SetLargesizeMode( bool mode )
{
    m_largesizeMode = mode;
}

bool
MP4Box::descendsFrom( MP4Box* parent, const char* type )
{
    const uint32_t id = BoxID( type );
    for( MP4Box* Box = parent; Box; Box = Box->GetParentBox() ) {
        if( id == BoxID(Box->GetType()) )
            return true;
    }
    return false;
}

// UDTA child Box types to be constructed as MP4UdtaElementBox.
// List gleaned from QTFF 2007-09-04.
static const char* const UDTA_ELEMENTS[] = {
    "\xA9" "arg",
    "\xA9" "ark",
    "\xA9" "cok",
    "\xA9" "com",
    "\xA9" "cpy",
    "\xA9" "day",
    "\xA9" "dir",
    "\xA9" "ed1",
    "\xA9" "ed2",
    "\xA9" "ed3",
    "\xA9" "ed4",
    "\xA9" "ed5",
    "\xA9" "ed6",
    "\xA9" "ed7",
    "\xA9" "ed8",
    "\xA9" "ed9",
    "\xA9" "fmt",
    "\xA9" "inf",
    "\xA9" "isr",
    "\xA9" "lab",
    "\xA9" "lal",
    "\xA9" "mak",
    "\xA9" "nak",
    "\xA9" "nam",
    "\xA9" "pdk",
    "\xA9" "phg",
    "\xA9" "prd",
    "\xA9" "prf",
    "\xA9" "prk",
    "\xA9" "prl",
    "\xA9" "req",
    "\xA9" "snk",
    "\xA9" "snm",
    "\xA9" "src",
    "\xA9" "swf",
    "\xA9" "swk",
    "\xA9" "swr",
    "\xA9" "wrt",
    "Allf",
    "name",
    "LOOP",
    "ptv ",
    "SelO",
    "WLOC",
    NULL // must be last
};

MP4Box*
MP4Box::factory( MP4FileClass &file, MP4Box* parent, const char* type )
{
    // type may be NULL only in case of root-Box
    if( !type )
        return new MP4RootBox(file);

    // construct Boxs which are context-savvy
    if( parent ) {
        const char* const ptype = parent->GetType();

        if( descendsFrom( parent, "ilst" )) {
            if( BoxID( ptype ) == BoxID( "ilst" ))
                return new MP4ItemBox( file, type );

            if( BoxID( type ) == BoxID( "data" ))
                return new MP4DataBox(file);

            if( BoxID( ptype ) == BoxID( "----" )) {
                if( BoxID( type ) == BoxID( "mean" ))
                    return new MP4MeanBox(file);
                if( BoxID( type ) == BoxID( "name" ))
                    return new MP4NameBox(file);
            }
        }
        else if( BoxID( ptype ) == BoxID( "meta" )) {
            if( BoxID( type ) == BoxID( "hdlr" ))
                return new MP4ItmfHdlrBox(file);
        }
        else if( BoxID( ptype ) == BoxID( "udta" )) {
            if( BoxID( type ) == BoxID( "hnti" ))
                return new MP4HntiBox(file);
            if( BoxID( type ) == BoxID( "hinf" ))
                return new MP4HinfBox(file);
            for( const char* const* p = UDTA_ELEMENTS; *p; p++ )
                if( !strcmp( type, *p ))
                    return new MP4UdtaElementBox( file, type );
        }
    }

    // no-context construction (old-style)
    switch( (uint8_t)type[0] ) {
        case 'S':
            if( BoxID(type) == BoxID("SVQ3") )
                return new MP4VideoBox( file, type );
            if( BoxID(type) == BoxID("SMI ") )
                return new MP4SmiBox(file);
            break;

        case 'a':
            if( BoxID(type) == BoxID("avc1") )
                return new MP4Avc1Box(file);
            if( BoxID(type) == BoxID("ac-3") )
                return new MP4Ac3Box(file);
            if( BoxID(type) == BoxID("avcC") )
                return new MP4AvcCBox(file);
            if( BoxID(type) == BoxID("alis") )
                return new MP4UrlBox( file, type );
            if( BoxID(type) == BoxID("alaw") )
                return new MP4SoundBox( file, type );
            if( BoxID(type) == BoxID("alac") )
                return new MP4SoundBox( file, type );
            break;

        case 'c':
            if( BoxID(type) == BoxID("chap") )
                return new MP4TrefTypeBox( file, type );
            if( BoxID(type) == BoxID("chpl") )
                return new MP4ChplBox(file);
            if( BoxID(type) == BoxID("colr") )
                return new MP4ColrBox(file);
            break;

        case 'd':
            if( BoxID(type) == BoxID("d263") )
                return new MP4D263Box(file);
            if( BoxID(type) == BoxID("damr") )
                return new MP4DamrBox(file);
            if( BoxID(type) == BoxID("dref") )
                return new MP4DrefBox(file);
            if( BoxID(type) == BoxID("dpnd") )
                return new MP4TrefTypeBox( file, type );
            if( BoxID(type) == BoxID("dac3") )
                return new MP4DAc3Box(file);
            break;

        case 'e':
            if( BoxID(type) == BoxID("elst") )
                return new MP4ElstBox(file);
            if( BoxID(type) == BoxID("enca") )
                return new MP4EncaBox(file);
            if( BoxID(type) == BoxID("encv") )
                return new MP4EncvBox(file);
            break;

        case 'f':
            if( BoxID(type) == BoxID("free") )
                return new MP4FreeBox(file);
            if( BoxID(type) == BoxID("ftyp") )
                return new MP4FtypBox(file);
            if( BoxID(type) == BoxID("ftab") )
                return new MP4FtabBox(file);
            break;

        case 'g':
            if( BoxID(type) == BoxID("gmin") )
                return new MP4GminBox(file);
            break;

        case 'h':
            if( BoxID(type) == BoxID("hdlr") )
                return new MP4HdlrBox(file);
            if( BoxID(type) == BoxID("hint") )
                return new MP4TrefTypeBox( file, type );
            if( BoxID(type) == BoxID("h263") )
                return new MP4VideoBox( file, type );
            if( BoxID(type) == BoxID("href") )
                return new MP4HrefBox(file);
            break;

        case 'i':
            if( BoxID(type) == BoxID("ipir") )
                return new MP4TrefTypeBox( file, type );
            if( BoxID(type) == BoxID("ima4") )
                return new MP4SoundBox( file, type );
            break;

        case 'j':
            if( BoxID(type) == BoxID("jpeg") )
                return new MP4VideoBox(file, "jpeg");
            break;

        case 'm':
            if( BoxID(type) == BoxID("mdhd") )
                return new MP4MdhdBox(file);
            if( BoxID(type) == BoxID("mvhd") )
                return new MP4MvhdBox(file);
            if( BoxID(type) == BoxID("mdat") )
                return new MP4MdatBox(file);
            if( BoxID(type) == BoxID("mpod") )
                return new MP4TrefTypeBox( file, type );
            if( BoxID(type) == BoxID("mp4a") )
                return new MP4SoundBox( file, type );
            if( BoxID(type) == BoxID("mp4s") )
                return new MP4Mp4sBox(file);
            if( BoxID(type) == BoxID("mp4v") )
                return new MP4Mp4vBox(file);
            break;

        case 'n':
            if( BoxID(type) == BoxID("nmhd") )
                return new MP4NmhdBox(file);
            break;

        case 'o':
            if( BoxID(type) == BoxID("ohdr") )
                return new MP4OhdrBox(file);
            break;

        case 'p':
            if( BoxID(type) == BoxID("pasp") )
                return new MP4PaspBox(file);
            break;

        case 'r':
            if( BoxID(type) == BoxID("rtp ") )
                return new MP4RtpBox(file);
            if( BoxID(type) == BoxID("raw ") )
                return new MP4VideoBox( file, type );
            break;

        case 's':
            if( BoxID(type) == BoxID("s263") )
                return new MP4S263Box(file);
            if( BoxID(type) == BoxID("samr") )
                return new MP4AmrBox( file, type );
            if( BoxID(type) == BoxID("sawb") )
                return new MP4AmrBox( file, type );
            if( BoxID(type) == BoxID("sdtp") )
                return new MP4SdtpBox(file);
            if( BoxID(type) == BoxID("stbl") )
                return new MP4StblBox(file);
            if( BoxID(type) == BoxID("stsd") )
                return new MP4StsdBox(file);
            if( BoxID(type) == BoxID("stsz") )
                return new MP4StszBox(file);
            if( BoxID(type) == BoxID("stsc") )
                return new MP4StscBox(file);
            if( BoxID(type) == BoxID("stz2") )
                return new MP4Stz2Box(file);
            if( BoxID(type) == BoxID("stdp") )
                return new MP4StdpBox(file);
            if( BoxID(type) == BoxID("sdp ") )
                return new MP4SdpBox(file);
            if( BoxID(type) == BoxID("sync") )
                return new MP4TrefTypeBox( file, type );
            if( BoxID(type) == BoxID("skip") )
                return new MP4FreeBox( file, type );
            if (BoxID(type) == BoxID("sowt") )
                return new MP4SoundBox( file, type );
            break;

        case 't':
            if( BoxID(type) == BoxID("text") )
                return new MP4TextBox(file);
            if( BoxID(type) == BoxID("tx3g") )
                return new MP4Tx3gBox(file);
            if( BoxID(type) == BoxID("tkhd") )
                return new MP4TkhdBox(file);
            if( BoxID(type) == BoxID("tfhd") )
                return new MP4TfhdBox(file);
            if( BoxID(type) == BoxID("trun") )
                return new MP4TrunBox(file);

            if( BoxID(type) == BoxID("tsc2") )
                return new MP4Tsc2Box(file);

          
            if( BoxID(type) == BoxID("twos") )
                return new MP4SoundBox( file, type );
            break;

        case 'u':
            if( BoxID(type) == BoxID("udta") )
                return new MP4UdtaBox(file);
            if( BoxID(type) == BoxID("url ") )
                return new MP4UrlBox(file);
            if( BoxID(type) == BoxID("urn ") )
                return new MP4UrnBox(file);
            if( BoxID(type) == BoxID("ulaw") )
                return new MP4SoundBox( file, type );
            break;

        case 'v':
            if( BoxID(type) == BoxID("vmhd") )
                return new MP4VmhdBox(file);
            break;

        case 'y':
            if( BoxID(type) == BoxID("yuv2") )
                return new MP4VideoBox( file, type );
            break;

        default:
            break;
    }

    // default to MP4StandardBox implementation
    return new MP4StandardBox( file, type ); 
}

