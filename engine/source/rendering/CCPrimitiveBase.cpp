/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCPrimitiveBase.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCPrimitives.h"
#include "CCFileManager.h"
#include "CCJS.h"


CCPrimitiveBase::CCPrimitiveBase(const long primitiveID)
{
    if( primitiveID != -1 )
    {
        this->primitiveID = primitiveID;
    }

	vertices = NULL;
	normals = NULL;
	textureInfo = NULL;
    frameBufferID = -1;
}


void CCPrimitiveBase::destruct()
{
	if( vertices != NULL )
	{
		gRenderer->derefVertexPointer( ATTRIB_VERTEX, vertices );
		free( vertices );
	}

	if( normals != NULL )
	{
        free( normals );
	}

    removeTexture();
}

void CCPrimitiveBase::setTexture(const char *file, CCResourceType resourceType, CCLambdaCallback *onDownloadCallback,
                                 const CCTextureLoadOptions options)
{
    if( resourceType == Resource_Unknown )
    {
        resourceType = CCFileManager::FindFile( file );
    }

    if( resourceType != Resource_Unknown )
    {
        const int textureIndex = gEngine->textureManager->assignTextureIndex( file, resourceType, options );
        setTextureHandleIndex( textureIndex );

        if( onDownloadCallback != NULL )
        {
            onDownloadCallback->safeRun();
            delete onDownloadCallback;
        }
    }
    else
    {
        CCLAMBDA_4( DownloadedCallback, CCPrimitiveBase, primitive, CCText, file, CCLambdaCallback*, nextCallback, CCTextureLoadOptions, options,
        {
            primitive->setTexture( file.buffer, Resource_Cached, nextCallback, options );
        });
        CCJSEngine::GetAsset( file, NULL, new DownloadedCallback( this, file, onDownloadCallback, options ) );
    }
}


void CCPrimitiveBase::setTextureHandleIndex(const int index)
{
    if( textureInfo == NULL )
    {
        textureInfo = new TextureInfo();
    }

    textureInfo->primaryIndex = index;

    const int textureHandleIndex = textureInfo->primaryIndex;
    CCTextureHandle *textureHandle = gEngine->textureManager->getTextureHandle( textureHandleIndex );
    if( textureHandle->texture != NULL )
    {
        adjustTextureUVs();
    }
    else
    {
        CCLAMBDA_CONNECT_THIS( textureHandle->onLoad, CCPrimitiveBase, adjustTextureUVs() );
    }
}


void CCPrimitiveBase::removeTexture()
{
	if( textureInfo != NULL )
	{
        DELETE_POINTER( textureInfo );
	}
}


void CCPrimitiveBase::render()
{
#if defined PROFILEON
    CCProfiler profile( "CCPrimitiveBase::render()" );
#endif

	bool usingTexture = false;
	if( textureInfo != NULL && textureInfo->primaryIndex > 0 )
	{
        //DEBUGLOG( "CCPrimitiveBase::render usingTexture %i", textureInfo->primaryIndex );
        
		if( gEngine->textureManager->setTextureIndex( textureInfo->primaryIndex ) )
		{
			usingTexture = true;

            #if defined PROFILEON
                CCTextureHandle *textureHandle = gEngine->textureManager->getTextureHandle( textureInfo->primaryIndex );
                if( textureHandle != NULL )
                {
                    profile.append( textureHandle->filePath.buffer );
                }
            #endif

            if( textureInfo->secondaryIndex > 0 )
            {
                // Why would you want to use the same texture twice? you wouldn't.. bad!
                CCASSERT( textureInfo->primaryIndex != textureInfo->secondaryIndex );

#ifndef DXRENDERER
#ifndef QT
                glActiveTexture( GL_TEXTURE1 );
                gEngine->textureManager->setTextureIndex( textureInfo->secondaryIndex );
                glActiveTexture( GL_TEXTURE0 );
#endif
#endif
            }
		}
	}
	else if( frameBufferID >= 0 )
    {
        gRenderer->frameBufferManager.bindFrameBufferTexture( frameBufferID );
    }
    else
	{
        //DEBUGLOG( "CCPrimitiveBase::render !usingTexture" );
		gEngine->textureManager->setTextureIndex( 0 );
	}

	renderVertices( usingTexture );
}
