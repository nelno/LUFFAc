/*______________________________________________________________________________________________

Filename: 	charbuffer.h
Purpose:	Routines for manipulating char buffers.
Date:		5/28/2019
Author:		Jonathan E. Wright
______________________________________________________________________________________________*/

#pragma once

#include <cstring>

#if defined( _MSC_VER )

#	define OT_STRCPY( dest, destSize, src ) strcpy_s( (dest), (destSize), (src) )
#	define OT_STRNCPY( dest, destSize, src, count ) strcpy_s( (dest), (destSize), (src), (count) )
#	define strcasecmp _stricmp
#	define OT_STRLEN( str ) strlen( (str) )
#	define OT_STRLEN_S( str, maxLen ) strlen_s( (str), (maxLen) )
#	define OT_VSNPRINTF( dest, destSize, fmt, arglist ) vsnprintf_s( (dest), (destSize), fmt, arglist )
#	define OT_SNPRINTF( dest, destSize, fmt, ... ) snprintf_s( (dest), (destSize), fmt, __VA_ARGS__ )

#elif defined( __GNUC__ )
	// FIXME: not safe from overwrites
#	define OT_STRCPY( dest, destSize, src ) strcpy( (dest), (src) )
#	define OT_STRNCPY( dest, destSize, src, count ) strncpy( (dest), (src), (count) ) // unsafe
#	define OT_STRLEN( str ) std::strlen( (str) )
#	define OT_STRLEN_S( str, maxLen ) strlen( (str) )
#	define OT_VSNPRINTF( dest, destSize, fmt, arglist ) vsnprintf( (dest), (destSize), fmt, arglist ) // unsafe
#	define OT_SNPRINTF( dest, destSize, fmt, ... ) snprintf( (dest), (destSize), fmt, __VA_ARGS__ )
#endif

namespace otter {

void AppendChar( char * dest, const size_t destSize, const char ch );
void AppendStr( char * dest, const size_t destSize, const char * src );
void AppendToPath( const char * path, const char * toAppend, const char pathSeparator, char * osPath, const size_t osPathSize );
bool ReplaceChar( const char * in, const char from, const char to, char * out, const size_t outSize );
int FindChar( const char * str, const char ch );
bool StripFilename( const char * path, const char pathSeparator, char * outPath, const size_t outPathSize );

} // namespace otter