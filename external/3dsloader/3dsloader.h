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
 * File header: 3dsloader.h
 *  
 */

/**********************************************************
 *
 * TYPES DECLARATION
 *
 *********************************************************/

// TODO: Optimize down vertices
#define MAX_VERTICES 8000 // Max number of vertices (for each object)
#define MAX_POLYGONS 12000 // Max number of polygons (for each object)

// Our vertex type
typedef struct{
    float x,y,z;
}vertex_type;

// The polygon (triangle), 3 numbers that aim 3 vertices
typedef struct{
    int a,b,c;
}polygon_type;

// The mapcoord type, 2 texture coordinates for each vertex
typedef struct{
    float u,v;
}mapcoord_type;

// The object type
typedef struct {
	char name[20];
    
	int vertices_qty;
    int polygons_qty;
	
    vertex_type vertex[MAX_VERTICES]; 
    vertex_type normal[MAX_VERTICES];
    polygon_type polygon[MAX_POLYGONS];
    mapcoord_type mapcoord[MAX_VERTICES];
} obj3ds_type;


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

extern int Load3DS(obj3ds_type *p_object, const char *p_filename);

extern void Calc3DSNormals(obj3ds_type *p_object);

