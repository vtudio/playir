/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCModel3DS.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCPrimitives.h"
#include "CCModel3DS.h"
#include "CCFileManager.h"
#include "CCTextureBase.h"


CCModel3DS::CCModel3DS(const char *file,
                       const char *texture1, const CCResourceType resourceType1, const CCTextureLoadOptions options1,
                       const char *texture2, const CCResourceType resourceType2, const CCTextureLoadOptions options2)
{
    primitive3ds = new CCPrimitive3DS();
    primitive3ds->load( file );
    primitive3ds->setTexture( texture1, resourceType1, NULL, options1 );
    if( texture2 != NULL )
    {
        //primitive3ds->textureInfo->secondaryIndex = gEngine->textureManager->assignTextureIndex( texture2, resourceType, mipmap, false, alwaysResident );
    }
    addPrimitive( primitive3ds );
}



// CCPrimitive3DS
CCPrimitive3DS::CCPrimitive3DS()
{
}


void CCPrimitive3DS::destruct()
{
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


bool CCPrimitive3DS::load(const char* file)
{
    CCText fullFilePath;
    CCFileManager::GetFilePath( fullFilePath, file, Resource_Packaged );

    // Once we've copied out the data, we delete the 3dsobject
    obj3ds_type *object = new obj3ds_type();
	const int result = Load3DS( object, fullFilePath.buffer );
    CCASSERT( result == 1 );

    // Extract the normals
    Calc3DSNormals( object );

    if( result == 1 )
    {
        vertexCount = object->polygons_qty * 3;

        modelUVs = (float*)malloc( sizeof( float ) * vertexCount * 2 );
        vertices = (float*)malloc( sizeof( float ) * vertexCount * 3 );
        normals = (float*)malloc( sizeof( float ) * vertexCount * 3 );

        for( int l_index = 0; l_index < object->polygons_qty; ++l_index )
        {
            const int uvIndex = l_index*3*2;
            const int vertexIndex = l_index*3*3;

            {
                const int index = object->polygon[l_index].a;
                CCASSERT( index >= 0 );
                CCASSERT( index < object->vertices_qty );

                // UVs
                {
                    const float u = object->mapcoord[index].u;
                    const float v = object->mapcoord[index].v;
                    modelUVs[uvIndex+0] = u;
                    modelUVs[uvIndex+1] = 1.0f - v;
                }

                // Vertices
                {
                    float x = object->vertex[index].x;
                    float y = object->vertex[index].y;
                    float z = object->vertex[index].z;
                    vertices[vertexIndex+0] = x;
                    vertices[vertexIndex+1] = y;
                    vertices[vertexIndex+2] = z;

                    mmX.consider( x );
                    mmY.consider( y );
                    mmZ.consider( z );
                }

                // Normals
                {
                    float x = object->normal[index].x;
                    float y = object->normal[index].y;
                    float z = object->normal[index].z;
                    normals[vertexIndex+0] = x;
                    normals[vertexIndex+1] = y;
                    normals[vertexIndex+2] = z;
                }
            }

            {
                const int index = object->polygon[l_index].b;
                CCASSERT( index >= 0 );
                CCASSERT( index < object->vertices_qty );

                // UVs
                {
                    const float u = object->mapcoord[index].u;
                    const float v = object->mapcoord[index].v;
                    modelUVs[uvIndex+2] = u;
                    modelUVs[uvIndex+3] = 1.0f - v;
                }

                // Vertices
                {
                    float x = object->vertex[index].x;
                    float y = object->vertex[index].y;
                    float z = object->vertex[index].z;
                    vertices[vertexIndex+3] = x;
                    vertices[vertexIndex+4] = y;
                    vertices[vertexIndex+5] = z;

                    mmX.consider( x );
                    mmY.consider( y );
                    mmZ.consider( z );
                }

                // Normals
                {
                    float x = object->normal[index].x;
                    float y = object->normal[index].y;
                    float z = object->normal[index].z;
                    normals[vertexIndex+3] = x;
                    normals[vertexIndex+4] = y;
                    normals[vertexIndex+5] = z;
                }
            }

            {
                const int index = object->polygon[l_index].c;
                CCASSERT( index >= 0 );
                CCASSERT( index < object->vertices_qty );

                // UVs
                {
                    const float u = object->mapcoord[index].u;
                    const float v = object->mapcoord[index].v;
                    modelUVs[uvIndex+4] = u;
                    modelUVs[uvIndex+5] = 1.0f - v;
                }

                // Vertices
                {
                    float x = object->vertex[index].x;
                    float y = object->vertex[index].y;
                    float z = object->vertex[index].z;
                    vertices[vertexIndex+6] = x;
                    vertices[vertexIndex+7] = y;
                    vertices[vertexIndex+8] = z;

                    mmX.consider( x );
                    mmY.consider( y );
                    mmZ.consider( z );
                }

                // Normals
                {
                    float x = object->normal[index].x;
                    float y = object->normal[index].y;
                    float z = object->normal[index].z;
                    normals[vertexIndex+6] = x;
                    normals[vertexIndex+7] = y;
                    normals[vertexIndex+8] = z;
                }
            }
        }

        width = mmX.size();
        height = mmY.size();
        depth = mmZ.size();

        delete object;
        return true;
    }

    delete object;
    return false;
}


void CCPrimitive3DS::renderVertices(const bool textured)
{
    CCRenderer::CCSetRenderStates( true );

	GLVertexPointer( 3, GL_FLOAT, 0, vertices, vertexCount );
    gRenderer->GLVertexAttribPointer( ATTRIB_NORMAL, 3, GL_FLOAT, true, 0, normals, vertexCount );
    CCSetTexCoords( adjustedUVs != NULL ? adjustedUVs : modelUVs );

	gRenderer->GLDrawArrays( GL_TRIANGLES, 0, vertexCount );
}
