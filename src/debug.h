/*______________________________________________________________________________________________

Filename: 	debug.h
Purpose:	debug utilities
Date:		5/26/2019
Author:		Jonathan E. Wright
______________________________________________________________________________________________*/

#pragma once

#include <cassert>
#include <cstdint>
#include <stdarg.h>

#include "charbuffer.h"

#if defined( DEBUG ) || defined( _DEBUG )
#define OTTER_ASSERT( x ) assert( x )
#define OT_GLERROR( x ) { if ( glGetError() != GL_NO_ERROR ) { debugout( x ); assert( false ); } }
#else
#define OTTER_ASSERT( x ) if ( !(x) ) { debugout( "ASSERT FAILED:" #x ); }
#define OT_GLERROR( x ) 
#endif

#define OTTER_ASSERT_FATAL( x ) if ( !(x) ) { assert( !"FATAL ERROR" ); exit( -1 ); }

#define OTTER_VERIFY( x ) if ( !( x ) ) { OTTER_ASSERT( x ); }

namespace otter {

void debugout( char const * fmt, ... );
void errorout( char const * fmt, ... );

//==============================================================
// cLogger
//==============================================================
class cLogger {
public:
	enum eFlags {
		FLAG_PRE_FORMAT = ( 1 << 0 )	// send this logger the log text before formatting var args.
	};

	cLogger( uint32_t const flags )
		: mFlags( flags ) {
	}

	// overload this to get text before the var args are formatted (and set FLAG_PRE_FORMAT)
	virtual void	PreLog( char const * fmt, va_list args );
	// overload this to get text after the var args are formated
	virtual void	PostLog( char const * msg );

	bool			IsPreLogger() const { return ( mFlags & FLAG_PRE_FORMAT ) != 0; }

protected:
	uint32_t	mFlags = 0;
};

//==============================================================
// cDebug
//==============================================================
class cDebug {
public:
	virtual ~cDebug() { }

	static cDebug *	Create();
	static void		Destroy( cDebug * & debug );

	static void RegisterLogger( cLogger * logger );
	static void UnregisterLogger( cLogger * logger );

protected:
	virtual	bool	Init() = 0;
	virtual void	Shutdown() = 0;
};


} // otter

