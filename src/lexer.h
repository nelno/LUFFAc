#pragma once

#include <cstdint>
#include <string>
#include <strings.h>
#include "charbuffer.h"

namespace otter {

//==============================================================
// cToken
//==============================================================
class cToken {
public:
	enum eTokenType {
		NONE,
		PUNCTUATION,
		NAME,
		STRING,
		NUMBER,
		MAX_TOKEN_TYPE
	};

	enum eNumberSubType {
		INTEGER,
		FLOAT
	};

	cToken() 
		: mType( NONE )
		, mSubType( -1 )
		, mFileIndex( -1 )
		, mLine( -1 )
		, mLineOffset( -1 )
		, mOffset( 0 ) {
	}
	virtual ~cToken() {
	}

	virtual	const char *	GetText() const = 0;
	// set text without a zero-terminator
	virtual void			SetText( char const * text, size_t const len ) = 0;

	eTokenType				GetType() const { return mType; }
	void					SetType( eTokenType const type ) { mType = type; }
	
	int32_t					GetSubType() const { return mSubType; }
	void					SetSubType( int32_t const st ) { mSubType = st; }

	int32_t					GetFileIndex() const { return mFileIndex; }					
	void					SetFileIndex( int32_t const index ) { mFileIndex = index; }

	int32_t					GetLine() const { return mLine; }
	void					SetLine( int32_t const line ) { mLine = line; }

	int32_t					GetLineOffset() const { return mLineOffset; }
	void					SetLineOffset( const int32_t ofs ) { mLineOffset = ofs; }

	size_t					GetOffset() const { return mOffset; }
	void					SetOffset( size_t const ofs ) { mOffset = ofs; }

	void					Clear() { SetText( "", 0 ); }

	bool					IsValid() const { return mType != NONE; }

	static char const *		GetTokenTypeName( eTokenType const tokenType );
/*
	cToken( cToken const & other ) 
		: mType( NONE )
		, mSubType( -1 )
		, mLine( -1 )
		, mLineOffset( -1 )
		, mOffset( 0 ) {
		this->SetText( other.GetText(), OT_STRLEN( other.GetText() ) );
		this->SetType( other.GetType() );
		this->SetSubType( other.GetSubType() );
		this->SetLine( other.GetLine() );
		this->SetLineOffset( other.GetLineOffset() );
		this->SetOffset( other.GetOffset() );
	}

	cToken & 				operator = ( cToken const & rhs ) {
		if ( &rhs != this ) {
			this->SetText( rhs.GetText(), OT_STRLEN( rhs.GetText() ) );
			this->SetType( rhs.GetType() );
			this->SetSubType( rhs.GetSubType() );
			this->SetLine( rhs.GetLine() );
			this->SetLineOffset( rhs.GetLineOffset() );
			this->SetOffset( rhs.GetOffset() );
		}
		return *this;
	}
*/

private:
	eTokenType	mType;
	int32_t		mSubType;
	int32_t		mFileIndex;		// an index into a list of files
	int32_t		mLine;
	int32_t		mLineOffset;	// offset on the line
	size_t		mOffset;		// offset in the file
};

//==============================================================
// cTokenStatic
//==============================================================
template< size_t len_ >
class cTokenStatic : public cToken {
public:
	cTokenStatic() {
		mText[0] = '\0';
	}
	~cTokenStatic() {
	}

	virtual	const char *	GetText() const override { return mText; }
	virtual void			SetText( char const * text, size_t const len ) { 
		if ( text == nullptr ) {
			mText[0] = '\0';
		} else {
			if ( len <= len_ - 1 ) {
				memcpy( mText, text, len );
				mText[len] = '\0';
			} else {
				memcpy( mText, text, len_ - 1 );
				mText[len_ - 1] = '\0';
			}
		}
	}
private:
	char	mText[len_];
};

//==============================================================
// cTokenString
//==============================================================
class cTokenString : public cToken {
public:
	cTokenString( cTokenString const & other ) = default;

	cTokenString() {
	}
	~cTokenString() {
	}

	virtual	const char *	GetText() const override {
		return mText.c_str(); 
	}
	virtual void			SetText( char const * text, size_t const len ) { 
		if ( text == nullptr || text[0] == '\0' || len == 0 ) {
			mText = "";
			return;
		}
		char t[len + 1];
		OT_STRNCPY( t, len + 1, text, len );
		t[len] = '\0';
		mText = t;
	}

	operator const char*() const { return mText.c_str(); }

private:
	std::string	mText;
};

//==============================================================
// cLexer
//
// NOTE: this does not support UTF8 or Unicode
//==============================================================
class cLexer {
public:
	enum eFlags {
		// FLAG_HASH_COMMENTS				= ( 1 << 0 ),	// treat # as starting a single-line comment
		FLAG_IGNORE_CASE				= ( 1 << 1 ),	// use case insensitive compare for names
		FLAG_ALLOW_PUNCTUATION_IN_NAMES	= ( 1 << 2 ),	// don't stop parsing a name token when punctuation is encountered
		// FLAG_NO_SINGLE_LINE_C_COMMENTS	= ( 1 << 3 ),	// don't treat // as a single-line comment
	};

	enum ePunctuation {
		PUNC_NONE = -1,			// not punctuation
		PUNC_EXCLAMATION = 0,	// !
		PUNC_TILDE,				// ~
		PUNC_PERIOD,			// .
		PUNC_COMMA,				// ,
		PUNC_EQUAL,				// =
		PUNC_QUESTION_MARK,		// ?
		PUNC_GREATER,			// >
		PUNC_LESS,				// <
		PUNC_COLON,				// :
		PUNC_SEMICOLON,			// ;
		PUNC_PAREN_OPEN,		// (
		PUNC_PAREN_CLOSE,		// )
		PUNC_BRACKET_OPEN,		// [
		PUNC_BRACKET_CLOSE,		// ]
		PUNC_BRACE_OPEN,		// {
		PUNC_BRACE_CLOSE,		// }
		PUNC_BAR,				// |
		PUNC_SLASH_FORWARD,		// /
		PUNC_SLASH_BACKWARD,
		PUNC_PLUS,				// +
		PUNC_MINUS,				// -
		PUNC_MULTIPLY,			// *
		PUNC_AMPERSAND,			// &
		PUNC_CARET,				// ^
		PUNC_PERCENT,			// %
		PUNC_DOLLAR,			// $
		PUNC_POUND,				// #
		PUNC_AT,				// @
		PUNC_QUOTE,				// "
		PUNC_SINGLE_QUOTE,		// '
		PUNC_MAX
	};

	enum eCommentType {
		COMMENT_NONE,
		COMMENT_BLOCK,
		COMMENT_LINE,
		COMMENT_HASH
	};


	typedef eCommentType (*CommentTypeFn)( uint32_t const flags, char const curChar, const char nextChar );
	typedef bool (*CommentEndFn)( uint32_t const flags, eCommentType const commentType, char const curChar, char const nextChar );

	class cErrorHandler {
	public:
		virtual ~cErrorHandler() { }
		virtual bool	Error( char const * fmt, ... );
	};

	struct sInitParms {
		uint32_t		mFlags = 0;
		const char *	mPunctuation = nullptr;
		CommentTypeFn	mCommentTypeFn = nullptr;
		CommentEndFn	mCommentEndFn = nullptr;
		int32_t			mFileIndex = -1;
	};

	cLexer( char const * name, char const * text, size_t const len, const uint32_t flags = 0 );
	cLexer( char const * name, char const * text, size_t const len, const sInitParms & initParms );
	~cLexer();

	bool				Error( char const * fmt, ... ) const;

	bool				NextToken( cToken & token );
	bool				NextToken( cToken & token, cErrorHandler & errorHandler );

	bool				PeekNextToken( cToken & token );
	bool				PeekNextToken( cToken & token, cErrorHandler & errorHandler );

	bool				ExpectName( cToken & token );
	bool				ExpectName( cToken & token, cErrorHandler & errorHandler );

	bool				ExpectName( char const * str, cToken & token );
	bool				ExpectName( char const * str, cToken & token, cErrorHandler & errorHandler );

	bool				ExpectDouble( cToken & token, double & f );
	bool				ExpectDouble( cToken & token, double & f, cErrorHandler & errorHandler );

	bool				ExpectFloat( cToken & token, float & f );
	bool				ExpectFloat( cToken & token, float & f, cErrorHandler & errorHandler );

	bool				ExpectInteger( cToken & token, int32_t & i );
	bool				ExpectInteger( cToken & token, int32_t & i, cErrorHandler & errorHandler );

	bool				ExpectPunctuation( cToken & token, ePunctuation & punc );
	bool				ExpectPunctuation( cToken & token, ePunctuation & punc, cErrorHandler & errorHandler );
	bool				ExpectPunctuation( ePunctuation const punc, cToken & token );
	bool				ExpectPunctuation( ePunctuation const punc, cToken & token, cErrorHandler & errorHandler );

	int32_t				GetLine() const { return mLine; }

	bool				HadError() const { return !mErrorMsg.empty(); }
	std::string const &	GetError() const { return mErrorMsg; }

	char				GetPunctuationName( ePunctuation const punc );

private:
	void				BuildPunctuationTable( const char * punctuation );
	ePunctuation		GetPunctuationType( const char ch ) const;
	bool 				IsNegativeNumber( cLexer::ePunctuation const punc, char const * curPtr, char const * endPtr ) const;
	bool				SkipWhitespace();
	bool				SkipComments();
	bool				AtEnd() const { return mCur >= mEnd || *mCur == '\0'; }

private:
	std::string			mName;

	static const int 	PUNC_TABLE_SIZE = 255;
	ePunctuation		mPunctuationTable[PUNC_TABLE_SIZE];
	char const *		mPunctuationNames = nullptr;
	int					mNumPunctuation = 0;

	char const *		mText = nullptr;
	size_t				mLen = 0;
	uint32_t			mFlags = 0;
	char const *		mCur = nullptr;
	char const *		mEnd = nullptr;
	char const *		mLineStart = nullptr;
	int32_t				mLine = -1;
	int32_t				mFileIndex = -1;

	mutable std::string		mErrorMsg;

	CommentTypeFn		mCommentTypeFn = nullptr;
	CommentEndFn		mCommentEndFn = nullptr;
};

} // namespace otter