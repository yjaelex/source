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

static bool transformSingleVideoFile(MP4FileClass * mp4File, URLFile * urlFile)
{
    uint64 startTime, endTime = 0;
    FileProvider* provider = (FileProvider*)urlFile;
    string urlStr = urlFile->name;

    startTime = osQueryNanosecondTimer();
    osLog(LOG_INFO, "Transform File: %s!", urlStr.c_str());
    
    try{
        mp4File->Open(urlStr.c_str(), File::MODE_READ, provider);
        mp4File->ReadFromFile();

        std::string fileName;
        std::size_t found = urlStr.find_last_of("/\\");
        fileName = urlStr.substr(found + 1);
        char h264FileName[64] = { 0 };
        sprintf_s(h264FileName, sizeof(h264FileName), "%s.264", fileName.c_str());
        mp4File->Extract264RawData(h264FileName);
        mp4File->Close();
    }
    catch (osException * excep){
        osLog(LOG_INFO, "Execptions: %s", excep->msg().c_str());
        if (urlFile->isAborted())
        {
            osLog(LOG_INFO, "%s downloading aborted!", urlStr.c_str());
        }
        mp4File->Close();
        return false;
    }

    endTime = osQueryNanosecondTimer();
    osLog(LOG_INFO, "Transform Done. Total time: %f s. File: %s! ", ((float)(endTime - startTime)) / 1000000000, urlStr.c_str());
    return true;
}

class TransformVideoRequest : public AbstractRequest
{
public:
    TransformVideoRequest(string & str)
    {
        m_urlStr = str;
        m_urlFile = NULL;
        m_mp4File = NULL;
    }
    ~TransformVideoRequest() {}

    // Abort the processing of the request.
    virtual void Abort()
    {
        AbstractRequest::Abort();

        AutoLock LockRequest(m_LockWorkerThread);
        if (m_mp4File && m_urlFile) m_urlFile->abort();
    }

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
            m_urlFile = new URLFile(m_urlStr, FileProvider::MODE_READ);
            m_urlFile->setAsync(true);
            m_mp4File = new MP4FileClass();

            transformSingleVideoFile(m_mp4File, m_urlFile);

            AutoLock LockRequest(m_LockWorkerThread);
            delete m_mp4File;
        }
        m_bReqProcessed = true;
        return 0;
    }

private:
    string          m_urlStr;
    URLFile*        m_urlFile;
    MP4FileClass*   m_mp4File;
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

void VideoLibraryJobMgr::WaitAll()
{
    uint32 count = m_JobReqList.size();
    for (uint32 i = 0; i < count; i++)
    {
        TransformVideoRequest * req = (TransformVideoRequest*)m_JobReqList.front();
        m_JobReqList.pop_front();

        req->WaitReqprocessed();
    }
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
    uint32 count = m_JobReqList.size();
    for (uint32 i = 0; i < count; i++)
    {
        TransformVideoRequest * req = (TransformVideoRequest*)m_JobReqList.front();
        if (req)
        {
            req->Abort();
            req->WaitReqprocessed();
            delete req;
        }
        m_JobReqList.pop_front();
    }
}
