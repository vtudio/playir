/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCTypes.h
 * Description : Contains base structures.
 *
 * Created     : 01/03/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCTYPES_H__
#define __CCTYPES_H__


#include "CCTools.h"


typedef	unsigned short ushort;


enum CCResourceType
{
    Resource_Unknown,
    Resource_Cached,
#ifdef IOS
    Resource_Temp,
#else
    Resource_Temp = Resource_Cached,
#endif
    Resource_Packaged
};


class CCBaseType
{
public:
    virtual ~CCBaseType();
    virtual void destruct() = 0;
};


class CCUpdater : public CCBaseType
{
public:
    typedef CCBaseType super;

    CCUpdater()
    {
#ifdef DEBUGON
        destructCalled = false;
#endif
    }

    virtual ~CCUpdater()
    {
#ifdef DEBUGON
        CCASSERT( destructCalled );
#endif
    }

    virtual void destruct()
    {
        #ifdef DEBUGON
            destructCalled = true;
        #endif
    }

    virtual bool update(const float delta) = 0;
    virtual void finish() {}

#ifdef DEBUGON
protected:
    bool destructCalled;
#endif
};


#include "CCArray.h"


struct CCData
{
    CCData();
    ~CCData();

    void setSize(const uint inLength);
    void ensureLength(const uint minLength, const bool keepData=false);
    void set(const char *data, const uint inLength);
	void append(const char *data, const uint inLength);
    
    CCData& operator=(const CCData &other);

    CCData& operator+=(const CCText &other);
    CCData& operator+=(const char *other);
    CCData& operator+=(const char other);
    CCData& operator+=(const int value);
    CCData& operator+=(const uint value);
    CCData& operator+=(const long value);
    CCData& operator+=(const long long value);
    CCData& operator+=(const unsigned long long value);
    CCData& operator+=(const float value);
    CCData& operator+=(const double value);

protected:
    void zero();

public:
	uint length;
    char *buffer;
    uint bufferSize;
};


#include "CCString.h"


#endif // __CCBASETYPES_H__
