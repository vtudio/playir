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
 * Math library for vectors management
 * 
 * File header
 *  
 */

#ifndef __3DSVECT_H__
#define __3DSVECT_H__


/**********************************************************
 *
 * TYPES DECLARATION
 *
 *********************************************************/

struct p3d_type
{
    float x,y,z;
};



/**********************************************************
 *
 * FUNCTIONS DECLARATION
 *
 *********************************************************/

extern void VectCreate(p3d_type *p_start, p3d_type *p_end, p3d_type *p_vector);
extern float VectLength(p3d_type *p_vector);
extern void VectNormalize(p3d_type *p_vector);
extern float VectScalarProduct(p3d_type *p_vector1, p3d_type *p_vector2);
extern void VectDotProduct(p3d_type *p_vector1, p3d_type *p_vector2, p3d_type *p_normal);


#endif