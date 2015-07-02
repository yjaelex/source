#ifndef OS_RINGBUFFER_H
#define OS_RINGBUFFER_H

#include "os.h"
#include "osThread.h"

#define MAX_RING_SIZE  16 

class OSRingBuffer
{
public:
    OSRingBuffer(){};
    virtual ~OSRingBuffer(){};
    virtual void drain() = 0;
    virtual bool write(OSThreadCommand *) = 0;
    virtual bool read(OSThreadCommand *) = 0;
};

class OSRingBufferWithLock : public OSRingBuffer
{
public:
    OSRingBufferWithLock();
    ~OSRingBufferWithLock();
    virtual bool write(OSThreadCommand *);
    virtual bool read(OSThreadCommand *);
    virtual void drain();
private:
    inline bool empty() { return (_wtpt == _rdpt); }

    inline bool full()
    {
        uint32 nextWriteSlot = (_wtpt + 1) % MAX_RING_SIZE;
        return (nextWriteSlot == _rdpt);
    }

    inline uint32 size(){ return (_wtpt + MAX_RING_SIZE - _rdpt) % MAX_RING_SIZE; }
    vector<OSThreadCommand> _ring;
    volatile uint32  _rdpt;
    volatile uint32  _wtpt;
    osLockHandle _lock;
};

#endif
