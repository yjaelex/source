#include "AVC.h"

void AVCDecoderConfigurationRecord::Read(File * file)
{
    m_configurationVersion = file->ReadUInt8();
    m_AVCProfileIndication = file->ReadUInt8();
    m_profile_compatibility = file->ReadUInt8();
    m_AVCLevelIndication = file->ReadUInt8();

    lengthSizeMinusOnePaddingBits = brb.readBits(6);
    lengthSizeMinusOne = brb.readBits(2);
    numberOfSequenceParameterSetsPaddingBits = brb.readBits(3);
    int numberOfSeuqenceParameterSets = brb.readBits(5);
    for (int i = 0; i < numberOfSeuqenceParameterSets; i++) {
        int sequenceParameterSetLength = file->readUInt16(content);

        byte[] sequenceParameterSetNALUnit = new byte[sequenceParameterSetLength];
        content.get(sequenceParameterSetNALUnit);
        sequenceParameterSets.add(sequenceParameterSetNALUnit);
    }
    long numberOfPictureParameterSets = file->ReadUInt8();
    for (int i = 0; i < numberOfPictureParameterSets; i++) {
        int pictureParameterSetLength = file->readUInt16(content);
        byte[] pictureParameterSetNALUnit = new byte[pictureParameterSetLength];
        content.get(pictureParameterSetNALUnit);
        pictureParameterSets.add(pictureParameterSetNALUnit);
    }
    if (content.remaining() < 4) {
        hasExts = false;
    }
    if (hasExts && (avcProfileIndication == 100 || avcProfileIndication == 110 || avcProfileIndication == 122 || avcProfileIndication == 144)) {
        // actually only some bits are interesting so masking with & x would be good but not all Mp4 creating tools set the reserved bits to 1.
        // So we need to store all bits
        brb = new BitReaderBuffer(content);
        chromaFormatPaddingBits = brb.readBits(6);
        chromaFormat = brb.readBits(2);
        bitDepthLumaMinus8PaddingBits = brb.readBits(5);
        bitDepthLumaMinus8 = brb.readBits(3);
        bitDepthChromaMinus8PaddingBits = brb.readBits(5);
        bitDepthChromaMinus8 = brb.readBits(3);
        long numOfSequenceParameterSetExt = file->ReadUInt8();
        for (int i = 0; i < numOfSequenceParameterSetExt; i++) {
            int sequenceParameterSetExtLength = file->readUInt16(content);
            byte[] sequenceParameterSetExtNALUnit = new byte[sequenceParameterSetExtLength];
            content.get(sequenceParameterSetExtNALUnit);
            sequenceParameterSetExts.add(sequenceParameterSetExtNALUnit);
        }
    }
    else {
        chromaFormat = -1;
        bitDepthLumaMinus8 = -1;
        bitDepthChromaMinus8 = -1;
    }
}
