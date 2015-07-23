#ifndef _VP_OSFILE_H
#define _VP_OSFILE_H

#include "vptypes.h"
#include "osRingBuffer.h"
#include "osThread.h"
#include <curl/curl.h>

class FileProvider
{
public:
    static FileProvider& standard();

public:
    //! file operation mode flags
    enum Mode {
        MODE_UNDEFINED, //!< undefined
        MODE_READ,      //!< file may be read
        MODE_MODIFY,    //!< file may be read/written
        MODE_CREATE,    //!< file will be created/truncated for read/write
    };

    //! type used to represent all file sizes and offsets
    typedef int64_t Size;

public:
    virtual ~FileProvider() { }

    virtual bool open( std::string name, Mode mode ) = 0;
    virtual bool seek( Size pos ) = 0;
    virtual bool read( void* buffer, Size size, Size& nin, Size maxChunkSize ) = 0;
    virtual bool write( const void* buffer, Size size, Size& nout, Size maxChunkSize ) = 0;
    virtual bool close() = 0;

    virtual int64_t getSize() = 0;
protected:
    FileProvider() { }
};

///////////////////////////////////////////////////////////////////////////////
///
/// File implementation.
///
/// File objects model real filesystem files in a 1:1 releationship and always
/// treated as binary; there are no translations of text content performed.
///
/// The interface represents all sizes with a signed 64-bit value, thus
/// the limit to this interface is 63-bits of size, roughly 9.22 million TB.
///
///////////////////////////////////////////////////////////////////////////////

class File : public FileProvider
{
public:
    ///////////////////////////////////////////////////////////////////////////
    //!
    //! Constructor.
    //!
    //! A new file object is constructed but not opened.
    //!
    //! @param name filename of file object, or empty-string.
	//!  On Windows, this should be a UTF-8 encoded string.
    //!  On other platforms, it should be an 8-bit encoding that is
    //!  appropriate for the platform, locale, file system, etc.
    //!  (prefer to use UTF-8 when possible).
    //! @param mode bitmask specifying mode flags.
    //!     See #Mode for bit constants.
    //! @param provider a fileprovider instance. If NULL a standard file
    //!     provider will be used otherwise the supplied provider must be
    //!     new-allocated and will be delete'd via ~File().
    //!
    ///////////////////////////////////////////////////////////////////////////

    explicit File( std::string name = "", Mode mode = MODE_UNDEFINED, FileProvider* = NULL );

    ///////////////////////////////////////////////////////////////////////////
    //!
    //! Destructor.
    //!
    //! File object is destroyed. If the file is opened it is closed prior
    //! to destruction.
    //!
    ///////////////////////////////////////////////////////////////////////////

    ~File();

    ///////////////////////////////////////////////////////////////////////////
    //!
    //! Open file.
    //!
    //! @param name filename of file object, or empty-string to use #name.
	//!     On Windows, this should be a UTF-8 encoded string.
    //!     On other platforms, it should be an 8-bit encoding that is
    //!     appropriate for the platform, locale, file system, etc.
    //!     (prefer to use UTF-8 when possible).
    //!
    //! @return true on failure, false on success.
    //!
    ///////////////////////////////////////////////////////////////////////////

    bool open( std::string name = "", Mode mode = MODE_UNDEFINED );

    ///////////////////////////////////////////////////////////////////////////
    //!
    //! Closes file.
    //!
    //! If the file has not been opened or is not considered the
    //! owner of a filehandle, no action is taken.
    //!
    //! @return true on failure, false on success.
    //!
    ///////////////////////////////////////////////////////////////////////////

    bool close();

    ///////////////////////////////////////////////////////////////////////////
    //!
    //! Set current file position in bytes.
    //!
    //! @param pos new file position in bytes.
    //!
    //! @return true on failure, false on success.
    //!
    ///////////////////////////////////////////////////////////////////////////

    bool seek( Size pos );

    ///////////////////////////////////////////////////////////////////////////
    //!
    //! Binary stream read.
    //!
    //! The function reads up to a maximum <b>size</b> bytes from file,
    //! storing them in <b>buffer</b>. The number of bytes actually read are
    //! returned in <b>nin</b>.
    //!
    //! @param buffer storage for data read from file.
    //! @param size maximum number of bytes to read from file.
    //! @param nin output indicating number of bytes read from file.
    //! @param maxChunkSize maximum chunk size for reads issued to operating
    //!     system or 0 for default.
    //!
    //! @return true on failure, false on success.
    //!
    ///////////////////////////////////////////////////////////////////////////

    bool read( void* buffer, Size size, Size& nin, Size maxChunkSize = 0 );

    ///////////////////////////////////////////////////////////////////////////
    //!
    //! Binary stream write.
    //!
    //! The function writes up to a maximum <b>size</b> bytes from
    //! <b>buffer</b> to file. The number of bytes actually written are
    //! returned in <b>nout</b>.
    //!
    //! @param buffer data to be written out to file.
    //! @param size maximum number of bytes to read from file.
    //! @param nout output indicating number of bytes written to file.
    //! @param maxChunkSize maximum chunk size for writes issued to operating
    //!     system or 0 for default.
    //!
    //! @return true on failure, false on success.
    //!
    ///////////////////////////////////////////////////////////////////////////

    bool write( const void* buffer, Size size, Size& nout, Size maxChunkSize = 0 );

    int64_t getSize();


private:
    std::string   _name;
    bool          _isOpen;
    Mode          _mode;
    Size          _size;
    Size          _position;
    FileProvider& _provider;

public:
    const std::string& name;      //!< read-only: file pathname or empty-string if not applicable
    const bool&        isOpen;    //!< read-only: true if file is open
    const Mode&        mode;      //!< read-only: file mode
    const Size&        size;      //!< read-only: file size
    const Size&        position;  //!< read-only: file position

public:
    void setName( const std::string& name );
    void setMode( Mode mode );
};

typedef struct CurlData
{
    uint8 * pBuffer;             /* buffer to store cached data*/
    size_t  buffer_len;          /* currently allocated buffers length */
    size_t  buffer_pos;          /* end of data in buffer*/
}CurlData;

class URLFile : public FileProvider
{
public:
    explicit URLFile(std::string name = "", Mode mode = MODE_UNDEFINED, bool saveToFile = false);

    virtual ~URLFile();

    bool open(std::string name = "", Mode mode = MODE_UNDEFINED);

    bool close();

    bool seek(Size pos);

    bool read(void* buffer, Size size, Size& nin, Size maxChunkSize = 0);

    bool write(const void* buffer, Size size, Size& nout, Size maxChunkSize = 0);

    int64_t getSize();

    void setAsync(bool bAsync)
    {
        _bASync = bAsync;
    }

    void setSaveToFile(bool savetofile)
    {
        _bSaveToFile = savetofile;
    }

    void abort()
    {
        _bAborting = true;
    }

    bool isAborted()
    {
        return _bJobAborted && !_bAborting;
    }

private:
    std::string   _name;
    bool          _isOpen;
    Mode          _mode;
    Size          _size;
    Size          _position;

public:
    const std::string& name;      //!< read-only: file pathname or empty-string if not applicable
    const bool&        isOpen;    //!< read-only: true if file is open
    const Mode&        mode;      //!< read-only: file mode
    const Size&        size;      //!< read-only: file size
    const Size&        position;  //!< read-only: file position

    CURL*         _curl;
    bool          _bSaveToFile;
    File*         _file;
    bool          _bASync;           // sync io means read/write
    uint64        _err_code;
    osEventHandle _eventHandle;
    osLockHandle  _lock;
    bool          _threadActive;
    volatile size_t        _curlWriteSize;
    volatile bool          _curlJobDone;
    OSRingBufferWithLock    _ringBuffer;
    CurlData*     _pCurCurlData;
    bool          _bAborting;
    bool          _bJobAborted;

public:
    void setName(const std::string& name);
    void setMode(Mode mode);
};

#endif

