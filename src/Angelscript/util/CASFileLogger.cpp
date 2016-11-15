#include <cassert>
#include <ctime>

#include <experimental/filesystem>

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

	//Create the directory hierarchy.
	std::experimental::filesystem::path path( pszFilename );

	path.remove_filename();

	std::error_code error;

	std::experimental::filesystem::create_directories( path, error );

	if( error )
		return false;

	//TODO: shouldn't use the MAX_PATH definition directly. - Solokiller
	char szBaseFilename[ 260 ];

	const char* pszFile;

	if( !bUseDatestamp )
	{
		pszFile = pszFilename;
	}
	else
	{
		time_t currentTime;

		time( &currentTime );

		const tm localTime = *localtime( &currentTime );

		const int iResult = snprintf( szBaseFilename, sizeof( szBaseFilename ), "%s-%04d-%02d-%02d", pszFilename, localTime.tm_year + 1900, localTime.tm_mon + 1, localTime.tm_mday );

		if( iResult < 0 || static_cast<size_t>( iResult ) >= sizeof( szBaseFilename ) )
			return false;

		pszFile = szBaseFilename;
	}

	//TODO
	char szFullFilename[ 260 ];

	const int iResult = snprintf( szFullFilename, sizeof( szFullFilename ), "%s%s", pszFile, m_szExtension.c_str() );

	if( iResult < 0 || static_cast<size_t>( iResult ) >= sizeof( szFullFilename ) )
		return false;

	m_File.reset( fopen( szFullFilename, "a" ) );

	return IsOpen();
}
