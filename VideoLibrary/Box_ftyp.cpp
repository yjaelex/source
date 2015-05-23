

#include "AllMP4Box.h"

namespace mp4v2 { namespace impl {



MP4FtypBox::MP4FtypBox(MP4FileClass &file)
    : MP4Box( file, "ftyp" )
    , majorBrand       ( *new MP4StringProperty( *this, "majorBrand" ))
    , minorVersion     ( *new MP4Integer32Property( *this, "minorVersion" ))
    , compatibleBrands ( *new MP4StringProperty( *this, "compatibleBrands", false, false, true ))
{
    majorBrand.SetFixedLength( 4 );
    compatibleBrands.SetFixedLength( 4 );

    AddProperty( &majorBrand );
    AddProperty( &minorVersion );
    AddProperty( &compatibleBrands );
}

void MP4FtypBox::Generate()
{
    MP4Box::Generate();

    majorBrand.SetValue( "mp42" );
    minorVersion.SetValue( 0 );

    compatibleBrands.SetCount( 2 );
    compatibleBrands.SetValue( "mp42", 0 );
    compatibleBrands.SetValue( "isom", 1 );
}

void MP4FtypBox::Read()
{
    compatibleBrands.SetCount( (m_size - 8) / 4 ); // brands array fills rest of atom
    MP4Box::Read();
}



}} // namespace mp4v2::impl
