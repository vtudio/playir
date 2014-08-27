/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCTools.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include <assert.h>
#include "CCFileManager.h"


#ifdef DEBUGON


#ifdef ALLOW_DEBUGLOG
#if defined WP8 || defined WIN8
#include <wrl/client.h>
void DEBUGLOG(const char *text, ...)
{
	CCText output = text;

	va_list argp;
	va_start(argp, text);
	char *next = va_arg(argp, char *);
	va_end(argp);

	OutputDebugStringA( output.buffer );
}
#endif
#endif

void CCDebugAssert(const bool condition, const char *file, const int line, const char *message)
{
    if( !condition )
    {
        DEBUGLOG( "%s %i \n", file, line );
        fflush( stdout );
        if( message )
        {
            DEBUGLOG( "ASSERT: %s \n", message );
        }

        // Root call to Java on Android for ease of debugging
#ifdef ANDROID
        CCJNI::Assert( file, line, message );
#endif
        assert( condition );
    }
}

#endif



bool CCSaveData(const char *id, const char *data)
{
    if( id != NULL && data != NULL )
    {
        CCText file = "cache/";
        file += id;
        CCFileManager::SaveCachedFile( file.buffer, data, strlen( data ) );
        return true;
    }
    return false;
}


bool CCLoadData(const char *id, CCText &result)
{
    CCText file = "cache/";
    file += id;

    if( CCFileManager::DoesFileExist( file.buffer, Resource_Cached ) )
    {
        const int fileSize = CCFileManager::GetFile( file.buffer, result, Resource_Cached, false, NULL );
        if( fileSize > 0 )
        {
            return true;
        }
    }

    return false;
}
