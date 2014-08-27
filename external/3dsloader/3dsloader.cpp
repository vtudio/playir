/*
 * ---------------- www.spacesimulator.net --------------
 *   ---- Space simulators and 3d engine tutorials ----
 *
 * Author: Damiano Vitulli
 *
 * This program is released under the BSD licence
 * By using this program you agree to licence terms on spacesimulator.net copyright page
 *
 *
 * 3ds models loader
 * 
 * Include File: 3dsloader.cpp
 *  
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "3dsvect.h"
#include "3dsloader.h"

#include "CCTools.h"
#include "CCFileManager.h"


/**********************************************************
 *
 * FUNCTION Load3DS (obj_type_ptr, char *)
 *
 * This function loads a mesh from a 3ds file.
 * Please note that we are loading only the vertices, polygons and mapping lists.
 * If you need to load meshes with advanced features as for example: 
 * multi objects, materials, lights and so on, you must insert other chunk parsers.
 *
 *********************************************************/

int Load3DS(obj3ds_type *p_object, const char *p_filename)
{
	int i; //Index variable
	
	unsigned short l_chunk_id; // Chunk identifier
	unsigned int l_chunk_length; //Chunk length

	unsigned char l_char; // Char variable
	unsigned short l_qty; // Number of elements in each chunk

	unsigned short l_face_flags; //Flag that stores some face information

    CCFileManager *file = CCFileManager::File( Resource_Packaged );

	//Open the file
	if( file->open( p_filename ) == false )
	{
        DEBUGLOG( "Error loading %s: ", p_filename );
        
		delete file;
		return 0;
	}

    while( file->endOfFile() == false )
	{
        file->read( &l_chunk_id, 2 );
		//DEBUGLOG( "ChunkID: %x\n", l_chunk_id );
        file->read( &l_chunk_length, 4 ); //Read the length of the chunk
		//DEBUGLOG( "ChunkLenght: %x\n", l_chunk_length );

		switch( l_chunk_id )
		{
			//----------------- MAIN3DS -----------------
			// Description: Main chunk, contains all the other chunks
			// Chunk ID: 4d4d
			// Chunk Lenght: 0 + sub chunks
			//-------------------------------------------
			case 0x4d4d:
			break;

			//----------------- EDIT3DS -----------------
			// Description: 3D Editor chunk, objects layout info
			// Chunk ID: 3d3d (hex)
			// Chunk Lenght: 0 + sub chunks
			//-------------------------------------------
			case 0x3d3d:
			break;

			//--------------- EDIT_OBJECT ---------------
			// Description: Object block, info for each object
			// Chunk ID: 4000 (hex)
			// Chunk Lenght: len(object name) + sub chunks
			//-------------------------------------------
			case 0x4000:
				i=0;
				do
				{
					file->read( &l_char, 1 );
					p_object->name[i] = l_char;
					i++;
				} while( l_char != '\0' && i<20 );
			break;

			//--------------- OBJ_TRIMESH ---------------
			// Description: Triangular mesh, contains chunks for 3d mesh info
			// Chunk ID: 4100 (hex)
			// Chunk Lenght: 0 + sub chunks
			//-------------------------------------------
			case 0x4100:
			break;

			//--------------- TRI_VERTEXL ---------------
			// Description: Vertices list
			// Chunk ID: 4110 (hex)
			// Chunk Lenght: 1 x unsigned short (number of vertices)
			//             + 3 x float (vertex coordinates) x (number of vertices)
			//             + sub chunks
			//-------------------------------------------
			case 0x4110:
				file->read( &l_qty, sizeof (unsigned short) );
                CCASSERT( l_qty < MAX_VERTICES );
				p_object->vertices_qty = l_qty;
				DEBUGLOG( "Number of vertices: %d\n", l_qty );
				for( i=0; i<l_qty; ++i )
				{
					//DEBUGLOG( "Vertex %i\n", i );
                    //fflush( stdout );
                    vertex_type &vertex = p_object->vertex[i];
                    
					file->read( &vertex.x, sizeof(float) );
					//DEBUGLOG( "Vertices list x: %f\n",vertex[i].x );
					file->read( &vertex.y, sizeof(float) );
					//DEBUGLOG( "Vertices list y: %f\n",vertex[i].y );
					file->read( &vertex.z, sizeof(float) );
					//DEBUGLOG( "Vertices list z: %f\n",p_object->vertex[i].z );
				}
				break;

			//--------------- TRI_FACEL1 ----------------
			// Description: Polygons (faces) list
			// Chunk ID: 4120 (hex)
			// Chunk Lenght: 1 x unsigned short (number of polygons)
			//             + 3 x unsigned short (polygon points) x (number of polygons)
			//             + sub chunks
			//-------------------------------------------
			case 0x4120:
			{
				file->read( &l_qty, sizeof (unsigned short) );
                CCASSERT( l_qty < MAX_POLYGONS );
				p_object->polygons_qty = l_qty;
				DEBUGLOG("Number of polygons: %d\n",l_qty);
				for (i=0; i<l_qty; i++)
				{
                    polygon_type &polygon = p_object->polygon[i];
                    
					file->read( &polygon.a, sizeof (unsigned short)  );
					//DEBUGLOG( "Polygon point a: %d\n",polygon.a );
                    CCASSERT( polygon.a >= 0 && polygon.a < p_object->vertices_qty );

					file->read( &polygon.b, sizeof (unsigned short)  );
					//DEBUGLOG("Polygon point b: %d\n",polygon.b);
                    CCASSERT( polygon.b >= 0 && polygon.b < p_object->vertices_qty );

					file->read( &polygon.c, sizeof (unsigned short)  );
					//DEBUGLOG("Polygon point c: %d\n",polygon.c);
                    CCASSERT( polygon.c >= 0 && polygon.c < p_object->vertices_qty );

					file->read( &l_face_flags, sizeof (unsigned short)  );
					//DEBUGLOG("Face flags: %x\n",l_face_flags);
				}
#ifdef DEBUGON
				fflush( stdout );
#endif
				break;
			}

			//------------- TRI_MAPPINGCOORS ------------
			// Description: Vertices list
			// Chunk ID: 4140 (hex)
			// Chunk Lenght: 1 x unsigned short (number of mapping points)
			//             + 2 x float (mapping coordinates) x (number of mapping points)
			//             + sub chunks
			//-------------------------------------------
			case 0x4140:
				file->read( &l_qty, sizeof (unsigned short) );
                CCASSERT( l_qty < MAX_VERTICES );
                CCASSERT( l_qty == p_object->vertices_qty );
				for (i=0; i<l_qty; i++)
				{
					file->read( &p_object->mapcoord[i].u, sizeof (float) );
					//DEBUGLOG("Mapping list u: %f\n",p_object->mapcoord[i].u);
					file->read( &p_object->mapcoord[i].v, sizeof (float) );
					//DEBUGLOG("Mapping list v: %f\n",p_object->mapcoord[i].v);
				}
				break;

			//----------- Skip unknow chunks ------------
			//We need to skip all the chunks that currently we don't use
			//We use the chunk lenght information to set the file pointer
			//to the same level next chunk
			//-------------------------------------------
			default:
			{
                file->seek( l_chunk_length-6 );
			}
		}
	}
    
	file->close();
	delete file;
    
    return 1; // Returns ok
}


/**********************************************************
 *
 * SUBROUTINE ObjCalcNormals(obj_type_ptr p_object)
 *
 * This function calculate all the polygons and vertices' normals of the specified object
 *
 * Input parameters: p_object = object
 *
 *********************************************************/

void Calc3DSNormals(obj3ds_type *p_object)
{
	int i;
	p3d_type l_vect1,l_vect2,l_vect3,l_vect_b1,l_vect_b2,l_normal;  //Some local vectors
	int l_connections_qty[MAX_VERTICES]; //Number of poligons around each vertex
    
    // Resetting vertices' normals...
	for( i=0; i<p_object->vertices_qty; ++i )
	{
		p_object->normal[i].x = 0.0f;
		p_object->normal[i].y = 0.0f;
		p_object->normal[i].z = 0.0f;
		l_connections_qty[i]=0;
	}
	
	for (i=0; i<p_object->polygons_qty; i++)
	{
        l_vect1.x = p_object->vertex[p_object->polygon[i].a].x;
        l_vect1.y = p_object->vertex[p_object->polygon[i].a].y;
        l_vect1.z = p_object->vertex[p_object->polygon[i].a].z;
        l_vect2.x = p_object->vertex[p_object->polygon[i].b].x;
        l_vect2.y = p_object->vertex[p_object->polygon[i].b].y;
        l_vect2.z = p_object->vertex[p_object->polygon[i].b].z;
        l_vect3.x = p_object->vertex[p_object->polygon[i].c].x;
        l_vect3.y = p_object->vertex[p_object->polygon[i].c].y;
        l_vect3.z = p_object->vertex[p_object->polygon[i].c].z;         
        
        // Polygon normal calculation
		VectCreate( &l_vect1, &l_vect2, &l_vect_b1 ); // Vector from the first vertex to the second one
        VectCreate ( &l_vect1, &l_vect3, &l_vect_b2 ); // Vector from the first vertex to the third one
        VectDotProduct( &l_vect_b1, &l_vect_b2, &l_normal ); // Dot product between the two vectors
        VectNormalize( &l_normal ); //Normalizing the resultant we obtain the polygon normal
        
		l_connections_qty[p_object->polygon[i].a]+=1; // For each vertex shared by this polygon we increase the number of connections
		l_connections_qty[p_object->polygon[i].b]+=1;
		l_connections_qty[p_object->polygon[i].c]+=1;
        
		p_object->normal[p_object->polygon[i].a].x+=l_normal.x; // For each vertex shared by this polygon we add the polygon normal
		p_object->normal[p_object->polygon[i].a].y+=l_normal.y;
		p_object->normal[p_object->polygon[i].a].z+=l_normal.z;
		p_object->normal[p_object->polygon[i].b].x+=l_normal.x;
		p_object->normal[p_object->polygon[i].b].y+=l_normal.y;
		p_object->normal[p_object->polygon[i].b].z+=l_normal.z;
		p_object->normal[p_object->polygon[i].c].x+=l_normal.x;
		p_object->normal[p_object->polygon[i].c].y+=l_normal.y;
		p_object->normal[p_object->polygon[i].c].z+=l_normal.z;	
	}	
	
    for (i=0; i<p_object->vertices_qty; i++)
	{
		if (l_connections_qty[i]>0)
		{
			p_object->normal[i].x /= l_connections_qty[i]; // Let's now average the polygons' normals to obtain the vertex normal!
			p_object->normal[i].y /= l_connections_qty[i];
			p_object->normal[i].z /= l_connections_qty[i];
		}
	}
}
