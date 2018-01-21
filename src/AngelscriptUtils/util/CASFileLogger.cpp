#include <cassert>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>
#include <sstream>

#ifdef WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include "ASPlatform.h"

#include "CASFileLogger.h"

CASFileLogger::CASFileLogger( const char* pszFilename, const Flags_t flags )
	: m_File( nullptr, ::fclose )
{
	Open( pszFilename, flags );
}

bool CASFileLogger::Open( const char* pszFilename, const Flags_t flags )
{
	assert( pszFilename );

	Close();

	m_szFilename = pszFilename;

	m_Flags = flags;

	//Open when something is logged.
	if( UsesDatestampMode() )
		return true;

	//open it now.
	return OpenFile( m_szFilename.c_str(), UsesDatestampMode() );
}

void CASFileLogger::Close()
{
	if( m_File )
	{
		m_File.reset();
	}
}

void CASFileLogger::VLog( LogLevel_t logLevel, const char* pszFormat, va_list list )
{
	if( !UsesDatestampMode() )
	{
		if( !IsOpen() )
		{
			return;
		}
	}
	else
	{
		if( !OpenFile( m_szFilename.c_str(), true ) )
			return;
	}

	if( UsesTimestampMode() )
	{
		time_t currentTime;

		time( &currentTime );

		const tm localTime = *localtime( &currentTime );

		fprintf( m_File.get(), "%02d:%02d:%02d: ", localTime.tm_hour, localTime.tm_min, localTime.tm_sec );
	}

	if( ShouldOutputLogLevel() )
	{
		fprintf( m_File.get(), "%s (%d) ", ASLog::ToString( static_cast<ASLog::ASLog>( logLevel ) ), logLevel );
	}

	vfprintf( m_File.get(), pszFormat, list );

	//Always flush it if it's critical.
	if( logLevel <= ASLog::CRITICAL )
		fflush( m_File.get() );

	if( UsesDatestampMode() )
		Close();
}

bool CASFileLogger::OpenFile( const char* pszFilename, const bool bUseDatestamp )
{
	assert( pszFilename );

	Close();

	if( !( *pszFilename ) )
		return false;

	//Create the directory hierarchy.
	std::string szPath( pszFilename );

	for( auto& c : szPath )
	{
		if( c == '\\' )
			c = '/';
	}

	auto uiDelim = szPath.rfind( '/' );

	if( uiDelim != std::string::npos )
	{
		szPath.resize( uiDelim );

		//Make each directory.
		std::string::size_type uiIndex = 0;
		for( auto c : szPath )
		{
			if( c == '/' )
			{
				MakeDirectory( szPath.substr( 0, uiIndex ).c_str() );
			}

			++uiIndex;
		}

		//Make last directory.
		MakeDirectory( szPath.c_str() );
	}

	std::ostringstream stream;

	stream << pszFilename;

	if( bUseDatestamp )
	{
		time_t currentTime;

		time( &currentTime );

		const tm localTime = *localtime( &currentTime );

		stream << '-';

		stream.fill( '0' );

		stream.width( 4 );
		stream << ( localTime.tm_year + 1900 );

		stream.width( 0 );
		stream << '-';

		stream.width( 2 );
		stream << ( localTime.tm_mon + 1 );

		stream.width( 0 );
		stream << '-';

		stream.width( 2 );
		stream << localTime.tm_mday;

		stream.width( 0 );
	}

	stream << m_szExtension;

	m_File.reset( fopen( stream.str().c_str(), "a" ) );

	return IsOpen();
}
