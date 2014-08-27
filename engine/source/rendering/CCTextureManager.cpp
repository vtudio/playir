/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCTextureManager.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCTexture2D.h"
#include "CCTextureFontPageFile.h"
#include "CCTextureSprites.h"
#include "CCAppManager.h"


CCTextureHandle::~CCTextureHandle()
{
    onLoad.deleteObjectsAndList();
    deleteTexture();
}


void CCTextureHandle::deleteTexture(const bool reduceMemory)
{
	if( texture != NULL )
	{
		if( reduceMemory )
		{
			gEngine->textureManager->totalTexturesLoaded--;
			gEngine->textureManager->totalUsedTextureSpace -= texture->getBytes();
		}
		delete texture;
		texture = NULL;
	}
}



CCTextureManager::CCTextureManager()
{
    textureSprites = new CCTextureSprites();
}


CCTextureManager::~CCTextureManager()
{
	textureHandles.deleteObjectsAndList();

    fontPages.deleteObjectsAndList();

    if( textureSprites != NULL )
    {
        delete textureSprites;
    }
}

static int CompareTextureTimes(const void *a, const void *b)
{
    CCTextureHandle &textureA = **(CCTextureHandle**)a;
    CCTextureHandle &textureB = **(CCTextureHandle**)b;
    return textureB.lastTimeUsed - textureA.lastTimeUsed;
}

void CCTextureManager::invalidateAllTextureHandles()
{
    CCNativeThreadLock();
    CCJobsThreadLock();
    
	DEBUGLOG( "CCTextureManager::invalidateAllTextureHandles start\n" );

    currentGLTexture = NULL;
    totalTexturesLoaded = 0;
    totalUsedTextureSpace = 0;

    const double startTime = CCEngine::GetSystemTime();
    const double finishTime = startTime + 0.25f;
    double currentTime = startTime;

    // Delete invalid textures
    for( int i=0; i<textureHandles.length; ++i )
    {
        CCTextureHandle *handle = textureHandles.list[i];
        if( handle->texture != NULL )
        {
        	bool valid = glIsTexture( handle->texture->name() );
            if( !valid )
            {
                DEBUGLOG( "CCTextureManager::invalidateAllTextureHandles adding %s\n", handle->filePath.buffer );
            	recreatingTextureHandles.addOnce( handle );
            	handle->deleteTexture( false );
            }
        }
    }
    
    qsort( recreatingTextureHandles.list, recreatingTextureHandles.length, sizeof(CCTextureHandle**), CompareTextureTimes );
#ifdef DEBUGON
    for( int i=0; i<recreatingTextureHandles.length; ++i )
    {
        CCTextureHandle *handle = recreatingTextureHandles.list[i];
        DEBUGLOG( "CCTextureManager::invalidateAllTextureHandles sorted %i %s\n", i, handle->filePath.buffer );
    }
#endif

    // Load in resident textures
    {
        CCTextureLoadOptions options;
        options.disableMipMapping = true;
        options.asyncLoad = false;
        options.alwaysResident = true;
        {
            const int textureHandleIndex = assignTextureIndex( "transparent.png", Resource_Packaged, options );
            CCTextureHandle *handle = getTextureHandle( textureHandleIndex );
            recreatingTextureHandles.remove( handle );
        }
        {
            const int textureHandleIndex = assignTextureIndex( "white.png", Resource_Packaged, options );
            CCTextureHandle *handle = getTextureHandle( textureHandleIndex );
            recreatingTextureHandles.remove( handle );
        }
    }

    CCLAMBDA_2( OnLoadCallback, CCTextureManager, textureManager, CCTextureHandle*, handle,
    {
        textureManager->recreatedTexture( handle );
    });

    // Load remaining textures
    for( int i=0; i<recreatingTextureHandles.length; ++i )
    {
    	CCTextureHandle *handle = recreatingTextureHandles.list[i];

        if( !handle->loadable )
        {
            DEBUGLOG( "CCTextureManager::invalidateAllTextureHandles unloadable %s\n", handle->filePath.buffer );
            recreatingTextureHandles.remove( handle );
            --i;
            continue;
        }

        if( handle->loading )
        {
            continue;
        }

        // Reload texture now?
		if( currentTime < finishTime || handle->options.alwaysResident )
		{
			currentTime = CCEngine::GetSystemTime();
			DEBUGLOG( "CCTextureManager::invalidateAllTextureHandles reloading %s %f %f\n", handle->filePath.buffer, startTime, currentTime );
			loadTextureSync( *handle );
            recreatingTextureHandles.remove( handle );
            --i;
			continue;
		}

        // Out of time, re-load later
        DEBUGLOG( "CCTextureManager::invalidateAllTextureHandles reloading later %s\n", handle->filePath.buffer );
        loadTextureAsync( *handle, new OnLoadCallback( this, handle ) );
    }

    if( recreatingTextureHandles.length > 100 )
    {
        recreatingTextureHandles.length = 100;
    }

    setTextureIndex( 0 );

    CCSetTexCoords( NULL );
	CCDefaultTexCoords();

	CCJobsThreadUnlock();
    CCNativeThreadUnlock();
}


void CCTextureManager::recreatedTexture(CCTextureHandle *handle)
{
    CCNativeThreadLock();
    CCJobsThreadLock();
    recreatingTextureHandles.remove( handle );
	DEBUGLOG( "CCTextureManager::invalidateAllTextureHandles reloaded %i %s\n", recreatingTextureHandles.length, handle->filePath.buffer );

	for( int i=0; i<recreatingTextureHandles.length; ++i )
	{
		handle = recreatingTextureHandles.list[0];
		DEBUGLOG( "CCTextureManager::invalidateAllTextureHandles remaining %i %s\n", i, handle->filePath.buffer );
        break;
	}
	CCJobsThreadUnlock();
    CCNativeThreadUnlock();
}


bool CCTextureManager::isReady()
{
    if( recreatingTextureHandles.length > 0 )
    {
        return false;
    }
    return true;
}


void CCTextureManager::loadFont(const char *name, const uint textureIndex, const char *csv)
{
    CCTextureFontPageFile *fontPage = NULL;
    for( int i=0; i<fontPages.length; ++i )
    {
        CCTextureFontPageFile *itr = (CCTextureFontPageFile*)fontPages.list[i];
        if( CCText::Equals( name, itr->getName() ) )
        {
            fontPage = itr;
            break;
        }
    }

    if( fontPage != NULL )
    {
        if( textureIndex != fontPage->getTextureIndex() || !CCText::Equals( csv, fontPage->getCSV() ) )
        {
            fontPage->load( textureIndex, csv );
        }
    }
    else
    {
        CCTextureFontPageFile *fontPage = new CCTextureFontPageFile( name );
        if( fontPage->load( textureIndex, csv ) )
        {
            fontPages.add( fontPage );
        }
    }
}


uint CCTextureManager::assignTextureIndex(const char *filePath, const CCResourceType resourceType,
                                          CCTextureLoadOptions options)
{
    //DEBUGLOG( "CCTextureManager::assignTextureIndex %s\n", filePath );

    CCTextureHandle *handle = NULL;
    for( int i=0; i<textureHandles.length; ++i )
    {
        CCTextureHandle *textureHandleItr = textureHandles.list[i];
        if( strcmp( textureHandleItr->filePath.buffer, filePath ) == 0 )
        {
            if( textureHandleItr->options.equals( options ) )
            {
                handle = textureHandleItr;
                if( !options.asyncLoad && handle->texture == NULL )
                {
                    loadTextureSync( *handle );
                }
                return i;
            }
		}
    }

    if( handle == NULL )
    {
        handle = new CCTextureHandle( filePath, resourceType );
        textureHandles.add( handle );
    }

    handle->options = options;
    if( !options.asyncLoad )
    {
        loadTextureSync( *handle );
    }

    uint index = (uint)( textureHandles.length-1 );
    return (uint)index;
}



CCTextureHandle* CCTextureManager::getTextureHandle(const char *filePath, const CCResourceType resourceType, const CCTextureLoadOptions options)
{
    for( int i=0; i<textureHandles.length; ++i )
    {
        CCTextureHandle *textureHandle = textureHandles.list[i];
        if( strcmp( textureHandle->filePath.buffer, filePath ) == 0 )
        {
            if( textureHandle->resourceType == resourceType )
            {
                if( textureHandle->options.filter == options.filter &&
                    textureHandle->options.disableMipMapping == options.disableMipMapping )
                {
                    return textureHandle;
                }
            }
		}
    }

    return NULL;
}


CCTextureHandle* CCTextureManager::getTextureHandle(const int handleIndex)
{
    if( handleIndex >= 0 )
    {
        return textureHandles.list[handleIndex];
    }
    return NULL;
}


void CCTextureManager::deleteTextureHandle(const char *filePath)
{
    CCTextureHandle *handle = NULL;
    for( int i=0; i<textureHandles.length; ++i )
    {
        CCTextureHandle *textureHandleItr = textureHandles.list[i];
        if( strcmp( textureHandleItr->filePath.buffer, filePath ) == 0 )
        {
            handle = textureHandleItr;
            textureHandles.remove( handle );
            delete handle;
            break;
		}
    }
}


void CCTextureManager::invalidateTextureHandle(const char *filePath)
{
    CCTextureHandle *handle = NULL;
    for( int i=0; i<textureHandles.length; ++i )
    {
        CCTextureHandle *textureHandleItr = textureHandles.list[i];
        if( strcmp( textureHandleItr->filePath.buffer, filePath ) == 0 )
        {
            handle = textureHandleItr;
            if( handle->texture != NULL )
            {
                handle->deleteTexture();
            }
            break;
		}
    }
}


void CCTextureManager::loadTextureAsync(CCTextureHandle &textureHandle, CCLambdaSafeCallback *callback)
{
    if( callback != NULL )
    {
        textureHandle.onLoad.add( callback );
    }

    if( textureHandle.loading )
    {
        return;
    }
    textureHandle.loading = true;

#if defined PROFILEON
    CCProfiler profile( "CCTextureManager::loadTextureAsync()" );
#endif

#ifdef DEBUGON
    CCText debug = "CCTextureManager::loadTextureAsync ";
    debug += textureHandle.filePath.buffer;
    debug += " ";
    debug += gEngine->time.lifetime;
    debug += "\n";
    DEBUGLOG( "%s", debug.buffer );
#endif

    CCLAMBDA_5( OnLoadedFunction, CCTextureManager, that, CCTextureBase*, texture, CCText, filePath, CCResourceType, resourceType, CCTextureLoadOptions, options, {
        const bool loaded = (bool)runParameters;

        CCTextureHandle *textureHandle = that->getTextureHandle( filePath.buffer, resourceType, options );
        if( textureHandle == NULL )
        {
            delete texture;
            texture = NULL;
            DEBUGLOG( "CCTextureManager::loadTextureAsync not found %s\n", filePath.buffer );
        }
        else
        {
            textureHandle->loading = false;
            if( !loaded )
            {
                that->loadTextureFailed( *textureHandle, &texture );
            }
            else
            {
                that->loadedTexture( *textureHandle, texture );
            }

            CCLAMBDA_SIGNAL pendingCallbacks;
            for( int i=0; i<textureHandle->onLoad.length; ++i )
            {
                textureHandle->onLoad.list[i]->runParameters = (void*)texture;
                pendingCallbacks.add( textureHandle->onLoad.list[i] );
            }
            textureHandle->onLoad.length = 0;
            CCLAMBDA_EMIT_ONCE( pendingCallbacks );

            gEngine->textureLoaded( *textureHandle );
        }
    });

	CCTextureBase *texture = new CCTexture2D();
    texture->loadAndCreateAsync( textureHandle.filePath.buffer, textureHandle.resourceType, textureHandle.options,
                                 new OnLoadedFunction( this, texture, textureHandle.filePath, textureHandle.resourceType, textureHandle.options ) );
}


void CCTextureManager::loadTextureSync(CCTextureHandle &textureHandle, CCLambdaSafeCallback *callback)
{
    if( callback != NULL )
    {
        textureHandle.onLoad.add( callback );
    }

    if( textureHandle.loading )
    {
        return;
    }
    textureHandle.loading = true;

#if defined PROFILEON
    CCProfiler profile( "CCTextureManager::loadTextureSync()" );
#endif

#ifdef DEBUGON
    CCText debug = "CCTextureManager::loadTextureSync ";
    debug += textureHandle.filePath.buffer;
    debug += " ";
    debug += gEngine->time.lifetime;
    debug += "\n";
    DEBUGLOG( "%s", debug.buffer );
#endif

	CCTextureBase *texture = new CCTexture2D();
    const bool loaded = !gEngine->paused && texture->loadAndCreateSync( textureHandle.filePath.buffer, textureHandle.resourceType, textureHandle.options );

    textureHandle.loading = false;
    if( !loaded )
    {
        loadTextureFailed( textureHandle, &texture );
    }
    else
    {
        loadedTexture( textureHandle, texture );
    }

    CCLAMBDA_SIGNAL pendingCallbacks;
    for( int i=0; i<textureHandle.onLoad.length; ++i )
    {
        textureHandle.onLoad.list[i]->runParameters = (void*)texture;
        pendingCallbacks.add( textureHandle.onLoad.list[i] );
    }
    textureHandle.onLoad.length = 0;
    CCLAMBDA_EMIT_ONCE( pendingCallbacks );

    gEngine->textureLoaded( textureHandle );
}


void CCTextureManager::loadTextureFailed(CCTextureHandle &textureHandle, CCTextureBase **texture)
{
//#ifdef DEBUGON
//    CCText debug = "CCTextureManager::loadTexture::loadAndCreate::Failed ";
//    debug += filePath.buffer;
//    debug += "\n";
//    DEBUGLOG( "%s", debug.buffer );
//#endif
    delete *texture;
    *texture = NULL;

    if( !gEngine->paused )
    {
        textureHandle.loadable = false;
    }
}


void CCTextureManager::loadedTexture(CCTextureHandle &textureHandle, CCTextureBase *texture)
{
#ifdef DEBUGON
    CCText debug;
    debug += "CCTextureManager::loadedTexture ";
    debug += textureHandle.filePath.buffer;
    debug += " ";
    debug += gEngine->time.lifetime;
    debug += "\n";
    DEBUGLOG( "%s", debug.buffer );
#endif

    // Estimated texture usage, need to cater for bit depth and mip maps for more accuracy
#ifdef WP8
    
    const int maxSpace = 32 * 1024 * 1024;
    
#else
    
    int maxSpace = 128 * 1024 * 1024;

    const float deviceMemory = CCAppManager::TotalMemory();
    if( deviceMemory <= 256.0f )
    {
        maxSpace = 32 * 1024 * 1024;
    }
    
#endif

    int newTotalUsedTextureSpace = totalUsedTextureSpace + texture->getBytes();
    if( newTotalUsedTextureSpace > maxSpace )
    {
        trim();
    }

    totalTexturesLoaded++;
    totalUsedTextureSpace += texture->getBytes();

#ifdef DEBUGON
	debug = "CCTextureManager::loadedTexture()::loaded ";
	debug += totalTexturesLoaded;
	debug += " ";
	debug += totalUsedTextureSpace;
	debug += "\n";
	DEBUGLOG( "%s", debug.buffer );
#endif

    textureHandle.texture = texture;
}


void CCTextureManager::trim()
{
    DEBUGLOG( "CCTextureManager::trimming %i %i \n", totalTexturesLoaded, totalUsedTextureSpace );

    int targetSpace = 64 * 1024 * 1024;

#ifdef WP8
    
    targetSpace = 24 * 1024 * 1024;

#else
    
    const float deviceMemory = CCAppManager::TotalMemory();
    if( deviceMemory <= 256.0f )
    {
    	targetSpace = 24 * 1024 * 1024;
    }
    
#endif

    while( totalUsedTextureSpace > targetSpace )
    {
        float oldestTime = -1.0f;
        CCTextureHandle *oldestHandle = NULL;
        for( int i=1; i<textureHandles.length; ++i )
        {
            CCTextureHandle *handle = textureHandles.list[i];
            //DEBUGLOG( "handle: %s \n", handle->filePath.buffer );
            if( handle != NULL )
            {
                if( !handle->options.alwaysResident )
                {
                    if( handle->texture != NULL )
                    {
                        const float distance = gEngine->time.lifetime - handle->lastTimeUsed;
                        if( distance > oldestTime )
                        {
                            oldestTime = distance;
                            oldestHandle = handle;
                        }
                    }
                }
            }
        }

        if( oldestHandle != NULL )
        {
        	oldestHandle->deleteTexture();
            DEBUGLOG( "CCTextureManager::trimmed %s %i %i \n", oldestHandle->filePath.buffer, totalTexturesLoaded, totalUsedTextureSpace );
        }
    }
}


void CCTextureManager::bindTexture(const CCTextureName *texture)
{
	if( currentGLTexture != texture )
	{
        if( texture != NULL )
        {
            gRenderer->GLBindTexture( GL_TEXTURE_2D, texture );
        }
		currentGLTexture = texture;
	}
}


bool CCTextureManager::setTextureIndex(const int handleIndex)
{
    CCTextureHandle *handle = textureHandles.list[handleIndex];
    if( handle != NULL && handle->loadable )
    {
        if( handle->texture == NULL )
        {
            if( handle->loading == false )
            {
                loadTextureAsync( *handle );
            }
            if( handleIndex != 0 )
            {
                setTextureIndex( 0 );
            }
            return false;
        }

        handle->lastTimeUsed = gEngine->time.lifetime;
		bindTexture( handle->texture );
        return true;
	}
	else
	{
		if( handleIndex != 0 )
		{
            setTextureIndex( 0 );
		}
		return false;
	}
}


CCTextureBase* CCTextureManager::getTexture(const int handleIndex, CCLambdaSafeCallback *callback, const bool async)
{
    CCTextureHandle *handle = textureHandles.list[handleIndex];
    if( handle != NULL )
    {
        if( handle->texture == NULL )
        {
            // Load texture (will fill the runParameters with texture as the result)
            if( async )
            {
                loadTextureAsync( *handle, callback );
            }
            else
            {
                loadTextureSync( *handle, callback );
                return handle->texture;
            }
        }
        else if( callback != NULL )
        {
            // Texture already loaded
            callback->runParameters = (void*)handle->texture;
            callback->safeRun();
            delete callback;
        }
    }
    else
    {
        // Nothing to load
        if( callback != NULL )
        {
            callback->safeRun();
            delete callback;
        }
    }

    return NULL;
}
