
#ifndef _VP_MP4BOX_H
#define _VP_MP4BOX_H

#include "vptypes.h"
#include "MP4FileClass.h"
#include "vpUtil.h"

class MP4Box;
class MP4BoxInfo;
class MP4FileClass;

#define Required    true
#define Optional    false
#define OnlyOne     true
#define Many        false
#define Counted     true


typedef vector<MP4Box*>::iterator MP4BoxArrayIndex;
typedef vector<MP4BoxInfo*>::iterator MP4BoxInfoArrayIndex;

/* helper class */
class MP4BoxInfo {
public:
    MP4BoxInfo() {
        m_name = NULL;
    }
    MP4BoxInfo(const char* name, bool mandatory, bool onlyOne);

    const char* m_name;
    bool m_mandatory;
    bool m_onlyOne;
    uint32_t m_count;
};


class MP4Box
{
public:
    static MP4Box* ReadBox( MP4FileClass& file, MP4Box* pParentBox );
    static MP4Box* CreateBox( MP4FileClass& file, MP4Box* parent, const char* type );
    static bool IsReasonableType( const char* type );

private:
    static MP4Box* factory( MP4FileClass &file, MP4Box* parent, const char* type );
    static bool descendsFrom( MP4Box* parent, const char* type );

public:
    MP4Box(MP4FileClass& file, const char* type = NULL);
    virtual ~MP4Box();

    MP4FileClass& GetFile() {
        return m_File;
    };

    uint64_t GetStart() {
        return m_nStart;
    };
    void SetStart(uint64_t pos) {
        m_nStart = pos;
    };

    uint64_t GetEnd() {
        return m_nEnd;
    };
    void SetEnd(uint64_t pos) {
        m_nEnd = pos;
    };

    uint64_t GetSize() {
        return m_nSize;
    }
    void SetSize(uint64_t size) {
        m_nSize = size;
    }

    const char* GetType() {
        return m_type;
    };
    void SetType(const char* type) {
        if (type && *type != '\0') {
            // not needed ASSERT(strlen(type) == 4);
            memcpy(m_type, type, 4);
            m_type[4] = '\0';
        } else {
            memset(m_type, 0, 5);
        }
    }

    void GetExtendedType(uint8_t* pExtendedType) {
        memcpy(pExtendedType, m_extendedType, sizeof(m_extendedType));
    };
    void SetExtendedType(uint8_t* pExtendedType) {
        memcpy(m_extendedType, pExtendedType, sizeof(m_extendedType));
    };

    bool IsUnknownType() {
        return m_unknownType;
    }
    void SetUnknownType(bool unknownType = true)
	{
        m_unknownType = unknownType;
    }

    bool IsRootBox() {
        return m_type[0] == '\0';
    }

    MP4Box* GetParentBox() {
        return m_pParentBox;
    }
    void SetParentBox(MP4Box* pParentBox) {
        m_pParentBox = pParentBox;
    }

    void AddChildBox(MP4Box* pChildBox)
	{
        pChildBox->SetParentBox(this);
		m_vChildBoxs.push_back(pChildBox);
    }

    void InsertChildBox(MP4Box* pChildBox, uint32_t index)
	{
		MP4BoxArrayIndex beginIndex = m_vChildBoxs.begin();
        pChildBox->SetParentBox(this);
		m_vChildBoxs.insert(beginIndex + index, pChildBox);
    }

    void DeleteChildBox(MP4Box* pChildBox)
	{
		MP4BoxArrayIndex index = m_vChildBoxs.begin();
		for (uint32 i = 0; i < m_vChildBoxs.size(); i++)
		{
			if (m_vChildBoxs[i] == pChildBox)
			{
				m_vChildBoxs.erase(index + i);
                return;
            }
        }
    }

    uint32_t GetNumberOfChildBoxs()
	{
		return m_vChildBoxs.size();
    }

    MP4Box* GetChildBox(uint32_t index) {
		return m_vChildBoxs[index];
    }

    MP4Box* FindBox(const char* name);

    MP4Box* FindChildBox(const char* name);

    uint32_t GetFlags();
    void SetFlags(uint32_t flags);

    uint8_t GetDepth();

    void Skip();

    virtual void Generate();
    virtual void Read();
    virtual void BeginWrite(bool use64 = false);
    virtual void Write();
    virtual void Rewrite();
    virtual void FinishWrite(bool use64 = false);
    virtual void Dump(uint8_t indent, bool dumpImplicits);
	virtual void DumpProperties(uint8_t indent, bool dumpImplicits);
    virtual void ReadProperties();

    bool GetLargesizeMode();

protected:

    void AddVersionAndFlags();

    void AddReserved(MP4Box& parentBox, const char* name, uint32_t size);

    void ExpectChildBox(const char* name,
                         bool mandatory, bool onlyOne = true);

    MP4BoxInfo* FindBoxInfo(const char* name);

    bool IsMe(const char* name);

    void ReadChildBoxs();

    void WriteChildBoxs();

    uint8_t GetVersion();
    void SetVersion(uint8_t version);

    void SetLargesizeMode( bool );

protected:
    MP4FileClass&					m_File;

	// Start & End pos/offset of this Box
    uint64_t						m_nStart;
    uint64_t						m_nEnd;

    bool							m_largesizeMode;			// true if largesize mode
	// Data size; m_nSize = AllBoxSize - HeaderSize;
    uint64_t						m_nSize;

    char							m_type[5];
    bool							m_unknownType;
    uint8_t							m_extendedType[16];

    MP4Box*							m_pParentBox;
    uint8_t							m_nDepth;

    vector<MP4BoxInfo*>				m_vChildBoxInfos;
	vector<MP4Box*>					m_vChildBoxs;
private:
    MP4Box();
    MP4Box( const MP4Box &src );
    MP4Box &operator= ( const MP4Box &src );
};

inline uint32_t BoxID(const char* type) {
    return STRTOINT32(type);
}

// inverse BoxID - 32 bit id to string
inline void IDBox(uint32_t type, char *s) {
    INT32TOSTR(type, s);
}


#endif

