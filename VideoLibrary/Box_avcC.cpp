#include "AllMP4Box.h"





/*
 * SizeTableProperty is a special version of the MP4TableProperty -
 * the BytesProperty will need to set the value before it can read
 * from the file
 */
class SizeTableProperty : public MP4TableProperty
{
public:
    SizeTableProperty(MP4Box& parentBox, const char *name, MP4IntegerProperty *pCountProperty) :
            MP4TableProperty(parentBox, name, pCountProperty) {};
protected:
    void ReadEntry(MP4FileClass& file, uint32_t index) {
        // Each table has a size, followed by the length field
        // first, read the length
        m_pProperties[0]->Read(file, index);
        MP4IntegerProperty *pIntProp = (MP4IntegerProperty *)m_pProperties[0];
        // set the size in the bytes property
        MP4BytesProperty *pBytesProp = (MP4BytesProperty *)m_pProperties[1];
        pBytesProp->SetValueSize(pIntProp->GetValue(index), index);
        // And read the bytes
        m_pProperties[1]->Read(file, index);
    };
private:
    SizeTableProperty();
    SizeTableProperty ( const SizeTableProperty &src );
    SizeTableProperty &operator= ( const SizeTableProperty &src );
};

MP4AvcCBox::MP4AvcCBox(MP4FileClass &file)
        : MP4Box(file, "avcC")
{
    MP4BitfieldProperty *pCount;
    MP4TableProperty *pTable;

    AddProperty( new MP4Integer8Property(*this,"configurationVersion")); /* 0 */

    AddProperty( new MP4Integer8Property(*this,"AVCProfileIndication")); /* 1 */

    AddProperty( new MP4Integer8Property(*this,"profile_compatibility")); /* 2 */

    AddProperty( new MP4Integer8Property(*this,"AVCLevelIndication")); /* 3 */

    AddProperty( new MP4BitfieldProperty(*this,"reserved", 6)); /* 4 */
    AddProperty( new MP4BitfieldProperty(*this,"lengthSizeMinusOne", 2)); /* 5 */
    AddProperty( new MP4BitfieldProperty(*this,"reserved1", 3)); /* 6 */
    pCount = new MP4BitfieldProperty(*this,"numOfSequenceParameterSets", 5);
    AddProperty(pCount); /* 7 */

    pTable = new SizeTableProperty(*this,"sequenceEntries", pCount);
    AddProperty(pTable); /* 8 */
    pTable->AddProperty(new MP4Integer16Property(pTable->GetParentBox(),"sequenceParameterSetLength"));
    pTable->AddProperty(new MP4BytesProperty(pTable->GetParentBox(),"sequenceParameterSetNALUnit"));

    MP4Integer8Property *pCount2 = new MP4Integer8Property(*this,"numOfPictureParameterSets");
    AddProperty(pCount2); /* 9 */

    pTable = new SizeTableProperty(*this,"pictureEntries", pCount2);
    AddProperty(pTable); /* 10 */
    pTable->AddProperty(new MP4Integer16Property(pTable->GetParentBox(),"pictureParameterSetLength"));
    pTable->AddProperty(new MP4BytesProperty(pTable->GetParentBox(),"pictureParameterSetNALUnit"));
}

void MP4AvcCBox::Generate()
{
    MP4Box::Generate();

    ((MP4Integer8Property*)m_pProperties[0])->SetValue(1);

    m_pProperties[4]->SetReadOnly(false);
    ((MP4BitfieldProperty*)m_pProperties[4])->SetValue(0x3f);
    m_pProperties[4]->SetReadOnly(true);

    m_pProperties[6]->SetReadOnly(false);
    ((MP4BitfieldProperty*)m_pProperties[6])->SetValue(0x7);
    m_pProperties[6]->SetReadOnly(true);
#if 0
    // property reserved4 has non-zero fixed values
    static uint8_t reserved4[4] = {
        0x00, 0x18, 0xFF, 0xFF,
    };
    m_pProperties[7]->SetReadOnly(false);
    ((MP4BytesProperty*)m_pProperties[7])->
    SetValue(reserved4, sizeof(reserved4));
    m_pProperties[7]->SetReadOnly(true);
#endif
}

//
// Clone - clone my properties to destination atom
//
// this method simplifies duplicating avcC atom properties from
// source to destination file using a single API rather than
// having to copy each property.  This API encapsulates the object
// so the application layer need not concern with each property
// thereby isolating any future changes to atom properties.
//
// ----------------------------------------
// property   description
// ----------------------------------------
//
// 0    configurationVersion
// 1    AVCProfileIndication
// 2    profile_compatibility
// 3    AVCLevelIndication
// 4    reserved
// 5    lengthSizeMinusOne
// 6    reserved
// 7    number of SPS
// 8    SPS entries
// 9    number of PPS
// 10   PPS entries
//
//
void MP4AvcCBox::Clone(MP4AvcCBox *dstBox)
{

    MP4Property *dstProperty;
    MP4TableProperty *pTable;
    uint16_t i16;
    uint64_t i32;
    uint64_t i64;
    uint8_t *tmp;

    // source pointer Property I16
    MP4Integer16Property *spPI16;
    // source pointer Property Bytes
    MP4BytesProperty *spPB;

    // dest pointer Property I16
    MP4Integer16Property *dpPI16;
    // dest pointer Property Bytes
    MP4BytesProperty *dpPB;


    // start with defaults and reserved fields
    dstBox->Generate();

    // 0, 4, 6 are now generated from defaults
    // leaving 1, 2, 3, 5, 7, 8, 9, 10 to export

    dstProperty=dstBox->GetProperty(1);
    ((MP4Integer8Property *)dstProperty)->SetValue(
        ((MP4Integer8Property *)m_pProperties[1])->GetValue());

    dstProperty=dstBox->GetProperty(2);
    ((MP4Integer8Property *)dstProperty)->SetValue(
        ((MP4Integer8Property *)m_pProperties[2])->GetValue());

    dstProperty=dstBox->GetProperty(3);
    ((MP4Integer8Property *)dstProperty)->SetValue(
        ((MP4Integer8Property *)m_pProperties[3])->GetValue());

    dstProperty=dstBox->GetProperty(5);
    ((MP4BitfieldProperty *)dstProperty)->SetValue(
        ((MP4BitfieldProperty *)m_pProperties[5])->GetValue());

    //
    // 7 and 8 are related SPS (one set of sequence parameters)
    //
    // first the count bitfield
    //
    dstProperty=dstBox->GetProperty(7);
    dstProperty->SetReadOnly(false);
    ((MP4BitfieldProperty *)dstProperty)->SetValue(
        ((MP4BitfieldProperty *)m_pProperties[7])->GetValue());
    dstProperty->SetReadOnly(true);

    // next export SPS Length and NAL bytes */

    // first source pointers
    pTable = (MP4TableProperty *) m_pProperties[8];
    spPI16 = (MP4Integer16Property *)pTable->GetProperty(0);
    spPB = (MP4BytesProperty *)pTable->GetProperty(1);

    // now dest pointers
    dstProperty=dstBox->GetProperty(8);
    pTable = (MP4TableProperty *) dstProperty;
    dpPI16 = (MP4Integer16Property *)pTable->GetProperty(0);
    dpPB = (MP4BytesProperty *)pTable->GetProperty(1);

    // sps length
    i16 = spPI16->GetValue();
    i64 = i16;
    // FIXME - this leaves m_maxNumElements =2
    // but src atom m_maxNumElements is 1
    dpPI16->InsertValue(i64, 0);

    // export byte array
    i32 = i16;
    // copy bytes to local buffer
    tmp = (uint8_t *)MP4Malloc(i32);
    ASSERT(tmp != NULL);
    spPB->CopyValue(tmp, 0);
    // set element count
    dpPB->SetCount(1);
    // copy bytes
    dpPB->SetValue(tmp, i32, 0);
    MP4Free((void *)tmp);

    //
    // 9 and 10 are related PPS (one set of picture parameters)
    //
    // first the integer8 count
    //
    dstProperty=dstBox->GetProperty(9);
    dstProperty->SetReadOnly(false);
    ((MP4Integer8Property *)dstProperty)->SetValue(
        ((MP4Integer8Property *)m_pProperties[9])->GetValue());
    dstProperty->SetReadOnly(true);

    // next export PPS Length and NAL bytes */

    // first source pointers
    pTable = (MP4TableProperty *) m_pProperties[10];
    spPI16 = (MP4Integer16Property *)pTable->GetProperty(0);
    spPB = (MP4BytesProperty *)pTable->GetProperty(1);

    // now dest pointers
    dstProperty=dstBox->GetProperty(10);
    pTable = (MP4TableProperty *) dstProperty;
    dpPI16 = (MP4Integer16Property *)pTable->GetProperty(0);
    dpPB = (MP4BytesProperty *)pTable->GetProperty(1);

    // pps length
    i16 = spPI16->GetValue();
    i64 = i16;
    dpPI16->InsertValue(i64, 0);

    // export byte array
    i32 = i16;
    // copy bytes to local buffer
    tmp = (uint8_t *)MP4Malloc(i32);
    ASSERT(tmp != NULL);
    spPB->CopyValue(tmp, 0);
    // set element count
    dpPB->SetCount(1);
    // copy bytes
    dpPB->SetValue(tmp, i32, 0);
    MP4Free((void *)tmp);
}

