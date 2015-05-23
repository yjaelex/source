

#include "AllMP4Box.h"





/*
 * This is used for the 4 bit sample size below.  We need the sampleCount
 * to be correct for the number of samples, but the table size needs to
 * be correct to read and write it.
 */

class MP4HalfSizeTableProperty : public MP4TableProperty
{
public:
    MP4HalfSizeTableProperty(MP4Box& parentBox, const char *name, MP4IntegerProperty *pCountProperty) :
            MP4TableProperty(parentBox, name, pCountProperty) {};

    // The count is half the actual size
    uint32_t GetCount() {
        return (m_pCountProperty->GetValue() + 1)/ 2;
    };
    void SetCount(uint32_t count) {
        m_pCountProperty->SetValue(count * 2);
    };
private:
    MP4HalfSizeTableProperty();
    MP4HalfSizeTableProperty ( const MP4HalfSizeTableProperty &src );
    MP4HalfSizeTableProperty &operator= ( const MP4HalfSizeTableProperty &src );
};


MP4Stz2Box::MP4Stz2Box(MP4FileClass &file)
        : MP4Box(file, "stz2")
{
    AddVersionAndFlags(); /* 0, 1 */

    AddReserved(*this, "reserved", 3); /* 2 */

    AddProperty( /* 3 */
        new MP4Integer8Property(*this, "fieldSize"));

    MP4Integer32Property* pCount =
        new MP4Integer32Property(*this, "sampleCount");
    AddProperty(pCount); /* 4 */

}

void MP4Stz2Box::Read()
{
    ReadProperties(0, 4);

    uint8_t fieldSize =
        ((MP4Integer8Property *)m_pProperties[3])->GetValue();
    //  uint32_t sampleCount = 0;

    MP4Integer32Property* pCount =
        (MP4Integer32Property *)m_pProperties[4];

    MP4TableProperty *pTable;
    if (fieldSize != 4) {
        pTable = new MP4TableProperty(*this, "entries", pCount);
    } else {
        // 4 bit field size uses a special table.
        pTable = new MP4HalfSizeTableProperty(*this, "entries", pCount);
    }

    AddProperty(pTable);

    if (fieldSize == 16) {
        pTable->AddProperty( /* 5/0 */
            new MP4Integer16Property(*this, "entrySize"));
    } else {
        pTable->AddProperty( /* 5/0 */
            new MP4Integer8Property(*this, "entrySize"));
    }

    ReadProperties(4);

    Skip(); // to end of atom
}



}
} // namespace mp4v2::impl
