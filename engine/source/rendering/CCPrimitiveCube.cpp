/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCPrimitiveCube.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCPrimitives.h"


static const uint vertexCount = 6 * 4;


CCPrimitiveCube::CCPrimitiveCube()
{
	vertices = (float*)malloc( sizeof( float ) * vertexCount * 3 );
}


// CCPrimitiveBase
void CCPrimitiveCube::renderVertices(const bool textured)
{
	// Indices
	static const ushort faces[] =
	{
		0,  1,  2,  3,  3,      // Face 0 - triangle strip ( v0,  v1,  v2,  v3)
        4,  4,  5,  6,  7,  7,  // Face 1 - triangle strip ( v4,  v5,  v6,  v7)
        8,  8,  9, 10, 11, 11,  // Face 2 - triangle strip ( v8,  v9, v10, v11)
        12, 12, 13, 14, 15, 15, // Face 3 - triangle strip (v12, v13, v14, v15)
        16, 16, 17, 18, 19, 19, // Face 4 - triangle strip (v16, v17, v18, v19)
        20, 20, 21, 22, 23      // Face 5 - triangle strip (v20, v21, v22, v23)
	};
	static const uint numberOfFaces = sizeof( faces ) / sizeof( ushort );

	// draw the cube
    CCRenderer::CCSetRenderStates( true );

	GLVertexPointer( 3, GL_FLOAT, 0, vertices, vertexCount );
	gRenderer->GLDrawElements( GL_TRIANGLE_STRIP, numberOfFaces, GL_UNSIGNED_SHORT, faces );
}


void CCPrimitiveCube::renderOutline()
{
	// Indices
	static const ushort faces[] =
	{
		0, 1, 3, 2, 0, 4, 5, 1, 5, 7, 3, 7, 6, 2, 6, 4,
	};
	static const uint numberOfFaces = sizeof( faces ) / sizeof( ushort );

    CCRenderer::CCSetRenderStates( true );

#ifndef DXRENDERER
	GLVertexPointer( 3, GL_FLOAT, 0, vertices, vertexCount );
	gRenderer->GLDrawElements( GL_LINE_STRIP, numberOfFaces, GL_UNSIGNED_SHORT, faces );
#endif
}


void CCPrimitiveCube::setupSquare(const float size)
{
	const float hSize = size * 0.5f;
	setupRectangle( size, -hSize, size, size );
}


void CCPrimitiveCube::setupRectangle(const float width, const float height)
{
    const float hHeight = height * 0.5f;
	setupRectangle( width, -hHeight, height, width );
}


void CCPrimitiveCube::setupRectangle(const float width, const float y, const float height, const float depth)
{
	const float hWidth = width * 0.5f;
	setupTrapezoid( depth, y, height, -hWidth, hWidth, -hWidth, hWidth );
}


void CCPrimitiveCube::setupTrapezoid(const float depth, const float y, const float height, const float bL, const float bR, const float tL, const float tR)
{
	setupTrapezoidZ( depth, 0.0f, 0.0f, y, height, bL, bR, tL, tR );
}


void CCPrimitiveCube::setupTrapezoidZ(const float depth, const float bZ, const float tZ, const float y, const float height, const float bL, const float bR, const float tL, const float tR)
{
	setup( depth, depth, bZ, tZ, y, height, bL, bR, tL, tR );
}


void CCPrimitiveCube::setupTrapezoidDepths(const float bottomDepth, const float topDepth, const float y, const float height, const float bL, const float bR, const float tL, const float tR)
{
	setup( bottomDepth, topDepth, 0.0f, 0.0f, y, height, bL, bR, tL, tR );
}


static void addVertex(float *vertices, int &index, const float x, const float y, const float z)
{
    vertices[index++] = x;
    vertices[index++] = y;
    vertices[index++] = z;
}


void CCPrimitiveCube::setup(const float bottomDepth, const float topDepth,
                            const float bZ, const float tZ,
                            const float y, const float height,
                            const float leftBottom, const float rightBottom,
                            const float leftTop, const float rightTop)
{
	const float hBottomDepth = bottomDepth * 0.5f;
	const float hTopDepth = topDepth * 0.5f;
	float frontTop = tZ + hTopDepth;
	float backTop = tZ - hTopDepth;
	float frontBottom = bZ + hBottomDepth;
	float backBottom = bZ - hBottomDepth;

	float bottom = y;
	float top = y+height;

    int vertexIndex = 0;
    // 0
    addVertex( vertices, vertexIndex,   leftBottom,     bottom, frontBottom );
    addVertex( vertices, vertexIndex,   rightBottom,    bottom, frontBottom );
    addVertex( vertices, vertexIndex,   leftTop,        top,    frontTop );
    addVertex( vertices, vertexIndex,   rightTop,       top,    frontTop );

    // 1
    addVertex( vertices, vertexIndex,   rightBottom,    bottom, frontBottom );
    addVertex( vertices, vertexIndex,   rightBottom,    bottom, backBottom );
    addVertex( vertices, vertexIndex,   rightTop,       top,    frontTop );
    addVertex( vertices, vertexIndex,   rightTop,       top,    backTop );

    // 2
    addVertex( vertices, vertexIndex,   rightBottom,    bottom, backBottom );
    addVertex( vertices, vertexIndex,   leftBottom,     bottom, backBottom );
    addVertex( vertices, vertexIndex,   rightTop,       top,    backTop );
    addVertex( vertices, vertexIndex,   leftTop,        top,    backTop );

    // 3
    addVertex( vertices, vertexIndex,   leftBottom,     bottom, backBottom );
    addVertex( vertices, vertexIndex,   leftBottom,     bottom, frontBottom );
    addVertex( vertices, vertexIndex,   leftTop,        top,    backTop );
    addVertex( vertices, vertexIndex,   leftTop,        top,    frontTop );

    // 4
    addVertex( vertices, vertexIndex,   leftBottom,     bottom, frontBottom );
    addVertex( vertices, vertexIndex,   rightBottom,    bottom, frontBottom );
    addVertex( vertices, vertexIndex,   leftBottom,     bottom, backBottom );
    addVertex( vertices, vertexIndex,   rightBottom,    bottom, backBottom );

    // 5
    addVertex( vertices, vertexIndex,   leftTop,        top,    frontTop );
    addVertex( vertices, vertexIndex,   rightTop,       top,    frontTop );
    addVertex( vertices, vertexIndex,   leftTop,        top,    backTop );
    addVertex( vertices, vertexIndex,   rightTop,       top,    backTop );
}
