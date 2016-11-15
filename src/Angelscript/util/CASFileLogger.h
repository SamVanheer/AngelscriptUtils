#ifndef ANGELSCRIPT_UTIL_CASFILELOGGER_H
#define ANGELSCRIPT_UTIL_CASFILELOGGER_H

#include <cstdio>
#include <cstdint>
#include <memory>
#include <string>

#include "IASLogger.h"
#include "CASBaseLogger.h"

/**
*	Logs to a file.
*	This logger has 2 modes: datestamp and timestamp modes.
*	Datestamp mode will append a datestamp to the filename and will open the file whenever something is logged.
*	Timestamp mode will prepend a timestamp to each message.
*	Both modes can be activated separately and used together.
*	The default extension is ".log".
*	Should be heap allocated, override Release if you want to use a stack allocated version.
*/
class CASFileLogger : public CASBaseLogger<IASLogger>
{
public:
	using Flags_t = uint32_t;

	struct Flag
	{
		enum LogFlag : Flags_t
		{
			NONE			= 0,

			/**
			*	Append a datestamp to the filename.
			*/
			USE_DATESTAMP		= 1 << 0,

			/**
			*	Prepend a timestamp to each message.
			*/
			USE_TIMESTAMP		= 1 << 1,

			/**
			*	Log the log level as well.
			*/
			OUTPUT_LOG_LEVEL	= 1 << 2
		};
	};

public:
	/**
	*	Creates a log that writes to the given file.
	*	@param pszFilename Name of the file to write to. Should not include the extension.
	*	@param flags Flags.
	*/
	CASFileLogger( const char* pszFilename, const Flags_t flags = Flag::NONE );

	~CASFileLogger() = default;

	void Release() override
	{
		delete this;
	}

	/**
	*	@return The filename.
	*/
	const std::string& GetFilename() const { return m_szFilename; }

	/**
	*	@return The extension used for the file.
	*/
	const std::string& GetExtension() const { return m_szExtension; }

	/**
	*	Sets the extension used for the file.
	*/
	void SetExtension( std::string&& szExtension )
	{
		m_szExtension = std::move( szExtension );
	}

	/**
	*	If true, this logger uses datestamp mode.
	*/
	bool UsesDatestampMode() const { return ( m_Flags & Flag::USE_DATESTAMP ) != 0; }

	/**
	*	Sets whether datestamp mode should be used.
	*/
	void SetUseDatestampMode( const bool bUseDatestamp )
	{
		if( bUseDatestamp )
			m_Flags |= Flag::USE_DATESTAMP;
		else
			m_Flags &= ~Flag::USE_DATESTAMP;

		//Always close it so the filename is corrected when the next message is logged.
		Close();
	}

	/**
	*	If true, this logger uses timestamp mode.
	*/
	bool UsesTimestampMode() const { return ( m_Flags & Flag::USE_TIMESTAMP ) != 0; }

	/**
	*	Sets whether timestamp mode should be used.
	*/
	void SetUseTimestampMode( const bool bUseTimestamp )
	{
		if( bUseTimestamp )
			m_Flags |= Flag::USE_TIMESTAMP;
		else
			m_Flags &= ~Flag::USE_TIMESTAMP;
	}

	/**
	*	If true, this logger outputs log levels.
	*/
	bool ShouldOutputLogLevel() const { return ( m_Flags & Flag::OUTPUT_LOG_LEVEL ) != 0; }

	/**
	*	Sets whether the log level should be output.
	*/
	void SetOutputLogLevel( const bool bOutputLogLevel )
	{
		if( bOutputLogLevel )
			m_Flags |= Flag::OUTPUT_LOG_LEVEL;
		else
			m_Flags &= ~Flag::OUTPUT_LOG_LEVEL;
	}

	/**
	*	@return Whether the file is open.
	*/
	bool IsOpen() const { return !!m_File; }

	/**
	*	Opens the log. If the log is already open, it is closed and reopened.
	*	@param pszFilename Name of the file to write to. Should not include the extension.
	*	@param flags Flags.
	*	@return If in datestamp mode, returns true. Otherwise, returns whether the file was opened.
	*/
	bool Open( const char* pszFilename, const Flags_t flags = Flag::NONE );

	/**
	*	Closes the log if it is open.
	*/
	void Close();

	void VLog( LogLevel_t logLevel, const char* pszFormat, va_list list ) override;

protected:
	bool OpenFile( const char* pszFilename, const bool bUseDatestamp );

private:
	std::unique_ptr<FILE, int ( * )( FILE* )> m_File;

	std::string m_szFilename;
	std::string m_szExtension = ".log";

	Flags_t m_Flags = Flag::NONE;

private:
	CASFileLogger( const CASFileLogger& ) = delete;
	CASFileLogger& operator=( const CASFileLogger& ) = delete;
};

#endif //ANGELSCRIPT_UTIL_CASFILELOGGER_H
