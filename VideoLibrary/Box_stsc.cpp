

#include "AllMP4Box.h"





MP4StscBox::MP4StscBox(MP4FileClass &file)
        : MP4Box(file, "stsc")
{
    AddVersionAndFlags();

    MP4Integer32Property* pCount =
        new MP4Integer32Property(*this, "entryCount");
    AddProperty(pCount);

    MP4TableProperty* pTable = new MP4TableProperty(*this, "entries", pCount);
    AddProperty(pTable);

    pTable->AddProperty(
        new MP4Integer32Property(pTable->GetParentBox(), "firstChunk"));
    pTable->AddProperty(
        new MP4Integer32Property(pTable->GetParentBox(), "samplesPerChunk"));
    pTable->AddProperty(
        new MP4Integer32Property(pTable->GetParentBox(), "sampleDescriptionIndex"));

    // As an optimization we add an implicit property to this table,
    // "firstSample" that corresponds to the first sample of the firstChunk
    MP4Integer32Property* pSample =
        new MP4Integer32Property(*this, "firstSample");
    pSample->SetImplicit();
    pTable->AddProperty(pSample);
}

void MP4StscBox::Read()
{
    // Read as usual
    MP4Box::Read();

    // Compute the firstSample values for later use
    uint32_t count =
        ((MP4Integer32Property*)m_pProperties[2])->GetValue();

    MP4Integer32Property* pFirstChunk = (MP4Integer32Property*)
                                        ((MP4TableProperty*)m_pProperties[3])->GetProperty(0);
    MP4Integer32Property* pSamplesPerChunk = (MP4Integer32Property*)
            ((MP4TableProperty*)m_pProperties[3])->GetProperty(1);
    MP4Integer32Property* pFirstSample = (MP4Integer32Property*)
                                         ((MP4TableProperty*)m_pProperties[3])->GetProperty(3);

    MP4SampleId sampleId = 1;

    for (uint32_t i = 0; i < count; i++) {
        pFirstSample->SetValue(sampleId, i);

        if (i < count - 1) {
            sampleId +=
                (pFirstChunk->GetValue(i+1) - pFirstChunk->GetValue(i))
                * pSamplesPerChunk->GetValue(i);
        }
    }
}



}
} // namespace mp4v2::impl
