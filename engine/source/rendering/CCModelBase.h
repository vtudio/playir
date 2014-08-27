/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCModelBase.h
 * Description : Represents the attributes of a renderable object.
 *
 * Created     : 01/03/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCMODELBASE_H__
#define __CCMODELBASE_H__


#include "CCRenderable.h"

class CCPrimitiveBase;

class CCModelBase : public CCRenderable
{
	typedef CCRenderable super;

protected:
    CCText modelID;

public:
    const char *shader;



public:
	CCModelBase(const long jsID=-1);
	virtual void destruct();

	// CCRenderable
	virtual void render(const bool alpha);

	void addModel(CCModelBase *model, const int index=-1);
    void removeModel(CCModelBase *model);
	void addPrimitive(CCPrimitiveBase *primitive);

    CCObjectPtrList<CCModelBase> models;
    CCObjectPtrList<CCPrimitiveBase> primitives;
};


#endif // __CCMODELBASE_H__
