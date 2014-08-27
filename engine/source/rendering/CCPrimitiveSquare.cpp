/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCPrimitiveSquare.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCPrimitives.h"
#include "CCTextureBase.h"


CCPrimitiveSquare::CCPrimitiveSquare(const long primitiveID) :
    CCPrimitiveBase( primitiveID )
{
	customUVs = NULL;
    adjustedUVs = NULL;

    scale = NULL;
	position = NULL;

    customVertexPositionBuffer = NULL;
}


void CCPrimitiveSquare::destruct()
{
    if( customVertexPositionBuffer != NULL )
    {
        CCASSERT( customVertexPositionBuffer == vertices );
        if( customVertexPositionBuffer == vertices )
        {
            vertices = NULL;
        }
    }

	if( customUVs != NULL )
	{
		delete customUVs;
	}

    if( adjustedUVs != NULL )
    {
        delete adjustedUVs;
    }

    if( scale != NULL )
    {
        DELETE_POINTER( scale );
    }

	if( position != NULL )
	{
		DELETE_POINTER( position );
	}

	super::destruct();
}


// CCPrimitiveBase
void CCPrimitiveSquare::adjustTextureUVs()
{
	// We scale the textures to be square on Android
#ifndef ANDROID
    if( textureInfo != NULL )
    {
        const int textureHandleIndex = textureInfo->primaryIndex;
        CCTextureHandle *textureHandle = gEngine->textureManager->getTextureHandle( textureHandleIndex );
        const CCTextureBase *texture = textureHandle->texture;
        if( texture != NULL )
        {
            const float width = texture->getImageWidth();
            const float height = texture->getImageHeight();
            const float allocatedWidth = texture->getAllocatedWidth();
            const float allocatedHeight = texture->getAllocatedHeight();

            if( width == allocatedWidth && height == allocatedHeight )
            {
                DELETE_POINTER( adjustedUVs );
            }
            else
            {
                if( customUVs != NULL )
                {
                    const float x2 = customUVs->uvs[0];
                    const float y1 = customUVs->uvs[1];
                    const float x1 = customUVs->uvs[2];
                    const float y2 = customUVs->uvs[5];

                    const float widthScale = width / allocatedWidth;
                    const float heightScale = height / allocatedHeight;
                    CCPrimitiveSquareUVs::Setup( &adjustedUVs, x1 * widthScale, y1 * heightScale, x2 * widthScale, y2 * heightScale );
                }
                else
                {
                    CCPrimitiveSquareUVs::Setup( &adjustedUVs, 0.0f, 0.0f, width / allocatedWidth, height / allocatedHeight );
                }
            }
        }
    }
#endif
}


void CCPrimitiveSquare::setTextureUVs(const float x1, const float y1, const float x2, const float y2)
{
    CCPrimitiveSquareUVs::Setup( &customUVs, x1, y1, x2, y2 );

    adjustTextureUVs();
}


void CCPrimitiveSquare::renderVertices(const bool textured)
{
#if defined PROFILEON
    CCProfiler profile( "CCPrimitiveSquare::renderVertices()" );
#endif

	if( position != NULL || scale != NULL )
	{
		GLPushMatrix();

		if( position != NULL )
		{
			GLTranslatef( position->x, position->y, position->z );
		}

		GLScalef( scale->x, scale->y, scale->z );
	}

    if( adjustedUVs != NULL )
    {
		CCSetTexCoords( adjustedUVs->uvs );
    }
    else if( customUVs != NULL )
	{
		CCSetTexCoords( customUVs->uvs );
	}
	else
	{
		CCDefaultTexCoords();
	}

	CCRenderer::CCSetRenderStates( true );

    if( vertices != NULL )
    {
        GLVertexPointer( 3, GL_FLOAT, 0, vertices, 4 );
    }
    else
    {
        static const float vertices_forwardFacing[] =
		{
            0.5f, 0.5f,  0.0f,          // Top right
            -0.5f, 0.5f,  0.0f,         // Top left
            0.5f,  -0.5f,  0.0f,        // Bottom right
			-0.5f,  -0.5f,  0.0f,       // Bottom left
		};
        GLVertexPointer( 3, GL_FLOAT, 0, vertices_forwardFacing, 4 );
    }
	gRenderer->GLDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

	if( position != NULL || scale != NULL )
	{
		GLPopMatrix();
	}
}


void CCPrimitiveSquare::renderOutline()
{
#if defined PROFILEON
	CCProfiler profile( "CCPrimitiveSquare::renderOutline()" );
#endif

    CCRenderer::CCSetRenderStates( true );

#ifndef DXRENDERER
	GLVertexPointer( 3, GL_FLOAT, 0, vertices, 4 );

	static const ushort faces[] =
	{
		0, 1, 3, 2, 0,
	};
	static const uint numberOfFaces = sizeof( faces ) / sizeof( ushort );

	gRenderer->GLDrawElements( GL_LINE_STRIP, numberOfFaces, GL_UNSIGNED_SHORT, faces );
#endif
}


void CCPrimitiveSquare::setCustomVertexPositionBuffer(float *buffer)
{
    if( customVertexPositionBuffer != NULL )
    {
        CCASSERT( customVertexPositionBuffer == vertices );
        if( customVertexPositionBuffer == vertices )
        {
            vertices = NULL;
        }
    }

    customVertexPositionBuffer = buffer;
    if( vertices != NULL )
    {
		gRenderer->derefVertexPointer( ATTRIB_VERTEX, vertices );
        free( vertices );
    }
    vertices = buffer;
}


void CCPrimitiveSquare::flipY()
{
    CCPrimitiveSquareUVs::Setup( &customUVs, 1.0f, 0.0f, 0.0f, 1.0f );
    adjustTextureUVs();
}
