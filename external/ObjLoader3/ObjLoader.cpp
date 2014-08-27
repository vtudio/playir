/*===================================================================================================
**
**	Author	:	Robert Bateman
**	E-Mail	:	rbateman@bournemouth.ac.uk
**	Brief	:	This Sourcefile is part of a series explaining how to load and render Alias Wavefront
**				Files somewhat efficently. If you are simkply after a reliable Obj Loader, then I would
**				Recommend version8; or possibly version9 and the supplied loader for extreme efficency.
**
**	Note	:	This Source Code is provided as is. No responsibility is accepted by myself for any
**				damage to hardware or software caused as a result of using this code. You are free to
**				make any alterations you see fit to this code for your own purposes, and distribute
**				that code either as part of a source code or binary executable package. All I ask is
**				for a little credit somewhere for my work!
** 
**				Any source improvements or bug fixes should be e-mailed to myself so I can update the
**				code for the greater good of the community at large. Credit will be given to the 
**				relevant people as always....
**				
**
**				Copyright (c) Robert Bateman, www.robthebloke.org, 2004
**
**				
**				National Centre for Computer Animation,
**				Bournemouth University,
**				Talbot Campus,
**				Bournemouth,
**				BH3 72F,
**				United Kingdom
**				ncca.bournemouth.ac.uk
**	
**
===================================================================================================*/


/*===================================================================================================
**
**	This version now reads in any n sided polygon. The face structure has become a tad dynamic
**	in order to render the polygons with GL_POLYGON
**
===================================================================================================*/


/*===================================================================================================
**
**												Includes
**
**=================================================================================================*/

#include <stdio.h>
#include <stdlib.h>

#include "CCPlatform.h"
#include "CCTools.h"
#include "CCMathTools.h"
#include "CCFileManager.h"

#include <string.h>

#include "ObjLoader.h"


/* Ignored until Version6 */
void ConvertMeshTo(ObjFile id,unsigned char What)
{
}
void SetLightPosition(float lx,float ly,float lz)
{
}
void SetTextures(ObjFile id,const char BumpTex[],const char BaseTex[])
{
}


/*===================================================================================================
**
**										Global Variables
**
**=================================================================================================*/

/*
**	The global head of the linked list of meshes. This is a linked list because it is possible that you will be
**	loading and deleting meshes during the course of the programs execution.
*/
ObjMesh *g_LinkedListHead = NULL;

/* 
**	This is used to generate a unique ID for each Obj File
*/
unsigned int g_ObjIdGenerator=0;

/*
**	This function is only called from within the *.c file and is used to create an ObjMesh structure and
**	initialise its values (adds the mesh to the linked list also).
*/
ObjMesh *MakeOBJ( void )
{
	/*
	**	The pointer we will create the mesh at the end of
	*/
	ObjMesh *pMesh = NULL;


	pMesh = (ObjMesh*) malloc (sizeof(ObjMesh));

	/*	If the program asserts here, then there was a memory allocation failure	*/
    CCASSERT(pMesh);

	/*
	**	Initialise all pointers to NULL
	*/
	pMesh->m_aFaces				= NULL;
	pMesh->m_aNormalArray		= NULL;
	pMesh->m_aTexCoordArray		= NULL;
	pMesh->m_aVertexArray		= NULL;
	pMesh->m_iNumberOfFaces		= 0;
	pMesh->m_iNumberOfNormals	= 0;
	pMesh->m_iNumberOfTexCoords = 0;
	pMesh->m_iNumberOfVertices	= 0;
	pMesh->m_iMeshID			= ++g_ObjIdGenerator;


	/*
	**	Insert the mesh at the beginning of the linked list
	*/
	pMesh->m_pNext				= g_LinkedListHead;
	g_LinkedListHead			= pMesh;

	return pMesh;
}

ObjMesh* LoadOBJ(const char *filePath, const CCResourceType resourceType)
{
    CCText *fileData = new CCText();
    {
        CCFileManager *file = CCFileManager::File( resourceType );

        /*
        **	Open the file for reading
        */
        if( file->open( filePath ) == false )
        {
            DEBUGLOG( "Error loading %s: ", filePath );
            
            delete file;
            DELETE_POINTER( fileData );
            return NULL;
        }
        
        const uint fileSize = file->size();
        if( fileSize == 0 )
        {
            file->close();
            delete file;
            DELETE_POINTER( fileData );
            return NULL;
        }
        fileData->setSize( fileSize+1 );
        file->read( fileData->buffer, fileSize );
        fileData->buffer[fileSize] = 0;
        file->close();
        delete file;
    }
    
    ObjMesh *mesh = LoadOBJTextData( *fileData );
    DELETE_POINTER( fileData );
    
    return mesh;
}


ObjMesh* LoadOBJTextData(CCText &fileData)
{
    CCPtrList<char> lines;
    fileData.split( lines, "\n" );

	/*
	**	Create the mesh structure and add it to the linked list
	*/
	ObjMesh *pMesh = MakeOBJ();

	/*
	**	Run through the whole file looking for the various flags so that we can count
	**	up how many data elements there are. This is done so that we can make one memory
	**	allocation for the meshes data and then run through the file once more, this time
	**	reading in the data. It's purely done to reduce system overhead of memory allocation due
	**	to otherwise needing to reallocate data everytime we read in a new element.
	*/
    for( int i=0; i<lines.length; ++i )
    {
		/*	Grab a line at a time	*/
        char *buffer = lines.list[i];

		/*	look for the 'vn' - vertex normal - flag	*/
		if( strncmp( "vn ", buffer, 3 ) == 0 )
		{
			++pMesh->m_iNumberOfNormals;
		}
        
		/*	look for the 'vt' - texturing co-ordinate - flag  */
		else if( strncmp( "vt ", buffer, 3 ) == 0 )
		{
			++pMesh->m_iNumberOfTexCoords;
		}
        
		/*	look for the 'v ' - vertex co-ordinate - flag  */
		else if( strncmp( "v ", buffer, 2 ) == 0 )
		{
			++pMesh->m_iNumberOfVertices;
		}
        
		/*	look for the 'f ' - face - flag  */
		else if( strncmp( "f ", buffer, 2 ) == 0 )
		{
			++pMesh->m_iNumberOfFaces;
		}
	}

	/*
	**	Allocate the memory for the data arrays and check that it allocated ok
	*/
    CCASSERT( pMesh->m_iNumberOfVertices > 0 )
	pMesh->m_aVertexArray = (ObjVertex*)calloc( pMesh->m_iNumberOfVertices, sizeof( ObjVertex )	);
	if( pMesh->m_aVertexArray == NULL )
    {
        DeleteOBJ( pMesh->m_iMeshID );
        return NULL;
    }

	/*	there are occasionally times when the obj does not have any normals in it */
	if( pMesh->m_iNumberOfNormals > 0 )
	{
		pMesh->m_aNormalArray = (ObjNormal*)calloc( pMesh->m_iNumberOfNormals, sizeof( ObjNormal )	);
        if( pMesh->m_aNormalArray == NULL )
        {
            DeleteOBJ( pMesh->m_iMeshID );
            return NULL;
        }
	}

	/*	there are occasionally times when the obj does not have any tex coords in it */
	if( pMesh->m_iNumberOfTexCoords > 0 )
	{
		pMesh->m_aTexCoordArray = (ObjTexCoord*)calloc( pMesh->m_iNumberOfTexCoords, sizeof( ObjTexCoord ) );
        if( pMesh->m_aTexCoordArray == NULL )
        {
            DeleteOBJ( pMesh->m_iMeshID );
            return NULL;
        }
	}

	pMesh->m_aFaces = (ObjFace*)calloc( pMesh->m_iNumberOfFaces, sizeof( ObjFace ) );
    if( pMesh->m_aFaces == NULL )
    {
        DeleteOBJ( pMesh->m_iMeshID );
        return NULL;
    }

	unsigned int vc=0,nc=0,tc=0,fc=0;
    CCText buffer;
    CCText value;
    CCPtrList<char> values;
    CCPtrList<char> splitValues;
    for( int lineIndex=0; lineIndex<lines.length; ++lineIndex )
    {
		/*	Grab a line at a time	*/
        buffer = lines.list[lineIndex];
        
        if( lineIndex > lines.length-2 )
        {
            buffer = lines.list[lineIndex];
        }
        
        values.clear();
        buffer.split( values, " " );
        
        // Check for bad data
        if( values.length > 1 )
        {
            const char *value = values.list[1];
            if( CCText::Equals( value, "" ) )
            {
                values.removeIndex( 1 );
            }
        }
        
        const char *value0 = values.list[0];

		/*	look for the 'vn' - vertex normal - flag	*/
		if( CCText::Equals( value0, "vn" ) )
		{
            CCASSERT( nc < pMesh->m_iNumberOfNormals );
            pMesh->m_aNormalArray[ nc ].x = (float)atof( values.list[1] );
            pMesh->m_aNormalArray[ nc ].y = (float)atof( values.list[2] );
            pMesh->m_aNormalArray[ nc ].z = (float)atof( values.list[3] );
			++nc;
		}
        
		/*	look for the 'vt' - texturing co-ordinate - flag  */
		else if( CCText::Equals( value0, "vt" ) )
		{
            CCASSERT( tc < pMesh->m_iNumberOfTexCoords );
            pMesh->m_aTexCoordArray[ tc ].u = (float)atof( values.list[1] );
            pMesh->m_aTexCoordArray[ tc ].v = (float)atof( values.list[2] );
			++tc;
		}
        
		/*	look for the 'v ' - vertex co-ordinate - flag  */
		else if( CCText::Equals( value0, "v" ) )
		{
            CCASSERT( vc < pMesh->m_iNumberOfVertices );
            const float x = (float)atof( values.list[1] );
            const float y = (float)atof( values.list[2] );
            const float z = (float)atof( values.list[3] );
            pMesh->m_aVertexArray[ vc ].x = x;
            pMesh->m_aVertexArray[ vc ].y = y;
            pMesh->m_aVertexArray[ vc ].z = z;
			++vc;
		}
        
		/*	look for the 'f ' - face - flag  */
		else if( CCText::Equals( value0, "f" ) )
		{
			/*
			**	Pointer to the face we are currently dealing with. It's only used so that
			**	the code becomes more readable and I have less to type.
			*/
            if( fc >= pMesh->m_iNumberOfFaces )
            {
                DeleteOBJ( pMesh->m_iMeshID );
                return NULL;
            }
			ObjFace *pf = &pMesh->m_aFaces[ fc ];

			/*
			**	These next few lines are used to figure out how many '/' characters there
			**	are in the string. This gives us the information we need to find out how
			**	many vertices are used in this face (by dividing by two)
			*/
            unsigned int ii = 0;
			for( uint i=0; i<buffer.length; i++ )
			{
				if( buffer.buffer[i] == '/' )
                {
					ii++;
                }
			}
            
            if( ii > 0 )
            {
                /*
                **	Allocate the indices for the vertices of this face
                */
                pf->m_aVertexIndices	= (unsigned int*)calloc( ii, sizeof(unsigned int) );

                /*
                **	Allocate the indices for the normals of this face only if the obj file
                **	has normals stored in it.
                */
                if( pMesh->m_iNumberOfNormals > 0 )
                {
                    pf->m_aNormalIndices	= (unsigned int*)calloc( ii, sizeof(unsigned int) );
                }

                /*
                **	Allocate the indices for the texturing co-ordinates of this face only if the obj file
                **	has texturing co-ordinates stored in it.
                */
                if( pMesh->m_iNumberOfTexCoords > 0 )
                {
                    pf->m_aTexCoordIndicies = (unsigned int*)calloc( ii, sizeof(unsigned int) );
                }

                /*
                **	tokenise the string using strtok(). Basically this splits the string up
                **	and removes the spaces from each chunk. This way we only have to deal with
                **	one set of indices at a time for each of the poly's vertices.
                */
                uint vertexIndex=0;
                for( int face=1; face<values.length; ++face )
                {
                    value = values.list[face];
                    
                    splitValues.clear();
                    value.split( splitValues, "/" );
                    if( splitValues.length > 1 )
                    {
                        if( tc > 0 && nc > 0 )
                        {
                            pf->m_aVertexIndices[vertexIndex] = atoi( splitValues.list[0] );
                            pf->m_aTexCoordIndicies[vertexIndex] = atoi( splitValues.list[1] );
                            pf->m_aNormalIndices[vertexIndex] = atoi( splitValues.list[2] );
                            
                            /* need to reduce the indices by 1 because array indices start at 0, obj starts at 1  */
                            if( pf->m_aTexCoordIndicies[vertexIndex] > 0 )
                            {
                                --pf->m_aTexCoordIndicies[vertexIndex];
                            }
                            
                            if( pf->m_aNormalIndices[vertexIndex] > 0 )
                            {
                                --pf->m_aNormalIndices[vertexIndex];
                            }
                            
                            if( pf->m_aTexCoordIndicies[vertexIndex] >= pMesh->m_iNumberOfTexCoords )
                            {
                                DeleteOBJ( pMesh->m_iMeshID );
                                return NULL;
                            }
                        }
                        else if( tc > 0 )
                        {
                            pf->m_aVertexIndices[vertexIndex] = atoi( splitValues.list[0] );
                            pf->m_aTexCoordIndicies[vertexIndex] = atoi( splitValues.list[1] );
                            
                            /* need to reduce the indices by 1 because array indices start at 0, obj starts at 1  */
                            if( pf->m_aTexCoordIndicies[vertexIndex] > 0 )
                            {
                                --pf->m_aTexCoordIndicies[vertexIndex];
                            }
                            
                            if( pf->m_aTexCoordIndicies[vertexIndex] >= pMesh->m_iNumberOfTexCoords )
                            {
                                DeleteOBJ( pMesh->m_iMeshID );
                                return NULL;
                            }
                        }
                        else if( nc > 0 )
                        {
                            pf->m_aVertexIndices[vertexIndex] = atoi( splitValues.list[0] );
                            pf->m_aNormalIndices[vertexIndex] = atoi( splitValues.list[1] );
                            
                            /* need to reduce the indices by 1 because array indices start at 0, obj starts at 1  */
                            if( pf->m_aNormalIndices[vertexIndex] > 0 )
                            {
                                --pf->m_aNormalIndices[vertexIndex];
                            }
                            
                            if( pf->m_aNormalIndices[vertexIndex] >= pMesh->m_iNumberOfNormals )
                            {
                                DeleteOBJ( pMesh->m_iMeshID );
                                return NULL;
                            }
                        }
                        
                        /* need to reduce the indices by 1 because array indices start at 0, obj starts at 1  */
                        if( pf->m_aVertexIndices[vertexIndex] > 0 )
                        {
                            --pf->m_aVertexIndices[vertexIndex];
                        }
                        
                        vertexIndex++;
                    }
                }
                
                if( ii < vertexIndex )
                {
                    DeleteOBJ( pMesh->m_iMeshID );
                    return NULL;
                }
                pf->m_iVertexCount = vertexIndex;
                ++fc;
            }
		}
	}

	return pMesh;
}


/*
**	Calling free() on NULL is VERY BAD in C, so make sure we
**	check all pointers before calling free.
*/
void DeleteMesh(ObjMesh* pMesh)
{
	/*
	**	If the pointer is valid
	*/
	if(pMesh)
	{
		/*	delete the face array */
		if(pMesh->m_aFaces)
		{
            // Delete allocated verts/normals/uvs
            for( uint i=0; i<pMesh->m_iNumberOfFaces; ++i )
            {
                ObjFace *pf = &pMesh->m_aFaces[i];
                if( pf )
                {
					if( pf->m_aVertexIndices )
					{
						free( pf->m_aVertexIndices );
					}
					if( pf->m_aNormalIndices )
					{
						free( pf->m_aNormalIndices );
					}
					if( pf->m_aTexCoordIndicies )
					{
						free( pf->m_aTexCoordIndicies );
					}
                }
            }
            
            free(pMesh->m_aFaces);
            pMesh->m_aFaces = NULL;
		}

		/*	delete the vertex array */
		if(pMesh->m_aVertexArray)
		{
			free(pMesh->m_aVertexArray);
			pMesh->m_aVertexArray = NULL;
		}

		/*	delete the normal array */
		if(pMesh->m_aNormalArray)
		{
			free(pMesh->m_aNormalArray);
			pMesh->m_aNormalArray = NULL;
		}

		/*	delete the texturing co-ordinate array */
		if(pMesh->m_aTexCoordArray)
		{
			free(pMesh->m_aTexCoordArray);
			pMesh->m_aTexCoordArray = NULL;
		}

		/*	free the mesh */
		free( pMesh );
	}
}

void DeleteOBJ(ObjFile id)
{
	/*
	**	Create two pointers to walk through the linked list
	*/
	ObjMesh *pCurr,
			*pPrev = NULL;

	/*
	**	Start traversing the list from the start
	*/
	pCurr = g_LinkedListHead;

	/*
	**	Walk through the list until we either reach the end, or
	**	we find the node we are looking for
	*/
	while(pCurr != NULL && pCurr->m_iMeshID != id)
	{
		pPrev = pCurr;
		pCurr = pCurr->m_pNext;
	}

	/*
	**	If we found the node that needs to be deleted
	*/
	if(pCurr != NULL)
	{
		/*
		**	If the pointer before it is NULL, then we need to
		**	remove the first node in the list
		*/
		if(pPrev == NULL)
		{
			g_LinkedListHead = pCurr->m_pNext;
		}

		/*
		**	Otherwise we are removing a node from somewhere else
		*/
		else
		{
			pPrev->m_pNext = pCurr->m_pNext;
		}

		/*
		**	Free the memory allocated for this mesh
		*/
		DeleteMesh(pCurr);
	}
}

/*
**	Delete all of the meshes starting from the front.
*/
void CleanUpOBJ(void)
{
	ObjMesh *pCurr;
	while(g_LinkedListHead != NULL)
	{
		pCurr = g_LinkedListHead;
		g_LinkedListHead = g_LinkedListHead->m_pNext;
		DeleteMesh(pCurr);
	}
}
