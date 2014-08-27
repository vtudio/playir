/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCTools.h
 * Description : Contains base functions.
 *
 * Created     : 01/03/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCTOOLS_H__
#define __CCTOOLS_H__


#include "CCPlatform.h"


// add GCC_PREPROCESSOR_DEFINITIONS DEBUGON in Project Settings
#define ALLOW_DEBUGLOG
#if defined DEBUGON && defined ALLOW_DEBUGLOG

#define LOG_FPS 1
#if defined WP8 || defined WIN8
extern void DEBUGLOG(const char *text, ...);
#else
#define DEBUGLOG printf
#endif
#define DEBUGFLUSH fflush( stdout );

#define LOG_NEWMAX(string, currentMax, newMax) \
if( currentMax < newMax ) { currentMax = newMax; printf( "%s - %i \n", string, currentMax ); }

#else

#define DEBUGLOG
#define DEBUGFLUSH
#define LOG_NEWMAX(string, currentMax, newMax)

#endif


#ifdef DEBUGON

extern void CCDebugAssert(const bool condition, const char *file, const int line, const char *message=NULL);

#define CCASSERT(condition) CCDebugAssert( condition, __FILE__, __LINE__ );

#define CCASSERT_MESSAGE(condition, message) CCDebugAssert( condition, __FILE__, __LINE__, message );

#ifdef DXRENDERER
#define DEBUG_OPENGL() {}
#else
#define DEBUG_OPENGL() \
{                                   \
    GLenum error = glGetError();    \
    CCASSERT( error ==  0 );          \
}
#endif

#else // DEBUGON

inline void DebugAssert(const bool condition, const char *message=NULL) {}
#define CCASSERT(cond) {};
#define CCASSERT_MESSAGE(cond,str) {}

#define DEBUG_OPENGL() {}

#endif


#define DELETE_OBJECT(object)       \
if( object != NULL )                \
{                                   \
    object->destruct();             \
    delete object;                  \
    object = NULL;                  \
}


#define DELETE_POINTER(pointer)     \
if( pointer != NULL )               \
{                                   \
    delete pointer;                 \
    pointer = NULL;                 \
}


#define FREE_POINTER(pointer)       \
if( pointer != NULL )               \
{                                   \
    free( pointer );                \
    pointer = NULL;                 \
}



inline bool CCHasFlag(const uint source, const uint flag)
{
    const uint result = source & flag;
    if( result != 0 )
    {
        return true;
    }

    return false;
}

inline void CCAddFlag(uint &source, const uint flag)
{
    if( CCHasFlag( source, flag ) == false )
    {
        source |= flag;
    }
}

inline void CCRemoveFlag(uint &source, const uint flag)
{
    if( CCHasFlag( source, flag ) )
    {
        source ^= flag;
    }
}


bool CCSaveData(const char *id, const char *data);
bool CCLoadData(const char *id, struct CCText &result);


#endif // __CCTOOLS_H__
