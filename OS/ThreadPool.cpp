/**
 * @author :    Suresh
 */

#include "ThreadPool.h"
#include <sstream>
#include "os.h"

/** 
    * Log error description.
    * 
    * @param       lActiveReq_i - Count of active requests.
    * @param       wstrError_i  - Error message.
    */
void ThreadPool::LogError(const long lActiveReq_i, const std::string& strError_i)
{
    std::string wstrLog( strError_i );
    PrepareLog( lActiveReq_i, wstrLog );
    LogError( wstrLog );
}

void ThreadPool::LogInfo(const long lActiveReq_i, const std::string& wstrInfo_i)
{
    std::string strLog( wstrInfo_i );
    PrepareLog( lActiveReq_i, strLog );
    LogInfo( strLog );
}

void ThreadPool::LogError(const std::string& strError_i)
{
    osLog(LOG_ERROR, strError_i.c_str());
}

void ThreadPool::LogInfo(const std::string& strInfo_i)
{
    osLog(LOG_INFO, strInfo_i.c_str());
}


/** 
    * Log thread ID, Active thread count and last error.
    * 
    * @param       lActiveReq_i - Active thread count.
    * @param       wstrLog_io   - Error or information description
    */
void ThreadPool::PrepareLog(const long lActiveReq_i, std::string& strLog_io)
{
    std::stringstream strmLog;
    strmLog << "##TP## [TID=" << osGetCurrentThreadID()
                << "] [ACTIVE REQUEST=" << lActiveReq_i
                << "] [LAST ERROR=" << osGetLastError()
                << "] " << strLog_io.c_str() << + "]";
    strLog_io = strmLog.str();
}


/** 
    * Prepare error or information log.
    * 
    * @param       wstrLog_io - Log information
    */
void AbstractRequest::PrepareLog( std::string& strLog_io )
{
    std::stringstream strmLog;
    strmLog << "##RQ## [RID=" << GetRequestID()
                << "] [Desc=" << strLog_io.c_str() << + "]";
    strLog_io = strmLog.str();
}


/** 
    * Constructor
    */
ThreadPool::ThreadPool() : m_bDestroyed( false ),
                            m_usThreadCount( 0u ),
                            m_usSemaphoreCount( 0u ),
                            m_lActiveThread( 0u ),
                            m_usPendingReqCount( 0u ),
                            m_hSemaphore( NULL ),
                            m_phThreadList( NULL )
{
}


/** 
    * Destructor
    */
ThreadPool::~ThreadPool()
{
    if( NULL != m_phThreadList )
    {
        if( !Destroy())
        {
            LogError( "Destroy() failed" );
        }
    }
}


/** 
    * Create thread pool with specified number of threads.
    * 
    * @param       usThreadCount_i - Thread count.
    */
bool ThreadPool::Create( const unsigned short usThreadCount_i)
{
    // Check thread pool is initialized already.
    if( NULL != m_phThreadList )
    {
        LogError( "ThreadPool already created" );
        return false;
    }
    // Validate thread count.
    if( 0 == usThreadCount_i )
    {
        LogError( "Minimum allowed thread count is one" );
        return false;
    }
    if( usThreadCount_i > 64 )
    {
        LogError( "Maximum allowed thread count is 64" );
        return false;
    }
    LogInfo( "Thread pool creation requested" );

    // Initialize values.
    m_lActiveThread = 0u;
    m_usSemaphoreCount = 0u;
    m_usPendingReqCount = 0u;
    m_usThreadCount = usThreadCount_i;
    // Create semaphore for thread count management.
    m_hSemaphore = CreateSemaphore( NULL, 0, m_usThreadCount, NULL );
    if( NULL == m_hSemaphore )
    {
        LogError( "Semaphore creation failed" );
        m_usThreadCount = 0u;
        return false;
    }
    // Create worker threads and make pool active
    if( !AddThreads())
    {
        LogError( "Threads creation failed" );
        Destroy();
        return false;
    }
    SetDestroyFlag( false );
    LogInfo( "Thread pool created successfully" );
    return true;
}


/** 
    * Destroy thread pool.
    */
bool ThreadPool::Destroy()
{
        // Check whether thread pool already destroyed.
        if( NULL == m_phThreadList )
        {
            LogError( "ThreadPool is already destroyed or not created yet" );
            return false;
        }
        // Cancel all requests.
        CancelRequests();
        // Set destroyed flag to true for exiting threads.
        SetDestroyFlag( true );
        // Release remaining semaphores to exit thread.
        {
            AutoLock LockThread( m_LockWorkerThread );
            if( m_lActiveThread < m_usThreadCount )
            {
                if( NULL == ReleaseSemaphore( m_hSemaphore, m_usThreadCount - m_lActiveThread, NULL ))
                {
                    LogError( "Failed to release Semaphore" );
                    return false;
                }
            }
        }
        // Wait for destroy completion and clean the thread pool.
        if( !DestroyPool())
        {
            LogError( "Thread pool destruction failed" );
            return false;
        }
        LogInfo( "Thread Pool destroyed successfully" );
        return true;
}


/** 
    * Post request to thread pool for processing
    * 
    * @param       pRequest_io - Request to be processed.
    */
bool ThreadPool::PostRequest( AbstractRequest* pRequest_io )
{
        AutoLock LockThread( m_LockWorkerThread );
        if( NULL == m_phThreadList )
        {
            LogError( "ThreadPool is destroyed or not created yet" );
            return false;
        }
        m_RequestQueue.push_back( pRequest_io );
        if( m_usSemaphoreCount < m_usThreadCount )
        {
            // Thread available to process, so notify thread.
            if( !NotifyThread())
            {
                LogError( "NotifyThread failed" );
                // Request notification failed. Try after some time.
                m_usPendingReqCount++;
                return false;
            }
        }
        else
        {
            // Thread not available to process.
            m_usPendingReqCount++;
        }
        return true;
}


/** 
    * Pop request from queue for processing.
    * 
    * @param       RequestQueue_io  - Request queue.
    * @return      AbstractRequest* - Request pointer.
    */
AbstractRequest* ThreadPool::PopRequest( REQUEST_QUEUE& RequestQueue_io )
{
    AutoLock LockThread( m_LockWorkerThread );
    if( !RequestQueue_io.empty())
    {
        AbstractRequest* pRequest = RequestQueue_io.front();
        RequestQueue_io.remove( pRequest );
        return pRequest;
    }
    return 0;
}


/** 
    * Create specified number of threads. Initial status of threads will be waiting.
    */
bool ThreadPool::AddThreads()
{
    // Allocate memory for all threads.
    m_phThreadList = new osThreadHandle[m_usThreadCount];
    if( NULL == m_phThreadList )
    {
        LogError( "Memory allocation for thread handle failed" );
        return false;
    }
    // Create worker threads.
    for( unsigned short usIdx = 0u; usIdx < m_usThreadCount; usIdx++ )
    {
        // Create worker thread
        m_phThreadList[usIdx] = osThreadCreate(reinterpret_cast<osThreadProc>(ThreadPool::ThreadProc), (uintp)this);
        if( NULL == m_phThreadList[usIdx] )
        {
            LogError( "CreateThread failed" );
            return false;
        }
    }
    return true;
}


/** 
    * Add request to queue and release semaphore by one.
    */
bool ThreadPool::NotifyThread()
{
    AutoLock LockThread( m_LockWorkerThread );
    // Release semaphore by one to process this request.
    if( NULL == ReleaseSemaphore( m_hSemaphore, 1, NULL ))
    {
        LogError( "ReleaseSemaphore failed" );
        return false;
    }
    m_usSemaphoreCount++;
    return true;
}


/** 
    * Process request in queue.
    */
bool ThreadPool::ProcessRequests()
{
    bool bContinue( true );
    do
    {
        LogInfo( "Thread WAITING" );
        // Wait for request.
        if( !WaitForRequest())
        {
            LogError( "WaitForRequest() failed" );
            continue;
        }
        // Thread counter.
        AutoCounter Counter( m_lActiveThread, m_LockWorkerThread );
        LogInfo( "Thread ACTIVE" );
        // Check thread pool destroy request.
        if( IsDestroyed())
        {
            LogInfo( "Thread EXITING" );
            break;
        }
        // Get request from request queue.
        AbstractRequest* pRequest = PopRequest( m_RequestQueue );
        if( NULL == pRequest )
        {
            LogError( "PopRequest failed" );
            continue;
        }
        // Execute the request.
        long lReturn = pRequest->Execute();
        if( NULL != lReturn )
        {
            LogError( "Request execution failed" );
            continue;
        }
        // Check thread pool destroy request.
        if( IsDestroyed())
        {
            LogInfo( "Thread EXITING" );
            break;
        }
        AutoLock LockThread( m_LockWorkerThread );
        // Inform thread if any pending request.
        if( m_usPendingReqCount > 0 )
        {
            if( m_usSemaphoreCount < m_usThreadCount )
            {
                // Thread available to process, so notify thread.
                if( !NotifyThread())
                {
                    LogError( "NotifyThread failed" );
                    continue;
                }
                m_usPendingReqCount--;
            }
        }
    } while( bContinue );
    return true;
}


/** 
    * Wait for request queuing to thread pool.
    */
bool ThreadPool::WaitForRequest()
{
    // Wait released when requested queued.
    DWORD dwReturn = WaitForSingleObject( m_hSemaphore, INFINITE );
    if( WAIT_OBJECT_0 != dwReturn )
    {
        LogError( "WaitForSingleObject failed" );
        return false;
    }
    AutoLock LockThread( m_LockWorkerThread );
    m_usSemaphoreCount--;
    return true;
}


/** 
    * Destroy and clean up thread pool.
    */
bool ThreadPool::DestroyPool()
{
    // Wait for the exist of threads.
    DWORD dwReturn = WaitForMultipleObjects( m_usThreadCount, (HANDLE*)m_phThreadList, TRUE, INFINITE );
    if( WAIT_OBJECT_0 != dwReturn )
    {
        LogError( "WaitForMultipleObjects failed" );
        return false;
    }
    // Close all threads.
    for( USHORT uIdx = 0u; uIdx < m_usThreadCount; uIdx++ )
    {
        if( TRUE != CloseHandle( HANDLE(m_phThreadList[uIdx]) ))
        {
            LogError( "CloseHandle failed for threads" );
            return false;
        }
    }
    // Clear memory allocated for threads.
    delete[] m_phThreadList;
    m_phThreadList = 0;
    // Close the semaphore
    if( TRUE != CloseHandle( m_hSemaphore ))
    {
        LogError( "CloseHandle failed for semaphore" );
        return false;
    }
    // Clear request queue.
    m_RequestQueue.clear();
    return true;
}


/** 
    * Check for destroy request.
    */
inline bool ThreadPool::IsDestroyed()
{
    // Avoid synchronization issues if destroy requested after validation.
    AutoLock LockThread( m_LockWorkerThread );
    // During thread pool destruction all semaphores are released
    // to exit all threads.
    return m_bDestroyed;
}


/** 
    * Set destroy flag
    */
inline void ThreadPool::SetDestroyFlag( const bool bFlag_i )
{
    AutoLock LockThread( m_LockWorkerThread );
    m_bDestroyed = bFlag_i;
}


/** 
    * Cancel all processing request in pool.
    */
void ThreadPool::CancelRequests()
{
    // Avoid synchronization issues if destroy requested after validation.
    AutoLock LockThread(m_LockWorkerThread);
    LogInfo("Thread pool destroy requested");
    // Clear main queue.
    m_RequestQueue.clear();
}


/** 
    * worker thread procedure.
    * 
    * @param       pParam_i - ThreadPool instance.
    * @return      UINT      - Return 0 on success.
    */
osThreadExitCode ThreadPool::ThreadProc(uintp pParam_i)
{
    ThreadPool* pThreadPool = reinterpret_cast<ThreadPool*>( pParam_i );
    if( NULL == pThreadPool )
    {
        return OS_THREAD_FAILED;
    }
    if( !pThreadPool->ProcessRequests())
    {
        pThreadPool->LogError( "ProcessRequests() failed" );
        return OS_THREAD_FAILED;
    }
    return OS_THREAD_SUCCESS;
}


