#ifndef VP_TRANSFORM_VIDEO_FILE_H
#define VP_TRANSFORM_VIDEO_FILE_H

#include "vptypes.h"
#include <list>

typedef enum VideoJobType
{
    VP_JOB_UNKNOWN = 0,
    VP_JOB_TRANSFORM_FILES,

}VideoJobType;

#define MAX_JOB_THREAD_COUNT    4

class VideoLibraryJobMgr
{
public:
    VideoLibraryJobMgr()
    {
    }
    ~VideoLibraryJobMgr()
    {
        AbortAllJobs();
    }

    pvoid PushJobRequest(uint32 jobType, uint32 argc, char ** argv);

    void AbortJob(pvoid job);
    void AbortAllJobs();
    void WaitAll();

private:
    list <pvoid>                  m_JobReqList;
};



#endif


