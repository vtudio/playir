/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCPrimitiveOBJ.h
 * Description : Loads and handles an obj model
 *
 * Created     : 26/12/11
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCPRIMITIVEOBJ_H__
#define __CCPRIMITIVEOBJ_H__


#include "ObjLoader.h"


class CCPrimitiveOBJ : public CCPrimitive3D
{
    typedef CCPrimitive3D super;

public:
    CCPrimitiveOBJ();
    virtual void destruct();

	static void LoadOBJ(const char *file, const CCResourceType resourceType, CCLambdaCallback *callback);
    virtual bool loadData(const char *fileData);
protected:
	bool loadOBJMesh(ObjMesh *objMesh);

    // PrimitiveBase
public:
	virtual void renderVertices(const bool textured);

    virtual void copy(const CCPrimitiveOBJ *primitive);
};


#endif // __CCPRIMITIVEOBJ_H__
