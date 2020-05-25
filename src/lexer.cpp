#include "lexer.h"
#include <cstdarg>
#include <strings.h>

#include "debug.h"
#include "charbuffer.h"

namespace otter {

char const * tokenTypeNames[cToken::MAX_TOKEN_TYPE] = {
	"none", "punctuation", "name", "string", "number"
};

static const char * punctuationNames = "!~.,=?<>:;()[]{}|/\\+-*&^%$#@\"\'";

char const * cToken::GetTokenTypeName( eTokenType const tokenType ) {
	return tokenTypeNames[tokenType];
}

static cLexer::ePunctuation FindPunctuationType( const char * punctuationNames, char const ch ) {
	for ( int i = 0; punctuationNames[i] != '\0'; i++ ) {
		if ( ch == punctuationNames[ i ] ) {
			return static_cast< cLexer::ePunctuation >( i );
		}
	}
	return cLexer::PUNC_NONE;
}

char cLexer::GetPunctuationName( ePunctuation const punc ) {
	if ( punc <= PUNC_NONE || punc >= PUNC_MAX ) {
		return 0;
	}

	OTTER_ASSERT( punc < static_cast< int32_t >( mNumPunctuation ) );
	return mPunctuationNames[punc];
}

bool cLexer::cErrorHandler::Error( char const * fmt, ... ) {
	char errorMsg[512];

	va_list args;
	va_start( args, fmt );
	OT_VSNPRINTF( errorMsg, sizeof( errorMsg ), fmt, args );
	va_end( args );

	debugout( "ERROR: %s", errorMsg );

	return false;
}

void cLexer::BuildPunctuationTable( const char * punctuation ) {
	const size_t len = strlen( punctuation );
	mPunctuationNames = new char[len + 1];
	OT_STRCPY( const_cast< char * >( mPunctuationNames ), len + 1, punctuation );

	for ( int i = 0; i < PUNC_TABLE_SIZE; ++i ) {
		mPunctuationTable[i] = cLexer::PUNC_NONE;
	}
	for ( int i = 0; punctuation[i] != '\0'; ++i ) {
		const uint8_t ch = static_cast< uint8_t >( punctuation[i] );
		mPunctuationTable[ch] = FindPunctuationType( punctuation, ch );
	}
}

cLexer::ePunctuation cLexer::GetPunctuationType( const char ch ) const {
	return mPunctuationTable[static_cast< uint8_t >( ch )];
}

static cLexer::eCommentType GetCommentType( uint32_t const flags, char const curChar, char const nextChar ) {
	constexpr bool allowSingleLineComments = true;
	constexpr bool allowHashComments = true;

	if ( curChar != '/' ) {
		if ( allowHashComments && curChar == '#' ) {
			return cLexer::COMMENT_HASH;
		}
		return cLexer::COMMENT_NONE;
	} 

	if ( nextChar == '*' ) {
		return cLexer::COMMENT_BLOCK;
	} else if ( allowSingleLineComments && nextChar == '/' ) {
		return cLexer::COMMENT_LINE;
	}
	return cLexer::COMMENT_NONE;
}

static bool IsCommentEnd( uint32_t const flags, cLexer::eCommentType const commentType, char const curChar, char const nextChar ) {
	constexpr bool allowSingleLineComments = true;

	OTTER_ASSERT( commentType != cLexer::COMMENT_NONE );
	if ( commentType == cLexer::COMMENT_NONE ) {
		return true;
	} else if ( commentType == cLexer::COMMENT_LINE ) {
		OTTER_ASSERT( allowSingleLineComments );
		return curChar == '\n';
	} else if ( commentType == cLexer::COMMENT_HASH ) {
		OTTER_ASSERT( allowSingleLineComments );
		return curChar == '\n';
	} else if ( commentType == cLexer::COMMENT_BLOCK ) {
		return curChar == '*' && nextChar == '/';
	}
	return false;
}

/*
static eCommentType GetCommentType( uint32_t const flags, char const * cur ) {
	if ( *cur != '/' ) {
		if ( ( flags & cLexer::FLAG_HASH_COMMENTS ) != 0 && *cur == '#' ) {
			return COMMENT_HASH;
		}
		return COMMENT_NONE;
	} 
	char const * next = cur + 1;
	
	if ( *next == '*' ) {
		return COMMENT_BLOCK;
	} else if ( ( flags & cLexer::FLAG_NO_SINGLE_LINE_C_COMMENTS ) == 0 && *next == '/' ) {
		return COMMENT_LINE;
	}
	return COMMENT_NONE;
}

static bool IsCommentEnd( uint32_t const flags, eCommentType const commentType, char const * cur ) {
	OTTER_ASSERT( commentType != COMMENT_NONE );
	if ( commentType == COMMENT_NONE ) {
		return true;
	} else if ( commentType == COMMENT_LINE ) {
		OTTER_ASSERT( ( flags & cLexer::FLAG_NO_SINGLE_LINE_C_COMMENTS ) == 0 );
		return *cur == '\n';
	} else if ( commentType == COMMENT_HASH ) {
		OTTER_ASSERT( ( flags & cLexer::FLAG_HASH_COMMENTS ) != 0 );
		return *cur == '\n';
	} else if ( commentType == COMMENT_BLOCK ) {
		return *cur == '*' && *( cur + 1 ) == '/';
	}
	return false;
}
*/

cLexer::cLexer( char const * name, char const * text, size_t const len, const uint32_t flags ) 
	: mName( name )
	, mText( text )
	, mLen( len )
	, mFlags( flags )
	, mCur( text )
	, mEnd( text + len )
	, mLineStart( text )
	, mLine( 0 )
	, mCommentTypeFn( GetCommentType )
	, mCommentEndFn( IsCommentEnd ) {
	BuildPunctuationTable( punctuationNames );
}

cLexer::cLexer( char const * name, char const * text, size_t const len, const sInitParms & initParms ) 
	: mName( name )
	, mText( text )
	, mLen( len )
	, mFlags( initParms.mFlags )
	, mCur( text )
	, mEnd( text + len )
	, mLineStart( text )
	, mLine( 0 )
	, mFileIndex( initParms.mFileIndex )
	, mCommentTypeFn( initParms.mCommentTypeFn )
	, mCommentEndFn( initParms.mCommentEndFn ) {
	BuildPunctuationTable( initParms.mPunctuation == nullptr ? punctuationNames : initParms.mPunctuation );
}

cLexer::~cLexer() {
	delete [] mPunctuationNames;
	mPunctuationNames = nullptr;
}

static bool IsWhitespace( char const ch ) {
	return ( ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' );
}

static bool IsEndOfLine( char const ch ) {
	return ch == '\n';
}

bool cLexer::SkipWhitespace() {
	if ( !IsWhitespace( *mCur ) ) {
		return false;
	}
	while( !AtEnd() && IsWhitespace( *mCur ) ) {
		if ( IsEndOfLine( *mCur ) ) {
			mLine++;
			mLineStart = mCur + 1;
		}
		mCur++;
	}
	return true;
}

bool cLexer::SkipComments() {
	eCommentType ct = mCommentTypeFn( mFlags, *mCur, *( mCur + 1 ) );
	if ( ct == COMMENT_NONE ) {
		return false;
	}
	while ( !AtEnd() && !mCommentEndFn( mFlags, ct, *mCur, * (mCur + 1 ) ) ) {
		if ( IsEndOfLine( *mCur ) ) {
			mLine++;
			mLineStart = mCur + 1;
		}
		mCur++;
	}
	return true;
}

bool cLexer::Error( char const * fmt, ... ) const {
	char temp[ 512 ];

	va_list	argPtr;
	va_start( argPtr, fmt );
	OT_VSNPRINTF( temp, sizeof( temp ), fmt, argPtr );
	va_end( argPtr );

	debugout( "%s(%d) : %s\n", mName.c_str(), mLine, temp );

	// only store the first error
	if ( !HadError() ) {
		char errorStr[512];
		OT_SNPRINTF( errorStr, sizeof( errorStr ), "%s(%d) : %s", mName.c_str(), mLine, temp );
		mErrorMsg = errorStr;
	}

	return false;
}

static bool IsNumeric( char const ch ) {
	return ch >= '0' && ch <= '9';
}

static char PeekNext( char const * curPtr, char const * endPtr, char & next ) {
	if ( curPtr + 1 >= endPtr ) {
		return false;
	}
	char const * nextPtr = curPtr + 1;
	next = *nextPtr;
	return true;
}

bool cLexer::IsNegativeNumber( cLexer::ePunctuation const punc, char const * curPtr, char const * endPtr ) const {
	if ( punc != cLexer::PUNC_MINUS ) {
		return false;
	}
	char nextCh;
	if ( !PeekNext( curPtr, endPtr, nextCh ) ) {
		return false;
	}
	if ( IsNumeric( nextCh ) ) {
		return true;
	}
	cLexer::ePunctuation const nextPunc = GetPunctuationType( nextCh );
	return nextPunc == cLexer::PUNC_PERIOD;
}

static bool IsNumberStartingWithDecimalPoint( cLexer::ePunctuation const punc, char const * curPtr, char const * endPtr ) {
	if ( punc != cLexer::PUNC_PERIOD ) {
		return false;
	}
	char nextCh;
	if ( !PeekNext( curPtr, endPtr, nextCh ) ) {
		return false;
	}
	return IsNumeric( nextCh );
}

bool cLexer::NextToken( cToken & token ) {
	token.Clear();
	token.SetFileIndex( mFileIndex );

	bool skipped;
	do {
		skipped = false;
		skipped |= SkipWhitespace();
		skipped |= SkipComments();
	} while ( !AtEnd() && skipped );

	if ( AtEnd() ) {
		return false;
	}

	token.SetLine( mLine );

	ePunctuation punc = GetPunctuationType( *mCur );
	//char nextCh;
	//const bool nextValid = PeekNext( mCur, mEnd, nextCh );
	if ( punc == PUNC_QUOTE || punc == PUNC_SINGLE_QUOTE ) {
		const int32_t openQuoteLine = mLine;
		const size_t openQuoteOffset = mCur - mLineStart;
		mCur++;
		char const * tokenStart = mCur;
		ePunctuation closeQuote = PUNC_MAX;
		for ( ; ; ) {
			if ( AtEnd() ) {
				return Error( "No closing quote found for open quote on line %i, offset %i", openQuoteLine, openQuoteOffset );
			}
			if ( IsEndOfLine( *mCur ) ) {
				mLine++;
			} else {
				closeQuote = GetPunctuationType( *mCur );
			}
			mCur++;
			if ( closeQuote == punc ) {
				break;
			}
		}

		token.SetType( cToken::STRING );
		token.SetSubType( punc );
		token.SetText( tokenStart, mCur - tokenStart - 1 );
		token.SetLine( openQuoteLine );
		token.SetLineOffset( static_cast< int32_t >( tokenStart - mLineStart ) );
		token.SetOffset( tokenStart - mText );
		return true;
	} else if ( IsNumeric( *mCur ) || IsNegativeNumber( punc, mCur, mEnd ) || IsNumberStartingWithDecimalPoint( punc, mCur, mEnd ) ) {
		token.SetType( cToken::NUMBER );
		token.SetSubType( cToken::INTEGER );
		int numDecimals = 0;
		int numMinuses = 0;
		char const * tokenStart = mCur;

		while ( !AtEnd() ) {
			// end of line terminates a token
			// white space terminates a token
			// comment start terminates a token
			if ( IsEndOfLine( *mCur ) || IsWhitespace( *mCur ) || mCommentTypeFn( mFlags, *mCur, *( mCur + 1 ) ) != COMMENT_NONE ) {
				break;
			}
			// Handle numbers terminated by f or L (double). This can never happen for the first character because
			// in that case IsNumeric() is true.
			if ( *mCur == 'f' || *mCur == 'F' || *mCur == 'l' || *mCur == 'L' ) {
				// consume the character, but then were done
				mCur++;
				break;
			}
			// handle scientific notation format
			if ( *mCur == 'e' || *mCur == 'E' || *mCur == 'l' || *mCur == 'L' ) {
				mCur++;
				continue;
			}
			// any punctutation other than a decimal point or minus terminates a number token
			ePunctuation const curPunc = GetPunctuationType( *mCur );
			if ( curPunc != PUNC_NONE && curPunc != PUNC_PERIOD && curPunc != PUNC_MINUS ) {
				break;
			}
			if ( curPunc == PUNC_PERIOD ) {
				numDecimals++;
				token.SetSubType( cToken::FLOAT );
				if ( numDecimals > 1 ) {
					return Error( "Invalid number format" );
				}
			} else if ( curPunc == PUNC_MINUS ) {
				numMinuses++;
				if ( numMinuses > 1 ) {
					return Error( "Invalid number format" );
				}
			} else if ( !IsNumeric( *mCur ) ) {
				// anything non-numeric terminates a number
				break;
			}

			mCur++;
		}
		if ( mCur > tokenStart ) {
			token.SetText( tokenStart, mCur - tokenStart );
			token.SetLineOffset( static_cast< int32_t > ( tokenStart - mLineStart ) );
			token.SetOffset( tokenStart - mText );
			token.SetLine( mLine );
			return true;
		}
	}
	else if ( punc != PUNC_NONE ) {
		char const p = *mCur;
		mCur++; // consume the punctuation
		token.SetType( cToken::PUNCTUATION );
		token.SetSubType( punc );
		token.SetLineOffset( static_cast< int32_t > ( mCur - mLineStart ) );
		token.SetOffset( mCur - mText );
		token.SetLine( mLine );
		char text[2];
		text[0] = p;
		text[1] = '\0';
		token.SetText( text, 1 );
		return true;
	} else {
		char const * tokenStart = mCur;
		token.SetType( cToken::NAME );
		// read until end of name
		while ( !AtEnd() ) {
			// punctuation terminates
			const ePunctuation curPunc = GetPunctuationType( *mCur );
			if ( curPunc != PUNC_NONE && ( mFlags & FLAG_ALLOW_PUNCTUATION_IN_NAMES ) == 0 ) {
				break;
			}
			// end of line terminates a token
			// white space terminates a token
			// comment start terminates a token
			if ( IsEndOfLine( *mCur ) || IsWhitespace( *mCur ) || mCommentTypeFn( mFlags, *mCur, *( mCur + 1 ) ) != COMMENT_NONE ) {
				break;
			}

			// consume the character
			mCur++;
		}
		if ( mCur > tokenStart ) {
			token.SetText( tokenStart, mCur - tokenStart );
			token.SetLineOffset( static_cast< int32_t > ( tokenStart - mLineStart ) );
			token.SetOffset( tokenStart - mText );
			token.SetLine( mLine );
			return true;
		}
	}
	return false;
}

bool cLexer::NextToken( cToken & token, cErrorHandler & errorHandler ) {
	bool wasAtEnd = AtEnd();
	if ( wasAtEnd ) {
		token.Clear();
		return false;
	}
	bool const r = NextToken( token );
	if ( !r ) {
		if ( !wasAtEnd && AtEnd() ) {
			return false;	// simply reached the end of the file. Not an error.
		}
		errorHandler.Error( mErrorMsg.c_str() );
	}
	return r;
}

bool cLexer::PeekNextToken( cToken & token ) {
	char const * savedCur = mCur;
	char const * savedLineStart = mLineStart;
	int32_t savedLine = mLine;

	const bool r = NextToken( token );
	
	mCur = savedCur;
	mLineStart = savedLineStart;
	mLine = savedLine;

	return r;
}

bool cLexer::PeekNextToken( cToken & token, cErrorHandler & errorHandler ) {
	bool wasAtEnd = AtEnd();
	if ( wasAtEnd ) {
		token.Clear();
		return false;
	}
	if ( !PeekNextToken( token ) ) {
		if ( !wasAtEnd && AtEnd() ) {
			return false;	// simply reached the end of the file. Not an error.
		}
		return errorHandler.Error( "Failed to peek next token." );
	}
	return true;
}

bool cLexer::ExpectName( cToken & token ) {
	bool const r = NextToken( token );
	return ( r && token.GetType() == cToken::NAME );
}

bool cLexer::ExpectName( cToken & token, cErrorHandler & errorHandler ) {
	if ( !ExpectName( token ) ) {
		return errorHandler.Error( "Expected a name, got a %s.", cToken::GetTokenTypeName( token.GetType() ) );
	}
	return true;
}

bool cLexer::ExpectName( char const * str, cToken & token ) {
	bool const r = NextToken( token );
	if ( r && token.GetType() == cToken::NAME ) {
		if ( ( mFlags & FLAG_IGNORE_CASE ) != 0 ) {
			return ( strcasecmp( str, token.GetText() ) == 0 );
		} else {
			return ( strcmp( str, token.GetText() ) == 0 );
		}
	}
	return false;
}

bool cLexer::ExpectName( char const * str, cToken & token, cErrorHandler & errorHandler ) {
	if ( !ExpectName( str, token ) ) {
		return errorHandler.Error( "Expected name '%s', got '%s'.", str, token.GetText() );
	}
	return true;
}

bool cLexer::ExpectDouble( cToken & token, double & f ) {
	bool const r = NextToken( token );
	if ( !r || token.GetType() != cToken::NUMBER ) {
		return false;
	}
	f = strtod( token.GetText(), nullptr );
	return true;
}

bool cLexer::ExpectDouble( cToken & token, double & f, cErrorHandler & errorHandler ) {
	if ( !ExpectDouble( token, f ) ) {
		return errorHandler.Error( "Expected double, got '%s'.", cToken::GetTokenTypeName( token.GetType() ) );
	}
	return true;
}

bool cLexer::ExpectFloat( cToken & token, float & f ) {
	bool const r = NextToken( token );
	if ( !r || token.GetType() != cToken::NUMBER ) {
		return false;
	}
	f = strtof( token.GetText(), nullptr );
	return true;
}

bool cLexer::ExpectFloat( cToken & token, float & f, cErrorHandler & errorHandler ) {
	if ( !ExpectFloat( token, f ) ) {
		return errorHandler.Error( "Expected float, got '%s'.", cToken::GetTokenTypeName( token.GetType() ) );
	}
	return true;
}

bool cLexer::ExpectInteger( cToken & token, int32_t & i ) {
	bool const r = NextToken( token );
	if ( !r || token.GetType() != cToken::NUMBER || token.GetSubType() != cToken::INTEGER ) {
		return false;
	}
	i = strtol( token.GetText(), nullptr, 10 );
	return true;
}

bool cLexer::ExpectInteger( cToken & token, int32_t & i, cErrorHandler & errorHandler ) {
	if ( !ExpectInteger( token, i ) ) {
		return errorHandler.Error( "Expected integer, got %s.", cToken::GetTokenTypeName( token.GetType() ) );
	}
	return true;
}

bool cLexer::ExpectPunctuation( cToken & token, ePunctuation & punc ) {
	bool const r = NextToken( token );
	if ( !r || token.GetType() != cToken::PUNCTUATION ) {
		return false;
	}
	punc = static_cast< ePunctuation >( token.GetSubType() );
	return true;
}

bool cLexer::ExpectPunctuation( cToken & token, ePunctuation & punc, cErrorHandler & errorHandler ) {
	if ( !ExpectPunctuation( token, punc ) ) {
		return errorHandler.Error( "Expected punctuation, got %s.", cToken::GetTokenTypeName( token.GetType() ) );
	}
	return true;
}

bool cLexer::ExpectPunctuation( ePunctuation const punc, cToken & token ) {
	bool const r = NextToken( token );
	if ( !r || token.GetType() != cToken::PUNCTUATION ) {
		return false;
	}
	return ( token.GetSubType() == punc );
}

bool cLexer::ExpectPunctuation( ePunctuation const punc, cToken & token, cErrorHandler & errorHandler ) {
	if ( !ExpectPunctuation( punc, token ) ) {
		if ( token.GetType() == cToken::PUNCTUATION ) {
			ePunctuation p = static_cast< ePunctuation >( token.GetSubType() );
			return errorHandler.Error( "Expected punctuation '%c', got '%c'", GetPunctuationName( punc ), GetPunctuationName( p ) );
		} else {
			return errorHandler.Error( "Expected punctuation, got %s.", cToken::GetTokenTypeName( token.GetType() ) );
		}
	}
	return true;
}

} // namespace otter