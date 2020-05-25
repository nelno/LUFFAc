// luffa.c

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <cinttypes>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include "fuzzywuzzy.hpp"
#include <conio.h>
#include <memory>
#include <time.h>
#include <chrono>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <filesystem>
#include "lexer.h"

template< typename T >
class cFileT {
public:
    cFileT( T * buffer, const size_t size )
        : mBuffer( buffer )
        , mSize( size ) {
        //mBuffer = std::make_unique<T>( buffer );
    }
    cFileT( const cFileT & other ) = delete;
    cFileT( cFileT && other ) = default;

    cFileT()
        : mSize( 0 ) {
    }

    ~cFileT() {
        mSize = 0;
    }

    const T* GetBuffer() const { return mBuffer.get(); }
    T* GetBuffer() { return mBuffer.get(); }
    size_t GetSize() const { return mSize; }

private:
    std::unique_ptr< T[] >  mBuffer;
    size_t                  mSize;
};

cFileT< char > ReadFile( const std::string & fileName ) {
    FILE * f = fopen( fileName.c_str(), "rb" );
    fseek( f, 0, SEEK_END );
    const size_t fsize = ftell( f );
    fseek( f, 0, SEEK_SET );
    if ( f != nullptr ) {
        cFileT< char > buffer( new char[fsize], fsize );

        const size_t numRead = fread( buffer.GetBuffer(), fsize, 1, f );
        if ( numRead == 1 ) {
            return buffer;
        }
    }
    return cFileT< char >();
}

bool IsWhitespace( const char c ) {
    return ( c == ' ' || c == '\n' || c == '\r' || c == '\t' );
}

bool SkipWhitespace( const char * & p, const char * end ) {
    for ( ; ; ) {
        if ( p >= end ) {
            return false;
        }
        if ( !IsWhitespace( *p ) ) {
            return true;
        }
        p++;
    }
}
/*
bool WordTokenize( const cFileT< char > & file, std::vector< std::string > & tokens ) {
    const char * buffer = file.GetBuffer();
    const size_t bufferSize = file.GetSize();

    const char * p = buffer;
    const char * end = buffer + bufferSize;
    const char * wordStart = p;
    while( p < end ) {
        if ( IsWhitespace( *p ) ) {
            assert( wordStart != nullptr );

            const size_t wordLen = (size_t)( p - wordStart );
            const size_t tokenSize = wordLen + 1;

            char token[tokenSize];
            strncpy( token, wordStart, wordLen );
            token[wordLen] = '\0';

            tokens.push_back( std::string( token ) );

            if ( !SkipWhitespace( p, end ) ) {
                break;
            }
            wordStart = p;
        }

        p++;
    }

    return true;
}
*/
void Test() {
    const string a = "I'm in your mind", b = "I'm in your mind fuzz";
    const string c = "fuzzy wuzzy was a bear", d = "wuzzy fuzzy was a bear";

    std::cout << fuzz::ratio(a, b) << '\n';
    std::cout << fuzz::partial_ratio(a, b) << '\n';
    std::cout << fuzz::token_sort_ratio(c, d) << '\n';
}

static otter::cLexer::eCommentType GetLuaCommentType( uint32_t const flags, 
        char const curChar, char const nextChar ) {
    if ( curChar == '-' && nextChar == '-' ) {
        return otter::cLexer::COMMENT_LINE;
    }

    if ( curChar == '[' && nextChar == ']' ) {
        return otter::cLexer::COMMENT_BLOCK;
    }
	return otter::cLexer::COMMENT_NONE;
}

static bool IsLuaCommentEnd( uint32_t const flags, 
        otter::cLexer::eCommentType const commentType, 
        char const curChar, char const nextChar ) {
    if ( commentType == otter::cLexer::COMMENT_LINE ) {
        return curChar == '\n';
    }
    if ( commentType == otter::cLexer::COMMENT_BLOCK ) {
        return curChar == ']' && nextChar == ']';
    }
	return false;
}

bool TokenizeFile( const std::string & fileName, int32_t const fileIndex, std::vector< otter::cTokenString > & tokens ) {
    cFileT< char > fileBuffer = ReadFile( fileName );

    otter::cLexer::sInitParms initParms;
    initParms.mCommentTypeFn = GetLuaCommentType;
    initParms.mCommentEndFn = IsLuaCommentEnd;
    initParms.mFileIndex = fileIndex;

    otter::cLexer lex( fileName.c_str(), fileBuffer.GetBuffer(), fileBuffer.GetSize(), initParms );

    otter::cTokenString token;
    while ( lex.NextToken( token ) ) {
        // std::cout << "token = '" << token << "'\n";
        otter::cToken::eTokenType tokenType = token.GetType();
        switch( tokenType ) {
            case otter::cToken::NAME:
                tokens.push_back( token );
                break;
            default:
                break;
        }
    }

    std::cout << "Found " << tokens.size() << " words in file.\n";

    return true;
}

void FindUniqueWordsInFiles( std::vector< std::string > & files, std::vector< otter::cTokenString >& uniqueWords ) {
    std::vector< otter::cTokenString > tokens;

    for ( size_t i = 0; i < files.size(); ++i ) {
        std::cout << "Loading file '" << files[i] << "'...";
        if ( !TokenizeFile( files[i], i, tokens ) ) {
            std::cout << " FAILED!\n";
        } else {
            std::cout << "\n";
        }
    }

    // add all words to a hash table to get only unique words
    std::unordered_map< std::string, int > wordHash;

    for ( size_t i = 0; i < tokens.size(); ++i ) {
        wordHash.insert( { tokens[i].GetText(), i } );
    }

    // now turn the hash back into a vector
    for ( auto it = wordHash.begin(); it != wordHash.end(); ++it ) {
        // std::cout << "Unique str '" << tokens[it->second] << "'\n";
        uniqueWords.push_back( tokens[it->second] );
    }
}

void FindMatchingFiles( const char * path, const char * ext, std::vector< std::string > & files ) {
    for ( const auto & p: std::filesystem::directory_iterator( path ) ) {
        std::filesystem::path filePath = p.path();
        std::filesystem::path extension = filePath.extension();
        if ( extension == ext ) {
            files.push_back( filePath.generic_string() );
        }
    }
}

//#define TEST

int main( const int argc, const char ** argv ) {
    std::vector< std::string > files;

#if defined( TEST )
    FindMatchingFiles( "e:\\projects\\github\\HammerOfJustas\\", ".lua", files );
#else
    if ( argc < 3 ) {
        std::cout << "LUFFA version 0.1\n";
        std::cout << "by Nelno the Amoeba\n\n";
        std::cout << "This utility will find similar identifiers in ASCII text files.\n\n";
        std::cout << "USAGE: luffa.exe <file path> <file ext>\n";
        exit(0);
    }

    FindMatchingFiles( argv[1], argv[2], files );
#endif
 
    std::vector< otter::cTokenString > uniqueWords;
    FindUniqueWordsInFiles( files, uniqueWords );

    std::cout << "Found " << uniqueWords.size() << " unique words in file.\n";

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for ( size_t i = 0; i < uniqueWords.size(); ++i ) {
        otter::cTokenString const & firstWord = uniqueWords[i];
        for ( size_t j = i + 1; j < uniqueWords.size(); ++j ) {
            otter::cTokenString const & secondWord = uniqueWords[j];
            uint32_t ratio = fuzz::ratio( firstWord.GetText(), secondWord.GetText() );
            if ( ratio > 90 ) {
                std::cout << "(" << ratio << ")\n";
                std::cout << "---> '" << firstWord.GetText() << "', " << files[firstWord.GetFileIndex()] << ":" << firstWord.GetLine() << "\n";
                std::cout << "     '" << secondWord.GetText() << "', " << files[secondWord.GetFileIndex()] << ":" << secondWord.GetLine() << "\n";
            }
        }
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / 1000.0f << " seconds" << std::endl;
    //std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;
    //std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count() << "[ns]" << std::endl;

#if defined( TEST )
    getch();
#endif

    return 0;
}