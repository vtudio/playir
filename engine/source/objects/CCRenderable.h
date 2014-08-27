/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCRenerable.h
 * Description : A renderable component.
 *
 * Created     : 01/03/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCRENDERABLE_H__
#define __CCRENDERABLE_H__


class CCRenderable : public CCBaseType
{
protected:
    long jsID;          // Used for js communication
                        // Note: If we create 300 objects a second we'll run out of ids after 82 days (2147483647/300)/60/60/24
    
    bool renderable;

    CCVector3 position;

    CCMatrix modelMatrix;
    bool updateModelMatrix;

    CCMatrix worldMatrix;
    bool updateWorldMatrix;

	CCColour *colour;

public:
    CCVector3 rotation;
    CCVector3 *scale;


public:
    typedef CCBaseType super;

    CCRenderable();

    // CCBaseType
    virtual void destruct();

    long getJSID() const
    {
        return jsID;
    }

    virtual void dirtyModelMatrix();
    virtual void dirtyWorldMatrix();

    bool shouldRender()
    {
        return renderable;
    }

    void setRenderable(const bool toggle)
    {
        renderable = toggle;
    }

    virtual void refreshModelMatrix();
    void refreshWorldMatrix(const CCMatrix *parentMatrix);

    CCMatrix& getModelMatrix() { return modelMatrix; }

    inline const CCVector3& getConstPosition() const { return position; }
    inline CCVector3& getPosition() { return position; }
    void setPosition(const CCVector3 &vector);
    virtual void setPositionXYZ(const float x, const float y, const float z);
    void setPositionX(const float x);
    void setPositionY(const float y);
    void setPositionZ(const float z);
    void setPositionXY(const float x, const float z);
    void setPositionXZ(const float x, const float z);
    void setPositionYZ(const float y, const float z);

    void translate(CCVector3 *vector);
    virtual void translate(const float x, const float y, const float z);

    virtual void rotationUpdated();
    void setRotation(const CCVector3 &vector);
    void setRotationY(const float y);
    void rotateX(const float x);
    void rotateY(const float y);
    void rotateZ(const float z);

    void setScale(const float value);
    void setScale(const float x, const float y, const float z);

    inline CCColour* getColour() { return colour; }
};


#endif // __CCRENDERABLE_H__
