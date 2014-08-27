/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCModel3D.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCPrimitives.h"
#include "CCTextureBase.h"
#include "CCAppManager.h"
#include "CCFileManager.h"
#include "CCPrimitiveOBJ.h"

#ifdef WP8
#include <ppl.h>
#include <ppltasks.h>
#endif


static bool ASYNC_LOAD_IN_PROGRESS = false;


CCPrimitive3D::CCPrimitive3D()
{
    vertexCount = 0;
    fileSize = 0;

    modelUVs = NULL;
    adjustedUVs = NULL;

    width = height = depth = 0.0f;

    cached = false;
    movedToOrigin = false;
}


void CCPrimitive3D::destruct()
{
    submodels.deleteObjects();
    
	while( ASYNC_LOAD_IN_PROGRESS )
	{
		//DEBUGLOG( "CCPrimtive3D::destruct() waiting...\n" );
		usleep( 1 );
	}

    if( cached )
    {
        modelUVs = NULL;
        vertices = NULL;
        normals = NULL;
    }

    if( modelUVs != NULL )
    {
        free( modelUVs );
    }

    if( adjustedUVs != NULL )
    {
        free( adjustedUVs );
    }

    super::destruct();
}


void CCPrimitive3D::loadDataAsync(const char *fileData)
{
	// Result on engine thread
	CCLAMBDA_2( Result, CCPrimitive3D, that, bool, loaded, {
        if( loaded )
        {
            that->loaded();
        }
        else
        {
            CCText script = "CCPrimitive3D.Loaded( ";
            script += that->primitiveID;
            script += " );";
            CCAppManager::WebJSEval( script.buffer, false, false );
        }
    });

	class Load : public CCLambdaSafeCallback															
	{																									
	public:																								
		Load(CCPrimitive3D *that, CCText fileData, bool loaded)													
		{																								
			this->that = that;																		
			this->lazyPointer = that->lazyPointer;														
			this->lazyID = that->lazyID;																
			this->fileData = fileData;																		
			this->loaded = loaded;                                            							
		}																								
	protected:		
		// Run on a random thread																					
		void run()																					
		{																								
			DEBUGLOG( "ASYNC_LOAD_IN_PROGRESS=true %s\n", that->filename.buffer );
            CCASSERT( ASYNC_LOAD_IN_PROGRESS == false );
			ASYNC_LOAD_IN_PROGRESS = true;
        
			loaded = that->loadData( fileData.buffer );
		
			DEBUGLOG( "ASYNC_LOAD_IN_PROGRESS=false %s\n", that->filename.buffer );
			ASYNC_LOAD_IN_PROGRESS = false;
		}		
		// Finish on the jobs thread																						
		void finish()																					
		{																								
			gEngine->jobsToEngineThread( new Result( that, loaded ) );
		}																								
	private:																						
		CCPrimitive3D *that;																					
		CCText fileData;																					
		bool loaded;                                                       								
	};

    gEngine->engineToJobsThread( new Load( this, fileData, false ) );
}


void CCPrimitive3D::removeTexture()
{
    super::removeTexture();
    for( int i=0; i<submodels.length; ++i )
    {
        Submodel *submodel = submodels.list[i];
        if( submodel->textureHandleIndex != -1 )
        {
            submodel->textureHandleIndex = -1;
        }
    }
}


void CCPrimitive3D::setSubmodelTextureHandleIndex(const char *submodelName, const int index)
{
    for( int i=0; i<submodels.length; ++i )
    {
        Submodel *submodel = submodels.list[i];
        if( CCText::Equals( submodel->name, submodelName ) )
        {
            submodel->textureHandleIndex = index;
            CCTextureHandle *textureHandle = gEngine->textureManager->getTextureHandle( index );
            if( textureHandle->texture != NULL )
            {
                adjustTextureUVs();
            }
            else
            {
                CCLAMBDA_CONNECT_THIS( textureHandle->onLoad, CCPrimitive3D, adjustTextureUVs() );
            }
        }
    }
}


void CCPrimitive3D::adjustTextureUVs()
{
	// We scale the textures to be square on Android
#ifndef ANDROID
    if( textureInfo != NULL )
    {
        const int textureHandleIndex = textureInfo->primaryIndex;
        CCTextureHandle *textureHandle = gEngine->textureManager->getTextureHandle( textureHandleIndex );
        const CCTextureBase *texture = textureHandle->texture;
        //CCASSERT( texture != NULL );
        if( texture != NULL )
        {
            const float width = texture->getImageWidth();
            const float height = texture->getImageHeight();
            const float allocatedWidth = texture->getAllocatedWidth();
            const float allocatedHeight = texture->getAllocatedHeight();

            if( width != allocatedWidth || height != allocatedHeight )
            {
                const float xScale = width / allocatedWidth;
                const float yScale = height / allocatedHeight;

                if( adjustedUVs == NULL )
                {
                    adjustedUVs = (float*)malloc( sizeof( float ) * vertexCount * 2 );
                }

                for( uint i=0; i<vertexCount; ++i )
                {
                    const int uvIndex = i*2;
                    const int x = uvIndex+0;
                    const int y = uvIndex+1;

                    adjustedUVs[x] = modelUVs[x] * xScale;
                    adjustedUVs[y] = modelUVs[y] * yScale;
                }

                return;
            }
        }
    }

    // Clear out our adjustedUVs if we haven't processed them above
    if( adjustedUVs != NULL )
    {
        free( adjustedUVs );
        adjustedUVs = NULL;
    }
#endif
}


const CCMinMax CCPrimitive3D::getYMinMaxAtZ(const float atZ) const
{
    CCMinMax mmYAtZ;

    for( uint i=0; i<vertexCount; ++i )
    {
        const uint index = i*3;
        float &y = vertices[index+1];
        float &z = vertices[index+2];

        if( z >= atZ )
        {
            mmYAtZ.consider( y );
        }
    }

    return mmYAtZ;
}


const CCVector3 CCPrimitive3D::getOrigin()
{
    if( movedToOrigin == false )
    {
        origin.x = mmX.min + ( width * 0.5f );
        origin.y = mmY.min + ( height * 0.5f );
        origin.z = mmZ.min + ( depth * 0.5f );
    }
    return origin;
}


void CCPrimitive3D::moveVerticesToOriginAsync(CCLambdaSafeCallback *callback)
{
	// Result on engine thread
    CCLAMBDA_2( Result, CCPrimitive3D, that, CCLambdaSafeCallback*, callback, {
        that->movedVerticesToOrigin();

		if( callback != NULL )
		{
			callback->safeRun();
			delete callback;
		}
    });
    
    CCLAMBDA_FINISH_2( Move, CCPrimitive3D, that, CCLambdaSafeCallback*, callback,
	
	// Run on a random thread
	{
        that->moveVerticesToOrigin();
	},
	
	// Finish on jobs thread
	{
        gEngine->jobsToEngineThread( new Result( that, callback ) );
    });

    gEngine->engineToJobsThread( new Move( this, callback ) );
}


void CCPrimitive3D::moveVerticesToOrigin()
{
    if( movedToOrigin == false )
    {
        const CCVector3 origin = getOrigin();

        mmX.reset();
        mmY.reset();
        mmZ.reset();

        for( uint i=0; i<vertexCount; ++i )
        {
            const uint index = i*3;
            float &x = vertices[index+0];
            float &y = vertices[index+1];
            float &z = vertices[index+2];

            x -= origin.x;
            y -= origin.y;
            z -= origin.z;

            mmX.consider( x );
            mmY.consider( y );
            mmZ.consider( z );
        }

        gRenderer->updateVertexPointer( ATTRIB_VERTEX, vertices );

        movedToOrigin = true;
    }
}



CCModel3D::CCModel3D()
{
    //shader = "phong";
}


CCModel3D::CCModel3D(const char *file, const CCResourceType resourceType,
					 const bool moveVerticesToOrigin)
{
    CCLAMBDA_2( Loaded, CCModel3D, that, CCPrimitiveOBJ*, primitive, {
        that->addPrimitive( primitive );
    });
    
	CCLAMBDA_2( LoadCallback, CCModel3D, that, bool, moveVerticesToOrigin,
	{
		CCPrimitiveOBJ *primitive = (CCPrimitiveOBJ*)runParameters;
		if( moveVerticesToOrigin )
		{
			that->moveVerticesToOriginAsync( new Loaded( that, primitive ) );
            return;
		}

        CCLambdaSafeCallback *callback = new Loaded( that, primitive );
        callback->safeRun();
        delete callback;
	});
    
	CCPrimitiveOBJ::LoadOBJ( file, resourceType, new LoadCallback( this, moveVerticesToOrigin ) );
}


void CCModel3D::setTexture(const char *file, CCResourceType resourceType,
                           const CCTextureLoadOptions options)
{
    if( file != NULL )
    {
        primitive->setTexture( file, resourceType, NULL, options );
    }
}
