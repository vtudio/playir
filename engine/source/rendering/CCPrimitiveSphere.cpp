/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCPrimitiveSphere.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCPrimitives.h"

static const uint space = 10;


void CCPrimitiveSphere::renderVertices(const bool textured)
{
    CCRenderer::CCSetRenderStates( true );

	GLVertexPointer( 3, GL_FLOAT, 0, vertices, vertexCount ) ;
	gRenderer->GLDrawArrays( GL_TRIANGLE_STRIP, 0, vertexCount );
}


static void appendNormals(CCVector3 *normal, const float *vertices, float *normals, const uint n, const float z)
{
    normal->set( vertices[n-3], vertices[n-2], vertices[n-1] * z );
    CCVector3Normalize( *normal );
    normals[n-3] = normal->x;
    normals[n-2] = normal->y;
    normals[n-1] = normal->z;
}


void CCPrimitiveSphere::setup(const float radius)
{
	vertexCount = ( 90 / space ) * ( 360 / space ) * 4;
	const size_t sizeOfMalloc = sizeof( float ) * vertexCount * 3;
	vertices = (float*)malloc( sizeOfMalloc );
	normals = (float*)malloc( sizeOfMalloc );

    float depth = -1.0f;
	float r = radius;
	int n = 0;

	CCVector3 normalVector;
	//for( float z = 1.0f; z >= -1.0f; z -= 2.0f )
    float z = 1.0f;
	{
		for( float b = 0.0f; b <= 90.0f - space; b+=space)
		{
			for( float a = 0.0f; a <= 360.0f - space; a+=space)
			{
				vertices[n++] = r * sinf((a) / 180.0f * CC_PI) * sinf((b) / 180.0f * CC_PI);
				vertices[n++] = r * cosf((a) / 180.0f * CC_PI) * sinf((b) / 180.0f * CC_PI);
				vertices[n++] = z * r * cosf((b) / 180.0f * CC_PI);
				//VERTEX[n].V = (2 * b) / 360;
				//VERTEX[n].U = (a) / 360;
				appendNormals( &normalVector, vertices, normals, n, depth );

				vertices[n++] = r * sinf((a) / 180.0f * CC_PI) * sinf((b + space) / 180.0f * CC_PI);
				vertices[n++] = r * cosf((a) / 180.0f * CC_PI) * sinf((b + space) / 180.0f * CC_PI);
				vertices[n++] = z * r * cosf((b + space) / 180.0f * CC_PI);
				//VERTEX[n].V = (2 * (b + space)) / 360;
				//VERTEX[n].U = (a) / 360;
				appendNormals( &normalVector, vertices, normals, n, depth );

				vertices[n++] = r * sinf((a + space) / 180.0f * CC_PI) * sinf((b) / 180.0f * CC_PI);
				vertices[n++] = r * cosf((a + space) / 180.0f * CC_PI) * sinf((b) / 180.0f * CC_PI);
				vertices[n++] = z * r * cosf((b) / 180.0f * CC_PI);
				//VERTEX[n].V = (2 * b) / 360;
				//VERTEX[n].U = (a + space) / 360;
				appendNormals( &normalVector, vertices, normals, n, depth );

				vertices[n++] = r * sinf((a + space) / 180.0f * CC_PI) * sinf((b + space) / 180.0f * CC_PI);
				vertices[n++] = r * cosf((a + space) / 180.0f * CC_PI) * sinf((b + space) / 180.0f * CC_PI);
				vertices[n++] = z * r * cosf((b + space) / 180.0f * CC_PI);
				//VERTEX[n].V = (2 * (b + space)) / 360;
				//VERTEX[n].U = (a + space) / 360;
				appendNormals( &normalVector, vertices, normals, n, depth );
			}
		}
	}
}
