#ifndef _THREAD_POOL_MGR_H_
#define _THREAD_POOL_MGR_H_

#include "osThread.h"
#include <list>

class SyncObject
{

public:
    // Constructor
    SyncObject()
    {
        m_hLock = osLockCreate(NULL);
    }

    // Destructor
    ~SyncObject()
    {
        osLockDestroy(m_hLock);
    }

    // Lock critical section.
    bool Lock()
    {
        osAcquireLock(m_hLock);
        return true;
    }

    // Unlock critical section.
    bool Unlock()
    {
        osReleaseLock(m_hLock);
        return true;
    }

private:
    SyncObject( const SyncObject& );
    SyncObject& operator = ( const SyncObject& );

private:

    osLockHandle                    m_hLock;
};

/**
    * AutoLock - This class own synchronization object during construction and
    *            release the ownership during the destruction.
    */
class AutoLock
{

public:

    /** 
        * Parameterized constructor
        * 
        * @param       LockObj_i - Synchronization object.
        * @return      Nil
        * @exception   Nil
        * @see         Nil
        * @since       1.0
        */
    AutoLock( SyncObject& LockObj_i ) : m_pSyncObject( &LockObj_i )
    {
        if( NULL != m_pSyncObject )
        {
            m_pSyncObject->Lock();
        }
    }

    /** 
        * Destructor.
        * 
        * @param       Nil
        * @return      Nil
        * @exception   Nil
        * @see         Nil
        * @since       1.0
        */
    ~AutoLock()
    {
        if( NULL != m_pSyncObject )
        {
            m_pSyncObject->Unlock();
            m_pSyncObject = NULL;
        }
    }

private:
    SyncObject* m_pSyncObject;
};


/**
    * AbstractRequest - This is abstract base class for the request to be processed in thread pool.
    *                   and it is polymorphic. The users of the ThreadPool must create a class 
    *                   which derived from this and override Execute() function.
    */
class AbstractRequest
{

public:
    // Constructor
    AbstractRequest() : m_bAborted( false ), m_usRequestID( 0u ){}
    // Destructor
    virtual ~AbstractRequest(){}
    // Thread procedure to be override in derived class. This function should return if request aborted.
    // Abort request can check by calling IsAborted() function during time consuming operation.
    virtual long Execute() = 0;
    // Set request ID.
    void SetRequestID( unsigned short uRequestID_i )
    {
        AutoLock LockRequest( m_LockWorkerThread );
        m_usRequestID = uRequestID_i;
    }
    // Get request ID.
    unsigned short GetRequestID()
    {
        AutoLock LockRequest( m_LockWorkerThread );
        return m_usRequestID;
    }
    // Abort the processing of the request.
    void Abort()
    {
        AutoLock LockRequest( m_LockWorkerThread );
        m_bAborted = true;
    }
    // Clear abort flag for re-posting the same request.
    void ClearAbortFlag()
    {
        AutoLock LockRequest( m_LockWorkerThread );
        m_bAborted = false;
    }

protected:
    // Check for the abort request
    bool IsAborted()
    {
        AutoLock LockRequest( m_LockWorkerThread );
        return m_bAborted;
    }
    // Prepare error or information log.
    void PrepareLog( std::string& strLog_io );

protected:
    // Synchronization object for resource locking.
    SyncObject m_LockWorkerThread;

private:
    // Abort flag.
    bool m_bAborted;
    // Request Identifier.
    unsigned short m_usRequestID;

};

/**
    * AutoCounter - Increment and decrement counter
    */
class AutoCounter
{

public:
    // Constructor.
    AutoCounter( unsigned short& usCount_io,
                    SyncObject& Lock_io ) :
                    m_usCount( usCount_io ), m_LockThread( Lock_io )
    {
        AutoLock Lock( m_LockThread );
        m_usCount++;
    }

    // Destructor.
    ~AutoCounter()
    {
        AutoLock Lock( m_LockThread );
        m_usCount--;
    }

private:
    // Counter variable.
    unsigned short& m_usCount;
    // Synchronization object for resource locking.
    SyncObject& m_LockThread;
};


typedef std::list<AbstractRequest*> REQUEST_QUEUE;


/**
    * ThreadPool - This class create and destroy thread pool based on the request.
    *              The requested to be processed can be post to pool as derived object of 
    *              AbstractRequest. Also a class can be derive from Logger to error and
    *              information logging.
    */
class ThreadPool
{

public:
    // Constructor.
    ThreadPool();
    // Destructor.
    ~ThreadPool();

    // Create thread pool with specified number of threads.
    bool Create( const unsigned short usThreadCount_i);
    // Destroy the existing thread pool.
    bool Destroy();
    // Post request to thread pool for processing.
    bool PostRequest( AbstractRequest* pRequest_io );

private:
    AbstractRequest* PopRequest( REQUEST_QUEUE& RequestQueue_io );
    bool AddThreads();
    bool NotifyThread();
    bool ProcessRequests();
    bool WaitForRequest();
    bool DestroyPool();
    bool IsDestroyed();
    void SetDestroyFlag( const bool bFlag_i );
    void CancelRequests();

    // Log error description.
    void LogError(const long lActiveReq_i, const std::string& strError_i);
    // Log information.
    void LogInfo(const long lActiveReq_i, const std::string& strInfo_i);
    void LogError(const std::string& strError_i);
    void LogInfo(const std::string& strInfo_i);
    void PrepareLog(const long lActiveReq_i, std::string& wstrLog_io);

    static osThreadExitCode APIENTRY ThreadProc(uintp pParam_i);

private:
    ThreadPool( const ThreadPool& );
    ThreadPool& operator = ( const ThreadPool& );

private:
    // Used for thread pool destruction.
    bool                                    m_bDestroyed;
    // Hold thread count in the pool.
    unsigned short                          m_usThreadCount;
    // Released semaphore count.
    unsigned short                          m_usSemaphoreCount;
    // Active thread count.
    unsigned short                          m_lActiveThread;
    // Active thread count.
    unsigned short                          m_usPendingReqCount;
    // Manage active thread count in pool.
    osSemHandle                             m_hSemaphore;
    // Hold thread handles.
    osThreadHandle*                         m_phThreadList;
    // Request queue.
    REQUEST_QUEUE                           m_RequestQueue;
    // Synchronization object for resource locking.
    SyncObject                              m_LockWorkerThread;

};

#endif // #ifndef _THREAD_POOL_MGR_H_


