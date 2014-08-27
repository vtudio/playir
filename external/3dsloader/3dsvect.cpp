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
 * Maths library for vectors management
 *  
 */

#include <math.h>
#include "3dsvect.h"


/**********************************************************
 *
 * SUBROUTINE VectCreate (p3d_ptr_type p_start, p3d_ptr_type p_end, p3d_ptr_type p_vector)
 *
 * This function creates a vector from two points
 *
 * Parameters: p_start = 3d point 1
 *	   		   p_end = 3d point 2
 *             p_vector = final vector
 *
 *********************************************************/

void VectCreate(p3d_type *p_start, p3d_type *p_end, p3d_type *p_vector )
{
    p_vector->x = p_end->x - p_start->x;
    p_vector->y = p_end->y - p_start->y;
    p_vector->z = p_end->z - p_start->z;
    VectNormalize(p_vector);
}



/**********************************************************
 *
 * FUNCTION VectLength (p3d_ptr_type p_vector)
 *
 * Returns the length of the vector
 *
 * Parameters: p_vector = vector
 *
 * Return value: (float) Length of the vector
 *
 *********************************************************/

float VectLength(p3d_type *p_vector)
{
	return (float)(sqrt(p_vector->x*p_vector->x + p_vector->y*p_vector->y + p_vector->z*p_vector->z));
}



/**********************************************************
 *
 * SUBROUTINE VectNormalize (p3d_ptr_type p_vector)
 *
 * This function Normalize a vector: all the three components x,y,z are scaled to 1
 *
 * Parameters: p_vector = vector
 *
 *********************************************************/

void VectNormalize(p3d_type *p_vector)
{
  float l_length;
  
  l_length = VectLength(p_vector);
  if (l_length==0) l_length=1;
  p_vector->x /= l_length;
  p_vector->y /= l_length;
  p_vector->z /= l_length;
}



/**********************************************************
 *
 * FUNCTION VectScalarProduct (p3d_ptr_type p_vector1,p3d_ptr_type p_vector2)
 *
 * Scalar product between two vectors
 *
 * Parameters: p_vector1 = vector1
 *			   p_vector2 = vector2
 *
 * Return value: (float) scalar product = vector1 x vector2
 *
 *********************************************************/

float VectScalarProduct(p3d_type *p_vector1, p3d_type *p_vector2)
{
    return (p_vector1->x*p_vector2->x + p_vector1->y*p_vector2->y + p_vector1->z*p_vector2->z);
}



/**********************************************************
 *
 * SUBROUTINE VectDotProduct (p3d_ptr_type p_vector1,p3d_ptr_type p_vector2,p3d_ptr_type p_normal)
 *
 * Calculate the dot product between p_vector1 and p_vector2 and stores the result in p_normal
 *
 * Parameters: p_vector1 = vector1
 *			   p_vector2 = vector2
 *             p_normal = dot product = vector1 dot vector2
 *
 *********************************************************/

void VectDotProduct(p3d_type *p_vector1, p3d_type *p_vector2, p3d_type *p_normal)
{
    p_normal->x=(p_vector1->y * p_vector2->z) - (p_vector1->z * p_vector2->y);
    p_normal->y=(p_vector1->z * p_vector2->x) - (p_vector1->x * p_vector2->z);
    p_normal->z=(p_vector1->x * p_vector2->y) - (p_vector1->y * p_vector2->x);
}
