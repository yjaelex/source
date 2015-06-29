#ifndef OS_THREAD_WINDOWS_H
#define OS_THREAD_WINDOWS_H
#include <Windows.h>
#include "os.h"
#include "osThread.h"

typedef DWORD TID_t;
#define OS_THREAD_ID()                  GetCurrentThreadId()
#define OS_THREAD_LOCAL_ALLOC(KEY)      ((KEY) = (osThreadLocalKey)TlsAlloc())
#define OS_THREAD_LOCAL_FREE(KEY)       TlsFree((DWORD)(KEY))
#define OS_THREAD_LOCAL_SET(KEY, VALUE) TlsSetValue((DWORD)(KEY), (void*)(VALUE))
#define OS_THREAD_LOCAL_GET(KEY)        TlsGetValue((DWORD)(KEY))
#define OS_THREAD_LOCAL_RESERVE(INDEX, SUCCESS) ((SUCCESS) = TlsReserve((DWORD)(INDEX)) ? true : false)
#define OS_THREAD_LOCAL_RELEASE(INDEX) TlsRelease((DWORD)(INDEX))


osThreadHandle CONV
osThreadCreate(osThreadProc proc, uintp param, bool joinable)
{
    return reinterpret_cast<osThreadHandle>(CreateThread(NULL, 0,
        reinterpret_cast<LPTHREAD_START_ROUTINE>(proc), reinterpret_cast<LPVOID>(param), 0, NULL));
}

osThreadExitCode CONV
osThreadGetExitCode(osThreadHandle thread)
{
    DWORD exitCode;
    BOOL success;

    success = GetExitCodeThread(reinterpret_cast<HANDLE>(thread), &exitCode);
    if (success == FALSE)
    {
        // check if failure getting the exit-code is because the thread has not actually started yet
        exitCode = WaitForSingleObject(reinterpret_cast<HANDLE>(thread), 0);
        if (exitCode == WAIT_TIMEOUT)
            return OS_THREAD_STILL_ACTIVE;  // yes, the failure is because it is still starting up
        return OS_THREAD_FAILED;            // GetExitCodeThread failed somehow
    }

    if (exitCode == STILL_ACTIVE)
        return OS_THREAD_STILL_ACTIVE;
    else if (exitCode == OS_THREAD_SUCCESS)
        return OS_THREAD_SUCCESS;

    return OS_THREAD_FAILED;
}

void CONV
osThreadSuspend(uint32 msec)
{
    Sleep(msec);
}

bool CONV
osThreadSetAffinityMask(uintp mask)
{
    if (SetThreadAffinityMask(GetCurrentThread(), (DWORD_PTR)mask))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool CONV
osProcessSetAffinityMask(uintp mask)
{
    if (SetProcessAffinityMask(GetCurrentProcess(), (DWORD_PTR)mask))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool CONV
osProcessGetAffinityMask(uintp * mask)
{
    DWORD_PTR systemAffinity;
    if (GetProcessAffinityMask(GetCurrentProcess(), (PDWORD_PTR)mask, (PDWORD_PTR)&systemAffinity))
    {
        return true;
    }
    else
    {
        return false;
    }
}

int32 CONV
osThreadGetPriority(osThreadHandle thread)
{
    return GetThreadPriority((thread == NULL) ? GetCurrentThread() : (HANDLE)thread);
}

void CONV
osThreadSetPriority(osThreadHandle thread, int32 priority)
{
    SetThreadPriority((thread == NULL) ? GetCurrentThread() : (HANDLE)thread, priority);
}

osEventHandle CONV osEventCreate(bool manualReset)
{
    HANDLE eventHandle;

    eventHandle = CreateEvent(NULL, (BOOL)manualReset, FALSE, NULL);

    return (osEventHandle)eventHandle;
}

void CONV osEventDestroy(osEventHandle handle)
{
    BOOL success;

    osAssert(handle != NULL);

    success = CloseHandle((HANDLE)handle);
    osAssert(success);
}

EXTERN osEventHandle CONV
osEventDuplicate(osEventHandle orig)
{
    HANDLE dupHandle = 0;
    DuplicateHandle(GetCurrentProcess(), (HANDLE)orig, GetCurrentProcess(), &dupHandle, 0, true, DUPLICATE_SAME_ACCESS);

    return (osEventHandle)dupHandle;
}

void CONV osEventSet(osEventHandle handle)
{
    BOOL success;

    success = SetEvent((HANDLE)handle);
    osAssert(success);
}

void CONV osEventReset(osEventHandle handle)
{
    BOOL success;

    success = ResetEvent((HANDLE)handle);
    osAssert(success);
}

void CONV osEventWait(osEventHandle handle)
{
    DWORD ret;

    ret = WaitForSingleObject((HANDLE)handle, INFINITE);
    osAssert(ret == WAIT_OBJECT_0);
}

bool CONV osEventTimedWait(osEventHandle handle, uint32 timeout)
{
    DWORD ret;

    ret = WaitForSingleObject((HANDLE)handle, timeout);
    osAssert(ret == WAIT_OBJECT_0 || ret == WAIT_TIMEOUT);
    return (ret == WAIT_OBJECT_0);
}

osEventHandle CONV osFenceEventCreate(bool manualReset, int type, void *privateData)
{
    return osEventCreate(manualReset);
}

void CONV osFenceEventDestroy(osEventHandle handle, bool destroyNative)
{
    osEventDestroy(handle);
}

void* CONV osFenceEventPrivateData(osEventHandle handle)
{
    return (void *)NULL;
}

osEventHandle CONV osCreateNamedEvent(const char *semName, bool& isFirst)
{
    HANDLE eventHandle = CreateEvent(NULL, FALSE, FALSE, semName);
    isFirst = (GetLastError() == ERROR_ALREADY_EXISTS) ? false : true;
    return (osEventHandle)eventHandle;
}

void CONV osNamedEventDestroy(osEventHandle handle, const char *semName)
{
    osEventDestroy(handle);
}

osEventHandle CONV osSemaphoreCreate(uint32 initialCount, uint32 maxCount, const char *semName)
{
    HANDLE eventHandle = CreateSemaphore(NULL, initialCount, maxCount, semName);
    return (osEventHandle)eventHandle;
}

bool CONV osSemaphoreRelease(osEventHandle handle, uint32 releaseCount, long *previousCount)
{
    BOOL ret = ReleaseSemaphore((HANDLE)handle, releaseCount, (LPLONG)previousCount);
    return ret ? true : false;
}

osLibraryHandle CONV osOpenLibrary(const char *name)
{
    return (osLibraryHandle)LoadLibrary(name);
}

bool CONV osCloseLibrary(osLibraryHandle handle)
{
    return FreeLibrary((HMODULE)handle) == TRUE ? true : false;
}

void * CONV osGetProcAddress(osLibraryHandle handle, const char *name)
{
    return (void *)GetProcAddress((HMODULE)handle, name);
}

#endif
