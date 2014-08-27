/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCTypes.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include <limits>


// CCBaseType
CCBaseType::~CCBaseType()
{
}



// CCData
CCData::CCData()
{
    zero();
}


void CCData::zero()
{
    length = 0;
    buffer = NULL;
    bufferSize = 0;
}


CCData::~CCData()
{
    if( buffer != NULL )
    {
        free( buffer );
    }
}


void CCData::setSize(const uint inLength)
{
    if( inLength > 0 )
    {
        length = inLength;
        if( length+1 > bufferSize )
        {
            if( buffer != NULL )
            {
                free( buffer );
            }

            // 32 byte aligned
            bufferSize = ( ( length / 32 ) + 1 ) * 32;
            buffer = (char*)malloc( bufferSize );
        }
        buffer[inLength] = 0;
    }
    else
    {
        if( buffer != NULL )
        {
            free( buffer );
        }
        zero();
    }
}


void CCData::ensureLength(const uint minLength, const bool keepData)
{
    if( minLength+1 > bufferSize )
    {
		char *currentData = buffer;
		const uint currentLength = length;

        // 32 byte aligned
        bufferSize = ( ( minLength / 32 ) + 1 ) * 32;
        buffer = (char*)malloc( bufferSize );
        CCASSERT( buffer != NULL );

		if( currentData != NULL )
		{
			if( keepData )
			{
				memcpy( buffer, currentData, currentLength );
				buffer[currentLength] = 0;
			}
			free( currentData );
		}
    }

    if( buffer != NULL )
    {
        buffer[minLength] = 0;
    }
}


void CCData::set(const char *data, const uint inLength)
{
    if( inLength > 0 )
    {
		setSize( inLength );
        memcpy( buffer, data, inLength );

#ifdef DEBUGON
        for( uint i=0; i<inLength; ++i )
        {
            if( buffer[i] != data[i] )
            {
                CCASSERT( false );
            }
        }
#endif
    }
}


void CCData::append(const char *data, const uint inLength)
{
    if( inLength > 0 )
    {
		const uint currentLength = length;
		const uint newLength = length+inLength;
		ensureLength( newLength, true );
		memcpy( buffer+currentLength, data, inLength );
		buffer[newLength] = 0;
		length = newLength;
	}
}


CCData& CCData::operator=(const CCData &other)
{
    set( other.buffer, other.length );
    return *this;
}


CCData& CCData::operator+=(const CCText &other)
{
	if( other.length > 0 )
	{
		*this += other.buffer;
	}
    return *this;
}


CCData& CCData::operator+=(const char *other)
{
    if( other != NULL )
    {
        if( length == 0 )
        {
            const uint length = strlen( other );
            ensureLength( length );
            set( other, length );
        }
        else
        {
            const uint otherLength = strlen( other );
            const uint totalLength = length + otherLength;
            ensureLength( totalLength, true );

            memcpy( buffer+length, other, otherLength );
            length = totalLength;
        }
    }
    return *this;
}


CCData& CCData::operator+=(const char value)
{
    const uint otherLength = 1;
    if( length == 0 )
    {
        ensureLength( otherLength );
        buffer[0] = value;
    }
    else
    {
        const uint totalLength = length + otherLength;
        ensureLength( totalLength, true );

        buffer[length] = value;
        length = totalLength;
    }
    return *this;
}


CCData& CCData::operator+=(const uint value)
{
    return operator+=( (int)value );
}

CCData& CCData::operator+=(const int value)
{
    char other[32];
    sprintf( other, "%i", value );
    *this += other;
    return *this;
}


CCData& CCData::operator+=(const long value)
{
    char other[64];
    sprintf( other, "%ld", value );
    *this += other;
    return *this;
}


CCData& CCData::operator+=(const long long value)
{
    char other[64];
    sprintf( other, "%lld", value );
    *this += other;
    return *this;
}


CCData& CCData::operator+=(const unsigned long long value)
{
    char other[64];
    sprintf( other, "%lld", value );
    *this += other;
    return *this;
}


CCData& CCData::operator+=(const float value)
{
    const int limit = CCNextPowerOf2( std::numeric_limits<float>().digits + std::numeric_limits<float>().digits10 );
    ensureLength( length+limit, true );
    
    sprintf( buffer, "%s%f", buffer, value );
    length = strlen( buffer );
    return *this;
}


CCData& CCData::operator+=(const double value)
{
    const int limit = CCNextPowerOf2( std::numeric_limits<double>().digits + std::numeric_limits<double>().digits10 );
    ensureLength( length+limit, true );
    
    sprintf( buffer, "%s%lf", buffer, value );
    length = strlen( buffer );
    return *this;
}
