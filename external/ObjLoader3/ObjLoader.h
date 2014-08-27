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

/*
**	Sentry, prevent the header file from being included more than once
*/
#ifndef _OBJ_LOADER_H_
#define _OBJ_LOADER_H_

#ifdef __cplusplus
extern "C" {
#endif
    
    /*
     **	A wee typedef that holds a reference to the objfile
     */
	typedef unsigned int ObjFile;
    
/*===================================================================================================
 **
 **										Internal Data Structures
 **
 **=================================================================================================*/


/*
 **	To start with, only deal with the obj file as a list of points. Essentually,
 **	if we are able to draw the points out they should roughly describe the mesh.
 **	So, lets look for the 'v' flag that says that the 3 floats after it describes a
 **	Vertex. Essentually then, we need to have a structure for a vertex.
 */
typedef struct
{
    float x,			/*	The x component of the vertex position	*/
    y,			/*	The y component of the vertex position	*/
    z;			/*	The z component of the vertex position	*/
} ObjVertex;


/*
 **	Now, the normal vector flag 'vn' could also do with a structure to represent it,
 **	this will ultimately look very similar to the Vertex structure seeing as they have
 **	similar data
 */
typedef struct
{
    float x,			/*	The x component of the normal vector	*/
    y,			/*	The y component of the normal vector	*/
    z;			/*	The z component of the normal vector	*/
} ObjNormal;


/*
 **	A Texturing co-ordinate usually has only two values, the u and the v. Make a third
 **	struct for this fact.
 */
typedef struct
{
    float u,			/*	The u parametric texturing co-ordinate	*/
    v;			/*	The v parametric texturing co-ordinate	*/
} ObjTexCoord;


/*
 **	Each face is composed of a number of `corners`. At each `corner` there will be at a
 **	minimum, one vertex coordinate, and possibly a normal vector and a texturing co-ordinate.
 **	Seeing as quite often a specific vertex is used by a number of faces, rather than store the
 **	vertices in the face structure itself, we can use an integer reference to the position of the
 **	vertex in the array. Basically it allows us the ability of storing the data as a 4byte value
 **	rather than the 12 needed for a vertex.
 **
 **	To summaraise,
 **
 **		Each face is comprised of a number of `corners`, in this case the number will be held in
 **	the value "m_iVertexCount". Taking into account the worst case scenario, for each corner, we
 **	could have a vertex, normal and a texturing co-ordinate (usual actually). So we may need to hold
 **	3 * m_iVertexCount number of indices for our faces. It kindof makes sense to basically dynamically
 **	allocate an array to hold the vertex indicies, an array for the normal indices and one for the uv
 **	coords. These are m_aVertexIndices, m_aNormalIndices, and m_aTexCoordIndicies respectivley.
 **
 **	So our full face structure looks a bit like this :
 */
typedef struct
{
    unsigned int *m_aVertexIndices,	/*	array of indicies that reference the vertex array in the mesh	*/
    *m_aNormalIndices,              /*	array of indicies that reference the normal array in the mesh	*/
    *m_aTexCoordIndicies;           /*	array of indicies that reference the uv coordinate array in the mesh	*/
    unsigned int  m_iVertexCount;	/*	the number of vertices that make up this mesh, ie, 3 = triangle; 4 = quad etc */
} ObjFace;

/*
 **	Each mesh is held as a structure with arrays of it's vertices, normals and texturing co-ordinates.
 **	An Array of faces then references the arrays. There is also a pointer to the next node in the linked
 **	list.
 */
typedef struct _ObjMesh
{
    ObjVertex		*m_aVertexArray;		/*	Array of vertices that make up this mesh	*/
    ObjNormal		*m_aNormalArray;		/*	Array of normals that make up this mesh		*/
    ObjTexCoord		*m_aTexCoordArray;		/*	Array of texturing co-ordinates that make up this mesh */
    ObjFace			*m_aFaces;				/*	Array of faces that make up this mesh */
    
    unsigned int	 m_iNumberOfVertices,	/*	The number of vertices in the m_aVertexArray array	*/
    m_iNumberOfNormals,	/*	The number of normals in the m_aNormalArray array	*/
    m_iNumberOfTexCoords,	/*	The number of uv's in the m_aTexCoordArray array	*/
    m_iNumberOfFaces;		/*	The number of faces in the m_aFaces array			*/
    
    struct _ObjMesh *m_pNext;				/*	Each mesh will be stored internally as a node on a linked list */
    ObjFile			 m_iMeshID;				/*	the ID of the mesh	*/
    
} ObjMesh;

	/*
	**	func	:	LoadOBJ
	**	params	:	filename	-	the name of the file you wish to load
	**	returns	:	a reference ID for this file. Will return 0 if the file failed to load
	**	notes	:	loads the *.obj file up and stores all the data internally to make the
	**				source nice and easy to use. Each objfile loaded is stored as a node on
	**				a linked list and uses a single unsigned integer as an ID to allow you
	**				to reference it.
	*/
	extern ObjMesh*	LoadOBJ(const char *filename, const CCResourceType resourceType);
	extern ObjMesh*	LoadOBJTextData(CCText &fileData);

	/*
	**	func	:	DeleteOBJ
	**	params	:	id	-	a reference ID to the mesh you wish to delete
	**	returns	:	nothing
	**	notes	:	Deletes the requested mesh node from the linked list, and frees all the data
	*/
	extern void		DeleteOBJ(ObjFile id);

	/*
	**	func	:	CleanUpOBJ
	**	returns	:	nothing
	**	notes	:	Deletes all the currently loaded data. Handy for cleaning up all data easily
	*/
	extern void		CleanUpOBJ(void);

	/*
	**	func	:	SetLightPosition
	**	param	:	lx	-	the lights x coord
	**	param	:	ly	-	the lights y coord
	**	param	:	lz	-	the lights z coord
	**	returns	:	nothing
	**	notes	:	Deletes all the currently loaded data. Handy for cleaning up all data easily
	*/
	extern void SetLightPosition(float lx,float ly,float lz);


	extern void SetTextures(ObjFile id,const char BumpTex[],const char BaseTex[]);

	/*=========================================================================================
	**
	**	This section handle conversion from the immediate mode format to one of three other
	**	OpenGL rendering methods, display lists, vertex arrays or vertex buffer objects
	**
	**=======================================================================================*/

	#define DISPLAY_LIST 1
	#define VERTEX_ARRAY 2
	#define VERTEX_BUFFER_OBJECT 3

	/*
	**	func	:	ConvertMeshTo
	**	params	:	id	-	a reference ID to the mesh you wish to convert to either a display list or vertex array
	**				What	-	either the defines DISPLAY_LIST, VERTEX_ARRAY or VERTEX_BUFFER_OBJECT
	**	returns	:	nothing
	**	notes	:	Changes the representation of the Mesh node so that it either uses Immediate mode
	**				Display lists or vertex arrays. Bear in mind that once the mesh has been converted,
	**				It cannot be converted to another format.
	*/
	extern void		ConvertMeshTo(ObjFile id,unsigned char What);

#ifdef __cplusplus
}
#endif

#endif
