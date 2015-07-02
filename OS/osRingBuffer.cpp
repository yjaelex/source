#include "osRingBuffer.h"

OSRingBufferWithLock::OSRingBufferWithLock()
{
    _rdpt = 0;
    _wtpt = 0;
    _ring.resize(MAX_RING_SIZE);
    _lock = osLockCreate(NULL);
}
OSRingBufferWithLock::~OSRingBufferWithLock()
{
    osAssert(_rdpt == _wtpt);
    osLockDestroy(_lock);
}
void OSRingBufferWithLock::drain()
{
    while (1)
    {
        osAcquireLock(_lock);
        if (empty())
        {
            osReleaseLock(_lock);
            break;
        }
        else
        {
            osReleaseLock(_lock);
            osThreadSuspend(1);
        }
    }
}
bool OSRingBufferWithLock::read(OSThreadCommand * pCmd)
{
    bool ret = false;

    osAcquireLock(_lock);
    if (!empty())
    {
        *pCmd = _ring[_rdpt];
        ret = true;

        //fprintf(stderr, "get read slot = %d\n", _rdpt);
        //fflush(stderr);
        osAssert(_ring[_rdpt].op && _ring[_rdpt].data);

        _rdpt = (_rdpt + 1) % MAX_RING_SIZE;
    }

    osReleaseLock(_lock);

    return ret;
}

bool OSRingBufferWithLock::write(OSThreadCommand * pCmd)
{
    bool ret = NULL;
    while (1)
    {
        osAcquireLock(_lock);
        if (!full())
        {
            break;
        }
        else
        {
            osReleaseLock(_lock);
            osThreadSuspend(1);
        }
    }

    _ring[_wtpt] = *pCmd;

    //fprintf(stderr, "get write slot = %d\n", _wtpt);
    //fflush(stderr);
    osAssert(_ring[_wtpt].op && _ring[_wtpt].data);

    _wtpt = (_wtpt + 1) % MAX_RING_SIZE;
    osReleaseLock(_lock);

    return ret;
}

