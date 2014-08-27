/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCModel3DS.h
 * Description : Loads and handles a 3ds model.
 *
 * Created     : 05/08/11
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCMODEL3DS_H__
#define __CCMODEL3DS_H__

#include "3dsloader.h"

class CCPrimitive3DS : public CCPrimitive3D
{
    typedef CCPrimitive3D super;

public:
    CCPrimitive3DS();
    virtual void destruct();

    bool load(const char *file);

public:
	virtual void renderVertices(const bool textured);
};

class CCModel3DS : public CCModelBase
{
public:
    typedef CCModelBase super;

	CCModel3DS(const char *file,
               const char *texture1, const CCResourceType resourceType1, const CCTextureLoadOptions options1,
               const char *texture2, const CCResourceType resourceType2, const CCTextureLoadOptions options2);

    float getWidth() { return primitive3ds->getWidth(); }
    float getHeight() { return primitive3ds->getHeight(); }
    float getDepth() { return primitive3ds->getDepth(); }

public:
	CCPrimitive3DS *primitive3ds;
};

#endif // __CCMODEL3DS_H__
