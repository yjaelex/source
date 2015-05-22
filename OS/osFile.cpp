#include "osFile.h"


const File::Size __maxChunkSize = 1024*1024;


///////////////////////////////////////////////////////////////////////////////

File::File( std::string name_, Mode mode_, FileProvider* provider_ )
    : _name     ( name_ )
    , _isOpen   ( false )
    , _mode     ( mode_ )
    , _size     ( 0 )
    , _position ( 0 )
    , _provider ( provider_ ? *provider_ : standard() )
    , name      ( _name )
    , isOpen    ( _isOpen )
    , mode      ( _mode )
    , size      ( _size )
    , position  ( _position )
{
}

///////////////////////////////////////////////////////////////////////////////

File::~File()
{
    close();
    delete &_provider;
}

///////////////////////////////////////////////////////////////////////////////

void
File::setMode( Mode mode_ )
{
    _mode = mode_;
}

void
File::setName( const std::string& name_ )
{
    _name = name_;
}

///////////////////////////////////////////////////////////////////////////////

bool
File::open( std::string name_, Mode mode_ )
{
    if( _isOpen )
        return true;

    if( !name_.empty() )
        setName( name_ );
    if( mode_ != MODE_UNDEFINED )
        setMode( mode_ );

    if( _provider.open( _name, _mode ))
        return true;

    _size = _provider.getSize();
    //FileSystem::getFileSize( _name, _size );

    _isOpen = true;
    return false;
}

bool
File::seek( Size pos )
{
    if( !_isOpen )
        return true;

    if( _provider.seek( pos ))
        return true;
    _position = pos;
    return false;
}

bool
File::read( void* buffer, Size size, Size& nin, Size maxChunkSize )
{
    nin = 0;

    if( !_isOpen )
        return true;

    if( _provider.read( buffer, size, nin, maxChunkSize ))
        return true;

    _position += nin;
    if( _position > _size )
        _size = _position;

    return false;
}

bool
File::write( const void* buffer, Size size, Size& nout, Size maxChunkSize )
{
    nout = 0;

    if( !_isOpen )
        return true;

    if( _provider.write( buffer, size, nout, maxChunkSize ))
        return true;

    _position += nout;
    if( _position > _size )
        _size = _position;

    return false;
}

bool
File::close()
{
    if( !_isOpen )
        return false;
    if( _provider.close() )
        return true;

    _isOpen = false;
    return false;
}

#include <sys/stat.h>
static bool getFileSize(string path_, File::Size& size_)
{
	size_ = 0;
	struct stat buf;
	if (stat(path_.c_str(), &buf))
		return true;
	size_ = buf.st_size;
	return false;
}

int64_t File::getSize()
{
   int64_t retSize = 0;
   getFileSize( _name, retSize );
   return retSize;
}

