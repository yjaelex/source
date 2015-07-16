#include "MP4FileClass.h"
#include "ThreadPool.h"
#include "TransformVideoFile.h"

static ThreadPool * g_ThreadPool = NULL;
static ThreadPool* getThreadPool()
{
    if (g_ThreadPool == NULL)
    {
        g_ThreadPool = new ThreadPool();
        g_ThreadPool->Create(MAX_JOB_THREAD_COUNT);
    }
    return g_ThreadPool;
}

static bool transformSingleVideoFile(string & urlStr)
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

class TransformVideoRequest : public AbstractRequest
{
public:
    TransformVideoRequest(string & str)
    {
        m_urlStr = str;
    }
    ~TransformVideoRequest() {}
    long Execute()
    {
        string strLog;
        if (IsAborted())
        {
            osLog(LOG_INFO, "\n***Request aborted***\n");
            return 0;
        }
        if (!m_urlStr.empty())
        {
            transformSingleVideoFile(m_urlStr);
        }

        m_bReqProcessed = true;
        return 0;
    }

private:
    string          m_urlStr;
};

pvoid VideoLibraryJobMgr::PushJobRequest(uint32 jobType, uint32 argc, char ** argv)
{
    if (argc < 1)   return NULL;
    if (VP_JOB_TRANSFORM_FILES == jobType)
    {
        string str;
        str.assign(argv[0]);
        TransformVideoRequest * req = new TransformVideoRequest(str);
        getThreadPool()->PostRequest(req);

        m_JobReqList.push_back((pvoid)req);
        return req;
    }

    return NULL;
}

void VideoLibraryJobMgr::AbortJob(pvoid job)
{
    AbstractRequest * req = (AbstractRequest*)job;

    if (req)
    {
        req->Abort();
    }
}
void VideoLibraryJobMgr::AbortAllJobs()
{

}
