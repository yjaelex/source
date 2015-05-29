#include "AllMP4Box.h"

MP4StscBox::MP4StscBox(MP4FileClass &file)
        : MP4Box(file, "stsc")
{
	m_entryCount = 0;
	m_SampletoChunkTable = 0;

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
	MP4FullBox::ReadProperties();
    // Compute the firstSample values for later use
	uint32_t  =
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
