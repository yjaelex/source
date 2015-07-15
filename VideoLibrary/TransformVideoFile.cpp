#include "MP4FileClass.h"

bool transformVideoFile(string & urlStr)
{
    FileProvider * provider = NULL;
    uint64 startTime, endTime = 0;
    URLFile * urlFile = new URLFile(urlStr, FileProvider::MODE_READ);
    urlFile->setAsync(true);
    provider = (FileProvider*)urlFile;

    osDump(0, "Transform Video File start!\n");
    osDump(4, "URL: %s \n", urlStr.c_str());
    startTime = osQueryNanosecondTimer();
    osDump(4, "START TIME: %lld ns \n", startTime);
    MP4FileClass * mp4File = new MP4FileClass();
    mp4File->Open(urlStr.c_str(), File::MODE_READ, provider);
    mp4File->ReadFromFile();

    std::string fileName;
    std::size_t found = urlStr.find_last_of("/\\");
    fileName = urlStr.substr(found + 1);
    char h264FileName[64] = { 0 };
    sprintf_s(h264FileName, sizeof(h264FileName), "%s.264", fileName.c_str());
    mp4File->Extract264RawData(h264FileName);
    mp4File->Close();

    endTime = osQueryNanosecondTimer();
    osDump(4, "END TIME  : %lld ns. Total: %f s. \n", endTime, ((float)(endTime - startTime)) / 1000000000);

    delete mp4File;
    return true;
}
