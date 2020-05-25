/*______________________________________________________________________________________________

Filename: 	debug.h
Purpose:	debug utilities
Date:		5/26/2019
Author:		Jonathan E. Wright
______________________________________________________________________________________________*/

#include "debug.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstdio>
#include <cstdarg>
#include <vector>

namespace otter {

class cDebugImpl : public cDebug {
public:
	static cDebugImpl *	mDebug;

	cDebugImpl();
	virtual ~cDebugImpl();

	bool	Init() override;
	void	Shutdown() override;

	void	RegisterLogger_Impl( cLogger * logger );
	void	UnregisterLogger_Impl( cLogger * logger );

	void	Log( char const * fmt, va_list args );

private:
	std::vector< cLogger* >	mPreLoggers;
	std::vector< cLogger* >	mPostLoggers;
};

cDebugImpl * cDebugImpl::mDebug = nullptr;

void cLogger::PreLog( char const *, va_list ) { 
}

void cLogger::PostLog( char const * ) { 
}

cDebug * cDebug::Create() {
	cDebugImpl * db = new cDebugImpl();
	if ( db != nullptr ) {
		if ( db->Init() ) {
			return db;
		}
		delete db;
		return nullptr;
	}
	return nullptr;
}
void cDebug::Destroy( cDebug * & debug ) {
	delete debug;
	debug = nullptr;
}

cDebugImpl::cDebugImpl() {

}
cDebugImpl::~cDebugImpl() {
}

bool cDebugImpl::Init() {
	mDebug = this;
	return true;
}

void cDebugImpl::Shutdown() {
	// manually unregister loggers before shutdown
	OTTER_ASSERT( mPreLoggers.size() == 0 );
	OTTER_ASSERT( mPostLoggers.size() == 0 );
}

void cDebugImpl::RegisterLogger_Impl( cLogger * logger ) {
	OTTER_ASSERT( logger != nullptr );
	if ( logger == nullptr ) {
		return;
	}
	if ( logger->IsPreLogger() ) {
		mPreLoggers.push_back( logger );
	} else {
		mPostLoggers.push_back( logger );
	}
}

void cDebugImpl::UnregisterLogger_Impl( cLogger * logger ) {
	if ( logger->IsPreLogger() ) {
		for ( size_t i = 0; i < mPreLoggers.size(); ++i ) {
			if ( mPreLoggers[i] == logger ) {
				mPreLoggers.erase( mPreLoggers.begin() + i );
				return;
			}
		}
	} else {
		for ( auto it = mPostLoggers.begin(); it != mPostLoggers.end(); ++it ) {
			if ( *it == logger ) {
				mPostLoggers.erase( it );
				return;
			}
		}
	}
}

void cDebugImpl::Log( char const * fmt, va_list args ) {
	char msg[8192];

	for ( size_t i = 0; i < mPreLoggers.size(); ++i ) {
		mPreLoggers[i]->PreLog( fmt, args );
	}

	std::vsnprintf( msg, sizeof( msg ), fmt, args );

	for ( size_t i = 0; i < mPostLoggers.size(); ++i ) {
		mPostLoggers[i]->PostLog( msg );
	}
}

void debugout( char const * fmt, ... ) {
	if ( cDebugImpl::mDebug != nullptr ) {
		va_list args;
		va_start( args, fmt );
		cDebugImpl::mDebug->Log( fmt, args );
		va_end( args );
	} else {
		va_list args;
		va_start( args, fmt );
		vprintf( fmt, args );
		va_end( args );
	}
}

void errorout( char const * fmt, ... ) {
	constexpr char const * ERROR_PREFIX = "ERROR: ";
	constexpr size_t ERROR_PREFIX_LEN = 7;

	size_t const fmtLen = OT_STRLEN( fmt );
	size_t const errorSize = fmtLen + OT_STRLEN( ERROR_PREFIX ) + 1;
	char errorFmt[errorSize];// = reinterpret_cast< char* >( _alloca( fmtLen + OT_STRLEN( ERROR_PREFIX ) + 1 ) );
	memcpy( errorFmt, ERROR_PREFIX, ERROR_PREFIX_LEN );
	memcpy( errorFmt + ERROR_PREFIX_LEN, fmt, fmtLen );
	errorFmt[ERROR_PREFIX_LEN + fmtLen] = '\0';

	if ( cDebugImpl::mDebug != nullptr ) {
		va_list args;
		va_start( args, fmt );
		cDebugImpl::mDebug->Log( errorFmt, args );
		va_end( args );
	} else {
		va_list args;
		va_start( args, fmt );
		vprintf( errorFmt, args );
		va_end( args );
	}
}

void cDebug::RegisterLogger( cLogger * logger ) {
	if ( cDebugImpl::mDebug != nullptr ) {
		cDebugImpl::mDebug->RegisterLogger_Impl( logger );
	}
}
void cDebug::UnregisterLogger( cLogger * logger ) {
	if ( cDebugImpl::mDebug != nullptr ) {
		cDebugImpl::mDebug->UnregisterLogger_Impl( logger );
	}
}

} // namespace otter