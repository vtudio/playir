/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCPrimitiveSphere.h
 * Description : Sphere drawable component.
 *
 * Created     : 01/03/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCPRIMITIVESPHERE_H__
#define __CCPRIMITIVESPHERE_H__


class CCPrimitiveSphere : public CCPrimitiveBase
{
public:
	typedef CCPrimitiveBase super;

	// CCPrimitiveBase
	virtual void renderVertices(const bool textured);

	void setup(const float radius);
protected:
	uint vertexCount;
};


#endif // __CCPRIMITIVESPHERE_H__
