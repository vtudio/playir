/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCCameraManager.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCCameraRecorder.h"
#include "CCAppManager.h"
#include "CCTexture2D.h"


CCCameraRecorder::CCCameraRecorder()
{
#ifdef Q_OS_WIN

    current.name = "camera";
    loading.name = "cameraLoading";
    current.textureIndex = gEngine->textureManager->assignTextureIndex( current.name, Resource_Packaged, true, false, true );

    loading.textureIndex = gEngine->textureManager->assignTextureIndex( loading.name, Resource_Packaged, true, false, true );

#endif
}


void CCCameraRecorder::updateJobsThread()
{
#ifdef Q_OS_WIN

    if( CCAppManager::IsCameraActive() )
    {
        static CCData fileData;

        CCText fullFilePath = QtRootPath().toUtf8().constData();
        fullFilePath += "perceptual.tmp";
        FILE *pFile = fopen( fullFilePath.buffer, "rb" );
        if( pFile != NULL )
        {
            // obtain file size:
            fseek( pFile , 0, SEEK_END );
            long lSize = ftell( pFile );
            rewind( pFile );

            // allocate memory to contain the whole file:
            fileData.setSize( sizeof(char)*lSize );

            // copy the file into the buffer:
            fileData.length = fread( fileData.buffer, 1, lSize, pFile );
            fclose( pFile );

            if( lSize > 0 )
            {
                int itr=0;

                int width = 0;
                {
                    int i=0;
                    char chars[16];
                    do
                    {
                        chars[i++] = fileData.buffer[itr++];
                    } while( fileData.buffer[itr] != 13 );
                    chars[i] = 0;
                    width = atoi( chars );
                    itr++; // \r
                    itr++; // \n
                }

                int height = 0;
                {
                    int i=0;
                    char chars[16];
                    do
                    {
                        chars[i++] = fileData.buffer[itr++];
                    } while( fileData.buffer[itr] != 13 );
                    chars[i] = 0;
                    height = atoi( chars );
                    itr++; // \r
                    itr++; // \n
                }

                const int resolution = 8;
                const int resolutionWidth = (width-resolution) / resolution;
                const int resolutionHeight = (height-resolution) / resolution;
                uint vertexCount = 6 * resolutionWidth * resolutionHeight;
                if( vertexCount > loading.vertexCount )
                {
                    if( loading.vertices != NULL )
                    {
                        free( loading.vertices );
                    }
                    if( loading.uvs != NULL )
                    {
                        free( loading.uvs );
                    }
                    loading.vertices = (float*)malloc( sizeof( float ) * 3 * vertexCount );
                    loading.uvs = (float*)malloc( sizeof( float ) * 2 * vertexCount );
                    loading.vertexCount = vertexCount;
                }

                float *vertices = loading.vertices;
                float *uvs = loading.uvs;

                const float uPadding = -0.075f;
                const float vPadding = -0.2f;
                const float invWidth = 1.0f / width*1.275f;
                const float invHeight = 1.0f / height*1.4f;

                int vertexIndex = 0;
                int uvIndex = 0;
                const float scaleSize = 0.25f;
                const float scaleDepth = 0.05125f;

                for( int y=0; y<height-resolution; y+=resolution )
                {
                    const int yDepthPitch = y*width;
                    for( int x=0; x<width-resolution; x+=resolution )
                    {
                        const int depthIndex = itr+yDepthPitch+x;
                        CCASSERT( depthIndex < itr+width*height );

                        const int depth = (unsigned char)fileData.buffer[depthIndex];
                        const int depthRight = (unsigned char)fileData.buffer[depthIndex+resolution];
                        const int depthBottom = (unsigned char)fileData.buffer[depthIndex+(width*resolution)];
                        const int depthBottomRight = (unsigned char)fileData.buffer[depthIndex+(width*resolution)+resolution];
                        {
                            vertices[vertexIndex++] = (x)*scaleSize;
                            vertices[vertexIndex++] = (height-y)*scaleSize;
                            vertices[vertexIndex++] = depth*scaleDepth;
                            uvs[uvIndex++] = (x * invWidth)+uPadding;
                            uvs[uvIndex++] = (y * invHeight)+vPadding;

                            // Right
                            vertices[vertexIndex++] = (x+resolution)*scaleSize;
                            vertices[vertexIndex++] = (height-y)*scaleSize;
                            vertices[vertexIndex++] = depthRight*scaleDepth;
                            uvs[uvIndex++] = ((x+resolution) * invWidth)+uPadding;
                            uvs[uvIndex++] = (y * invHeight)+vPadding;

                            // Bottom
                            vertices[vertexIndex++] = (x)*scaleSize;
                            vertices[vertexIndex++] = (height-(y+resolution))*scaleSize;
                            vertices[vertexIndex++] = depthBottom*scaleDepth;
                            uvs[uvIndex++] = (x * invWidth)+uPadding;
                            uvs[uvIndex++] = ((y+resolution) * invHeight)+vPadding;
                        }
                        {
                            // Right
                            vertices[vertexIndex++] = (x+resolution)*scaleSize;
                            vertices[vertexIndex++] = (height-y)*scaleSize;
                            vertices[vertexIndex++] = depthRight*scaleDepth;
                            uvs[uvIndex++] = ((x+resolution) * invWidth)+uPadding;
                            uvs[uvIndex++] = ((y) * invHeight)+vPadding;

                            // Bottom Right
                            vertices[vertexIndex++] = (x+resolution)*scaleSize;
                            vertices[vertexIndex++] = (height-(y+resolution))*scaleSize;
                            vertices[vertexIndex++] = depthBottomRight*scaleDepth;
                            uvs[uvIndex++] = ((x+resolution) * invWidth)+uPadding;
                            uvs[uvIndex++] = ((y+resolution) * invHeight)+vPadding;

                            // Bottom
                            vertices[vertexIndex++] = (x)*scaleSize;
                            vertices[vertexIndex++] = (height-(y+resolution))*scaleSize;
                            vertices[vertexIndex++] = depthBottom*scaleDepth;
                            uvs[uvIndex++] = ((x) * invWidth)+uPadding;
                            uvs[uvIndex++] = ((y+resolution) * invHeight)+vPadding;
                        }
                    }
                }

                CCASSERT( vertexIndex == (vertexCount*3) );

                CCLAMBDA_4( DepthFunction, CCCameraRecorder, recorder, float*, vertices, float*, uvs, uint, vertexCount, {

                    CameraRecording &current = recorder->current;
                    if( current.vertices != NULL )
                    {
                        free( current.vertices );
                    }
                    current.vertices = vertices;

                    if( current.uvs != NULL )
                    {
                        free( current.uvs );
                    }
                    current.uvs = uvs;

                    current.vertexCount = vertexCount;

                    for( int i=0; i<current.linked.length; ++i )
                    {
                        current.linked.list[i]->referenceData( vertices, uvs, vertexCount );
                    }

                });
                gEngine->jobsToEngineThread( new DepthFunction( this, vertices, uvs, vertexCount ) );
                loading.vertices = NULL;
                loading.uvs = NULL;
                loading.vertexCount = 0;

                itr++; // \r
                itr++; // \n
                CCASSERT( itr <= fileData.length );
            }
        }

        CCTextureHandle *loadingHandle = gEngine->textureManager->getTextureHandle( loading.textureIndex );
        if( !loadingHandle->loading )
        {
            if( loadingHandle->texture != NULL )
            {
                CCTextureHandle *currentHandle = gEngine->textureManager->getTextureHandle( current.textureIndex );
                CCLAMBDA_2( SwapFunction, CCTextureBase, texture, CCTextureHandle*, handle, {

                    if( handle->texture != NULL )
                    {
                        delete handle->texture;
                    }
                    handle->texture = texture;

                    if( gEngine->getJSEngine() != NULL )
                    {
                        gEngine->getJSEngine()->loadedReferencedImage( handle->filePath.buffer, handle->texture );
                    }

                });
                gEngine->jobsToEngineThread( new SwapFunction( loadingHandle->texture, currentHandle ) );
            }
            loadingHandle->texture = NULL;

            fullFilePath = QtRootPath().toUtf8().constData();
            fullFilePath += "cameracolour.bmp";
            pFile = fopen( fullFilePath.buffer, "rb" );
            if( pFile != NULL )
            {
                // obtain file size:
                fseek( pFile , 0, SEEK_END );
                long lSize = ftell( pFile );
                rewind( pFile );

                // allocate memory to contain the whole file:
                fileData.setSize( sizeof(char)*lSize );

                // copy the file into the buffer:
                fileData.length = fread( fileData.buffer, 1, lSize, pFile );
                fclose( pFile );

                CCTextureBase *texture = new CCTexture2D();
                const bool loadedSuccessfully = texture->loadData( fileData.buffer, fileData.length );
                if( loadedSuccessfully )
                {
                    loadingHandle->loading = true;

                    CCLAMBDA_2( CreateFunction, CCTextureBase, texture, CCTextureHandle*, handle, {

                        texture->createGLTexture( handle->mipmap );
                        handle->loading = false;
                        handle->texture = texture;

                        if( gEngine->getJSEngine() != NULL )
                        {
                            gEngine->getJSEngine()->loadedReferencedImage( handle->filePath.buffer, handle->texture );
                        }
                        
                    });
                    gEngine->jobsToEngineThread( new CreateFunction( texture, loadingHandle ) );
                }
                else
                {
                    delete texture;
                }
            }
        }
    }
#endif
}


void CCCameraRecorder::unlink(void *pointer)
{
    current.linked.remove( pointer );
}
