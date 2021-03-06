#include "osFile.h"

static const size_t __maxDataBufferSize = 2*1024 * 1024;
static const size_t __initDataBufferSize = 2 * 1024 * 1024;
static const size_t __allocDataBufferSize = 2 * 1024 * 1024;

URLFile::URLFile(std::string name_, Mode mode_, bool saveToFile)
    : _name(name_)
    , _isOpen(false)
    , _mode(mode_)
    , _size(0)
    , _position(0)
    , name(_name)
    , isOpen(_isOpen)
    , mode(_mode)
    , size(_size)
    , position(_position)
{
    _curl = NULL;;
    _bSaveToFile = saveToFile;
    _eventHandle = 0;
    _lock = 0;
    _bASync = false;
    _file = NULL;
    _curlJobDone = false;
    _curlWriteSize = 0;
    _err_code = 0;
    _bAborting = false;
}

URLFile::~URLFile()
{
    close();
    if (_pCurCurlData)
    {
        osFree(_pCurCurlData->pBuffer);
        delete _pCurCurlData;
        _pCurCurlData = NULL;
    }

}

/* curl calls this routine to get more data */
static size_t write_callback(char *buffer,
    size_t size,
    size_t nitems,
    void *userp)
{
    char *newbuff;
    size_t rembuff;

    URLFile *urlFile = (URLFile *)userp;
    size *= nitems;
    osAssert(urlFile);

    if (urlFile->_bSaveToFile)
    {
        FileProvider::Size outSize = 0;
        if (urlFile->_bASync)   osAcquireLock(urlFile->_lock);
        urlFile->_file->write(buffer, size, outSize);
        if (urlFile->_bASync)   osReleaseLock(urlFile->_lock);
        urlFile->_curlWriteSize += size;
        return outSize;
    }
    else
    {
        rembuff = urlFile->_pCurCurlData->buffer_len - urlFile->_pCurCurlData->buffer_pos; /* remaining space in buffer */

        if (size > rembuff)
        {
            /* not enough space in buffer */
            size_t newSize = urlFile->_pCurCurlData->buffer_len + (size - rembuff); // minimum size
            if (urlFile->getSize() != (-1))
            {
                newSize = urlFile->getSize();
            }
            else
            {
                newSize = urlFile->_pCurCurlData->buffer_len + __allocDataBufferSize;
            }
            if (urlFile->_bASync)   osAcquireLock(urlFile->_lock);
            newbuff = (char*)realloc(urlFile->_pCurCurlData->pBuffer, newSize);
            if (urlFile->_bASync)   osReleaseLock(urlFile->_lock);
            if (newbuff == NULL)
            {
                osLog(LOG_ERROR, "callback buffer grow failed\n");
                size = rembuff;
                return 0;
            }
            else
            {
                /* realloc succeeded increase buffer size*/
                urlFile->_pCurCurlData->buffer_len = newSize;
                urlFile->_pCurCurlData->pBuffer = (uint8*)newbuff;
            }
        }

        memcpy(&urlFile->_pCurCurlData->pBuffer[urlFile->_pCurCurlData->buffer_pos], buffer, size);
        urlFile->_pCurCurlData->buffer_pos += size;
        urlFile->_curlWriteSize += size;
    }

    //if (urlFile->_bASync && (urlFile->_pCurCurlData->buffer_len > __maxDataBufferSize))
    //{
    //    OSThreadCommand cmd;
    //    cmd.op = THREAD_CMD_COPY_DATA;
    //    cmd.data = (intp)urlFile->_pCurCurlData;
    //    cmd.callback = NULL;
    //    urlFile->_ringBuffer.write(&cmd);
    //    urlFile->_pCurCurlData = NULL;
    //    osEventSet(urlFile->_eventHandle);
    //}

    return size;
}

#define MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL     3

static int xferinfo(void *p,
    curl_off_t dltotal, curl_off_t dlnow,
    curl_off_t ultotal, curl_off_t ulnow)
{
    URLFile *urlFile = (URLFile *)p;
    CURL *curl = urlFile->_curl;
    double curtime = 0;

    if (urlFile->_bAborting)
    {
        return 1;
    }

    curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &curtime);

    /* under certain circumstances it may be desirable for certain functionality
    to only run every N seconds, in order to do this the transaction time can
    be used */
    if ((curtime - urlFile->_lastRunTime) >= MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL)\
    {
        urlFile->_lastRunTime = curtime;
        //fprintf(stderr, "TOTAL TIME: %f \r\n", curtime);
    }

    std::string fileName;
    std::size_t found = urlFile->name.find_last_of("/\\");
    fileName = urlFile->name.substr(found + 1);
    osLog(LOG_INFO,
        "File: %s"
        //"UP: %" CURL_FORMAT_CURL_OFF_T " of %" CURL_FORMAT_CURL_OFF_T
        "  DOWN: %" CURL_FORMAT_CURL_OFF_T " of %" CURL_FORMAT_CURL_OFF_T,
        fileName.c_str(), dlnow, dltotal);
    return 0;
}

/* for libcurl older than 7.32.0 (CURLOPT_PROGRESSFUNCTION) */
static int older_progress(void *p,
    double dltotal, double dlnow,
    double ultotal, double ulnow)
{
    return xferinfo(p,
        (curl_off_t)dltotal,
        (curl_off_t)dlnow,
        (curl_off_t)ultotal,
        (curl_off_t)ulnow);
}

static osThreadExitCode threadFunc(uintp param)
{
    OSThreadCommand cmd;
    bool readResult = false;
    URLFile * urlFile = reinterpret_cast<URLFile*>(param);
    urlFile->_threadActive = true;

    while (urlFile->_threadActive)
    {
 /*       if (osEventTimedWait(urlFile->_eventHandle, 1))
        {
            cmd.op = THREAD_CMD_NONE;
            cmd.data = 0;
            readResult = urlFile->_ringBuffer.read(&cmd);

            if (readResult)
            {
                urlFile->_threadStatus = IO_THREAD_STATUS_RUNNING;
                pThread->handleEvent(&cmd);
            }
            else
            {
                pThread->_threadStatus = IO_THREAD_STATUS_IDLE;
            }
        }
        else
        {
            pThread->_threadStatus = IO_THREAD_STATUS_IDLE;
        }*/

        //uint64 startTime, endTime = 0;
        //startTime = osQueryNanosecondTimer();
        //osDump(8, "*** CURL START TIME: %lld ns \n", startTime);

        uint32 http_code = 0;
        CURLcode retCode = curl_easy_perform(urlFile->_curl);
        curl_easy_getinfo(urlFile->_curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (retCode != CURLE_OK)
        {
            if ((CURLE_ABORTED_BY_CALLBACK == retCode) && urlFile->_bAborting)
            {
                urlFile->_bAborting = false;
                urlFile->setErrorCode(FILE_IO_ABORTED);
                urlFile->_curlJobDone = true;
                return OS_THREAD_SUCCESS;
            }

            osLog(LOG_ERROR, "CURL: write fail.");
            urlFile->_err_code = (retCode << 32) | http_code;
            urlFile->setErrorCode(FILE_IO_CURL_ERR);
            urlFile->_curlJobDone = true;
            return OS_THREAD_FAILED;
        }
        urlFile->_curlJobDone = true;

        //endTime = osQueryNanosecondTimer();
        //(8, "*** CURL END TIME  : %lld ns. Total: %f s. \n", endTime, ((float)(endTime - startTime)) / 1000000000);

        return OS_THREAD_SUCCESS;
    }
    return OS_THREAD_SUCCESS;
}


void
URLFile::setMode(Mode mode_)
{
    _mode = mode_;
}

void
URLFile::setName(const std::string& name_)
{
    _name = name_;
}

bool
URLFile::open(std::string name_, Mode mode_)
{
    if (_isOpen)
        return true;

    if (!name_.empty())
        setName(name_);
    if (mode_ != MODE_UNDEFINED)
        setMode(mode_);

    //// TODO: only support url read now.
    osAssert(_mode == MODE_READ);
    if (_bSaveToFile)
    {
        std::size_t found = name.find_last_of("/\\");
        std::string fileName = name.substr(found + 1);
        _file = new File(fileName, MODE_CREATE);
        _file->open();
    }
    else
    {
        _pCurCurlData = new CurlData();
        _pCurCurlData->buffer_len = __initDataBufferSize;
        _pCurCurlData->buffer_pos = 0;
        _pCurCurlData->pBuffer = (uint8*)osMalloc(__initDataBufferSize);
    }
    if (_bASync)
    {
        _eventHandle = osEventCreate();
        _lock = osLockCreate(NULL);
    }

    _curl = curl_easy_init();
    CURLcode retCode = curl_easy_setopt(_curl, CURLOPT_URL, _name.c_str());
    curl_easy_setopt(_curl, CURLOPT_FOLLOWLOCATION, 1);
    retCode = curl_easy_setopt(_curl, CURLOPT_WRITEDATA, this);
    retCode = curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, write_callback);
    osAssert(retCode == CURLE_OK);
#ifdef DEBUG_CURL
    curl_easy_setopt(_curl, CURLOPT_VERBOSE, 1L);
#endif

    double filesize = -0.1;
    curl_easy_setopt(_curl, CURLOPT_HEADER, 1);
    curl_easy_setopt(_curl,CURLOPT_NOBODY,1);
    retCode = curl_easy_perform(_curl);
    uint32 http_code = 0;
    if (retCode == CURLE_OK)
    {
        curl_easy_getinfo(_curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &filesize);
        curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code == 200 && retCode != CURLE_ABORTED_BY_CALLBACK)
        {
            //Succeeded
            _size = (filesize < 0.0001) ? -1 : (uint64_t)filesize;
        }
        else
        {
            //Failed
            osLog(LOG_ERROR, "CURL: open fail. Error Code: %d", http_code);
            _err_code = (retCode << 32) | http_code;
            _size = 0;
            return true;
        }

    }
    else
    {
        osLog(LOG_ERROR, "CURL: open fail.");
        _size = 0;
        return true;
    }

    _curlWriteSize = 0;
    if (!_bSaveToFile)
    {
        _pCurCurlData->buffer_pos = 0;
    }
    curl_easy_reset(_curl);
    retCode = curl_easy_setopt(_curl, CURLOPT_URL, _name.c_str());
    curl_easy_setopt(_curl, CURLOPT_FOLLOWLOCATION, 1);
    retCode = curl_easy_setopt(_curl, CURLOPT_WRITEDATA, this);
    retCode = curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, write_callback);
    osAssert(retCode == CURLE_OK);

    curl_easy_setopt(_curl, CURLOPT_PROGRESSFUNCTION, older_progress);
    /* pass the struct pointer into the progress function */
    curl_easy_setopt(_curl, CURLOPT_PROGRESSDATA, this);
#if LIBCURL_VERSION_NUM >= 0x072000
    /* xferinfo was introduced in 7.32.0, no earlier libcurl versions will
    compile as they won't have the symbols around.
    If built with a newer libcurl, but running with an older libcurl:
    curl_easy_setopt() will fail in run-time trying to set the new
    callback, making the older callback get used.
    New libcurls will prefer the new callback and instead use that one even
    if both callbacks are set. */
    curl_easy_setopt(_curl, CURLOPT_XFERINFOFUNCTION, xferinfo);
    /* pass the struct pointer into the xferinfo function, note that this is
    an alias to CURLOPT_PROGRESSDATA */
    curl_easy_setopt(_curl, CURLOPT_XFERINFODATA, this);
#endif
    curl_easy_setopt(_curl, CURLOPT_NOPROGRESS, 0L);

    if (_bASync)
    {
        // start curl write thread.
        osThreadCreate(threadFunc, (uintp)this);
    }
    _isOpen = true;
    return false;
}

bool
URLFile::write(const void* buffer, Size size, Size& nout, Size maxChunkSize)
{
    nout = 0;
    return true;
}


bool
URLFile::close()
{
    if (!_isOpen)
        return true;
    if (_file)
    {
        _file->close();
        delete _file;
    }

    if (_pCurCurlData)
    {
        if (_pCurCurlData->pBuffer)
        {
            osFree(_pCurCurlData->pBuffer);
        }
        delete _pCurCurlData;
        _pCurCurlData = NULL;
    }
    if (_bASync)
    {
        osEventDestroy(_eventHandle);
        osLockDestroy(_lock);
    }
    _curlWriteSize = 0;
    _curlJobDone = false;
    curl_easy_cleanup(_curl);
    _curl = NULL;
    _isOpen = false;
    return false;
}

bool
URLFile::read(void* buffer, Size size, Size& nin, Size maxChunkSize)
{
    nin = 0;

    if (!_isOpen)
        return true;

    // async read.
    if (_bASync && (!_curlJobDone))
    {
        while (_curlWriteSize < (size + _position))
        {
            osThreadSuspend(0);
            if (_curlJobDone)
            {
                break;
            }
        }
    }
    else
    {
        if (false == _curlJobDone)
        {
            CURLcode retCode = curl_easy_perform(_curl);
            if (retCode != CURLE_OK)
            {
                if ((CURLE_ABORTED_BY_CALLBACK == retCode) && _bAborting)
                {
                    _bAborting = false;
                    setErrorCode(FILE_IO_ABORTED);
                    _curlJobDone = true;
                    osLog(LOG_INFO, "CURL: job aborted.");
                    return true;
                }

                osLog(LOG_ERROR, "CURL: fail. Err code: %d", retCode);
                setErrorCode(FILE_IO_CURL_ERR);
                _err_code = (retCode << 32);
                _curlJobDone = true;
                return true;
            }
            _curlJobDone = true;
        }
    }

    if (isAborted())    return true;

    if (_bASync) osAcquireLock(_lock);
    if (_bSaveToFile)
    {
        osAssert(_file);
        _file->read(buffer, size, nin, maxChunkSize);
    }
    else
    {
        osAssert(_pCurCurlData);
        osAssert(_position <= _pCurCurlData->buffer_pos);
        nin = (_position + size > _pCurCurlData->buffer_pos) ?
            (_pCurCurlData->buffer_pos - _position) : size;
        memcpy(buffer, (void*)((uintp)_pCurCurlData->pBuffer + _position), (size_t)nin);
    }
    if (_bASync) osReleaseLock(_lock);

    _position += nin;
    if (_position > _size)
        _size = _position;

    return false;
}


bool
URLFile::seek(Size pos)
{
    if (!_isOpen)
        return true;

    if (_bASync && (!_curlJobDone))
    {
        while (_curlWriteSize < pos)
        {
            osThreadSuspend(0);
            if (_curlJobDone)
            {
                break;
            }
        }
    }
    else
    {
        if (false == _curlJobDone)
        {
            CURLcode retCode = curl_easy_perform(_curl);
            if (retCode != CURLE_OK)
            {
                if ((CURLE_ABORTED_BY_CALLBACK == retCode) && _bAborting)
                {
                    _bAborting = false;
                    setErrorCode(FILE_IO_ABORTED);
                    _curlJobDone = true;
                    osLog(LOG_INFO, "CURL: job aborted.");
                    return true;
                }

                osLog(LOG_ERROR, "CURL: fail.  Err code: %d", retCode);
                setErrorCode(FILE_IO_CURL_ERR);
                _curlJobDone = true;
                return true;
            }
            _curlJobDone = true;
        }
    }

    if (isAborted())    return true;

    if (_bSaveToFile)
    {
        osAssert(_file);
        _file->seek(pos);
    }
    _position = pos;
    return false;
}

int64_t URLFile::getSize()
{
    return _size;
}


