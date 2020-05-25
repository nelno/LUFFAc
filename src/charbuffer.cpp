/*______________________________________________________________________________________________

Filename: 	charbuffer.h
Purpose:	Routines for manipulating char buffers.
Date:		5/28/2019
Author:		Jonathan E. Wright
______________________________________________________________________________________________*/

#include "charbuffer.h"

#include <string.h>
#include <cinttypes>

#include "debug.h"

namespace otter {

void AppendChar( char * dest, const size_t destSize, const char ch ) {
	if ( dest == nullptr || destSize <= 1 ) {
		return;
	}
	const size_t destLen = OT_STRLEN( dest );
	if ( destLen >= destSize - 1 ) {
		return;
	}
	dest[destLen] = ch;
	dest[destLen + 1] = '\0';
}

void AppendStr( char * dest, const size_t destSize, const char * src ) {
	if ( dest == nullptr || destSize <= 1 || src == nullptr ) {
		return;
	}

	const size_t destLen = OT_STRLEN( dest );
	OTTER_ASSERT( destLen > 0 );

	size_t desti = destLen;
	size_t srci = 0;
	while ( desti < destSize && src[srci] != '\0' ) {
		dest[desti++] = src[srci++];
	}
	if ( desti < destSize ) {
		dest[desti] = '\0';
	} else {
		dest[destSize - 1] = '\0';
	}
}

void AppendToPath( const char * path, const char * toAppend, const char separator, char * osPath, const size_t osPathSize ) {
	if ( osPath == nullptr || osPathSize < 1 ) {
		return;
	}
	if ( osPathSize == 1 ) {
		osPath[0] = '\0';
		return;
	}
	if ( path == nullptr ) {
		if ( toAppend != nullptr ) {
			OT_STRCPY( osPath, osPathSize, toAppend );
		}
		return;
	}

	OT_STRCPY( osPath, osPathSize, path );

	const size_t pathLen = OT_STRLEN( osPath );
	OTTER_ASSERT( pathLen > 0 );

	if ( osPath[pathLen - 1] != separator ) {
		AppendChar( osPath, osPathSize, separator );
	}
	AppendStr( osPath, osPathSize, toAppend );
}

bool ReplaceChar( const char * in, const char from, const char to, char * out, const size_t outSize ) {
	if ( in == nullptr ) {
		return false;
	}
	if ( out == nullptr || outSize < 1 ) {
		return false;
	}

	out[0] = '\0';
	size_t i = 0;
	for ( ; in[i] != '\0' && i < outSize; ++i ) {
		if ( in[i] == from ) {
			out[i] = to;
		} else {
			out[i] = in[i];
		}
	}
	if ( i < outSize ) {
		out[i] = '\0';
		return true;
	} else {
		OTTER_ASSERT( i < outSize );
		out[sizeof( out ) - 1] = '\0';
		return false;
	}
}

int FindChar( const char * str, const char ch ) {
	for ( int i = 0; str[i] != '\0'; ++i ) {
		if ( str[i] == ch ) {
			return i;
		}
	}
	return -1;
}

bool StripFilename( const char * path, const char pathSeparator, char * outPath, const size_t outPathSize ) {
	// go backwards until the first path separator
	const int32_t len = (int32_t)OT_STRLEN_S( path, outPathSize );
	for ( int32_t i = len - 1; i >= 0; --i ) {
		if ( path[i] == pathSeparator ) {
			OT_STRNCPY( outPath, outPathSize, path, i );
			outPath[i] = '\0';
			break;
		}
	}
	return false;
}

} // namespace otter