/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCString.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include <limits>


CCText::CCText(const int inLength)
{
    ensureLength( inLength );
}


CCText::CCText(const char *text)
{
    zero();
    set( text );
}


CCText::CCText(const CCText &other)
{
    zero();
    set( other.buffer );
}


bool CCText::operator==(const char *other) const
{
    if( length > 0 )
    {
        return ( strcmp( buffer, other ) == 0 );
    }
    return false;
}


bool CCText::operator!=(const char *other) const
{
    if( length > 0 )
    {
        return ( strcmp( buffer, other ) != 0 );
    }
    return false;
}


CCText& CCText::operator=(const char *text)
{
    set( text );
    return *this;
}


CCText& CCText::operator=(const CCText &other)
{
    if( this != &other )
    {
        if( other.buffer != NULL )
        {
            set( other.buffer );
        }
        else if( buffer != NULL )
        {
            free( buffer );
            buffer = NULL;
            bufferSize = 0;
        }
    }
    return *this;
}


void CCText::set(const char *text)
{
    if( text != NULL )
    {
        length = strlen( text );
        ensureLength( length );

        if( buffer != NULL )
        {
            memcpy( buffer, text, length );
        }
    }
}


void CCText::set(const char *text, const uint length)
{
    if( length > 0 )
    {
        this->length = length;
        ensureLength( length );

        if( buffer != NULL )
        {
            memcpy( buffer, text, length );
        }

#ifdef DEBUGON
        for( uint i=0; i<length; ++i )
        {
            if( buffer[i] != text[i] )
            {
                CCASSERT( false );
            }
        }
        CCASSERT( buffer[length] == '\0' );
#endif
    }
}


void CCText::clear()
{
    set( "" );
}


void CCText::trimLength(const uint maxLength)
{
    if( length > maxLength )
    {
        length = maxLength;
        buffer[length] = '\0';
    }
}


bool CCText::StartsWith(const char *buffer, const char *token)
{
    if( buffer != NULL )
    {
        const char *find = strstr( buffer, token );
        if( find == buffer )
        {
            return true;
        }
    }
    return false;
}


void CCText::stripExtension()
{
    length -= 4;
    buffer[length] = 0;
}


void CCText::stripFile()
{
    CCPtrList<char> directorySplit;
    CCText data = buffer;
    data.split( directorySplit, "/" );

    this->clear();
    for( int i=0; i<directorySplit.length-1; ++i )
    {
        *this += directorySplit.list[i];
        *this += "/";
    }
}

void CCText::stripDirectory(const bool windowsDirectories)
{
    CCPtrList<char> directorySplit;
    CCText data = buffer;
    data.split( directorySplit, "/" );

    if( directorySplit.length > 0 )
    {
        set( directorySplit.list[directorySplit.length-1] );
        for( int i=0; i<directorySplit.length-1; ++i )
        {
            const char *split = directorySplit.list[i];
            if( CCText::Contains( split, "?" ) )
            {
                set( split );
                for( int j=(i+1); j<directorySplit.length; ++j )
                {
                    *this += "/";
                    *this += directorySplit.list[j];
                }
                break;
            }
        }
    }

	if( windowsDirectories )
	{
		data = buffer;
		data.split( directorySplit, "\\" );
		if( directorySplit.length > 1 )
		{
			set( directorySplit.list[directorySplit.length-1] );
		}
	}
}


void CCText::strip(const char *token)
{
    CCPtrList<char> tokenSplit;
    CCText data = buffer;
    data.split( tokenSplit, token );
    if( tokenSplit.length > 1 )
    {
        set( tokenSplit.list[0] );
        for( int i=1; i<tokenSplit.length; ++i )
        {
            *this += tokenSplit.list[i];
        }
    }
}


void CCText::toLowerCase()
{
	for( uint i=0; i<length; ++i )
	{
		char lowercase = tolower( buffer[i] );
		buffer[i] = lowercase;
	}
}


void CCText::SetLastWord(const char *inBuffer, CCText &outText)
{
    CCText nameData = inBuffer;
    CCPtrList<char> nameDataSplit;
    nameData.split( nameDataSplit, " " );
    CCASSERT( nameDataSplit.length > 0 );
    outText = nameDataSplit.list[nameDataSplit.length-1];
}


void CCText::replaceChar(const char search, const char replace)
{
    for( uint i=0; i<length; ++i )
    {
        if( buffer[i] == search )
        {
            buffer[i] = replace;
        }
    }
}


void CCText::replaceChars(const char *token, const char *replace)
{
    CCPtrList<char> tokenSplit;
    split( tokenSplit, token );
    CCText newText;
    for( int i=0; i<tokenSplit.length; ++i )
    {
        const char *rawData = tokenSplit.list[i];
        newText += rawData;

        if( i < tokenSplit.length-1 )
        {
            newText += replace;
        }
    }

    set( newText.buffer );
}


void CCText::replaceChars(const char *token, const CCText &replace)
{
    replaceChars( token, replace.buffer );
}


void CCText::replaceChars(const char *token, const char replace)
{
    CCPtrList<char> tokenSplit;
    split( tokenSplit, token );
    CCText newText;
    for( int i=0; i<tokenSplit.length; ++i )
    {
        const char *rawData = tokenSplit.list[i];
        newText += rawData;

        if( i < tokenSplit.length-1 )
        {
            newText += replace;
        }
    }

    set( newText.buffer, newText.length );
}


void CCText::split(CCPtrList<char> &splitList, const char *token, const bool first)
{
    const uint tokenLength = strlen( token );
    if( length > 0 && buffer != NULL )
    {
        char *previousFind = NULL;
        char *currentFind = buffer;
        splitList.add( currentFind );
        do
        {
            previousFind = currentFind;
            currentFind = strstr( currentFind, token );
            if( currentFind != NULL )
            {
                *currentFind = 0;
                currentFind += tokenLength;
                if( currentFind != buffer )
                {
                    splitList.add( currentFind );

                    if( first )
                    {
                        break;
                    }
                }
            }
        } while( currentFind != NULL && currentFind != previousFind );
    }
}


void CCText::splitBetween(CCText source, const char *from, const char *to)
{
    CCText &result = source;
    result.splitAfter( result, from );
    result.splitBefore( result, to );
    set( result.buffer );
}


void CCText::splitBefore(CCText source, const char *before)
{
    CCPtrList<char> list1;
    source.split( list1, before, true );
    set( list1.list[0] );
}


void CCText::splitBeforeLast(CCText source, const char *before)
{
    CCPtrList<char> list1;
    source.split( list1, before, false );
    if( list1.length > 2 )
    {
        set( list1.list[list1.length-2] );
    }
    else
    {
        set( list1.list[0] );
    }
}


void CCText::splitAfter(CCText source, const char *after)
{
    CCPtrList<char> list1;
    source.split( list1, after, true );
    if( list1.length > 1 )
    {
        set( list1.list[1] );
        for( int i=2; i<list1.length; ++i )
        {
            *this += after;
            *this += list1.list[i];
        }
    }
    else
    {
        *this = "";
    }
}


void CCText::splitAfterLast(CCText source, const char *after)
{
    CCPtrList<char> list1;
    source.split( list1, after, false );
    if( list1.length > 1 )
    {
        set( list1.list[list1.length-1] );
    }
    else
    {
        *this = "";
    }
}


void CCText::removeBetween(const char *from, const char *to)
{
    CCText start( buffer );
    start.splitAfter( start, to );
    CCText end( buffer );
    end.splitBefore( end, from );

    *this = end.buffer;
    *this += from;
    *this += to;
    *this += start.buffer;
}


void CCText::removeBetweenIncluding(const char *from, const char *to)
{
    CCText start( buffer );
    CCPtrList<char> list1;
    split( list1, from );
    if( list1.length > 1 )
    {
        CCText end( list1.list[1] );
        start.set( list1.list[0] );
        end.splitAfter( end, to );

        set( start.buffer );
        *this += end.buffer;
    }
}


void CCText::encodeForWeb()
{
    replaceChars( "\\", "\\\\" );
    replaceChars( "\"", "\\\"" );
    replaceChars( "\'", "\\\'" );
    replaceChars( "\n", "\\n" );
    replaceChars( "\r", "\\r" );
    replaceChars( "\f", "\\f" );
}


void CCText::removeNewLines()
{
    replaceChars( "\n", "" );
    replaceChars( "\r", "" );
}
