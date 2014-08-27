/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCCollideable.h
 * Description : A scene managed collideable object
 *
 * Created     : 01/03/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

class CCMovementInterpolator;

class CCCollideable : public CCObject
{
	typedef CCObject super;

protected:
	uint drawOrder;
    bool collisionsEnabled;

public:
	uint collideableType;
	CCVector3 collisionBounds;
    CCSize collisionSize;
    CCSize inverseCollisionSize;

	CCVector3 aabbMin, aabbMax;
	bool updateCollisions;

	CCPtrList<CCOctree> octrees;

    bool visible;

protected:
    // The owner system allows us tie object to our lifespan, without modifying their collision or position
    CCCollideable *owner;
    CCPtrList<CCCollideable> owns;

#ifdef DEBUGON
    CCText debugName;
#endif

    CCMovementInterpolator *movementInterpolator;



public:
	CCCollideable(const char *objectID=NULL);
	virtual void destruct();

	// CCRenderable
	virtual void setPositionXYZ(const float x, const float y, const float z);
	virtual void translate(const float x, const float y, const float z);

	// CCObject
	virtual void setScene(CCSceneBase *scene);
	virtual void removeFromScene();
	virtual void deactivate();
    virtual bool shouldCollide(CCCollideable *collideWith, const bool initialCall);

    inline int getDrawOrder() const { return drawOrder; }
    void setDrawOrder(const int drawOrder)
    {
        this->drawOrder = drawOrder;
    }

    virtual void renderModel(const bool alpha);
protected:
	virtual void renderCollisionBox();

public:
    virtual const char* getType() const { return NULL; }

    void setSquareCollisionBounds(const float size);
    void setSquareCollisionBounds(const float width, const float heigth);
	void setHSquareCollisionBounds(const float hSize);
	void setHSquareCollisionBounds(const float hWidth, const float hHeight);

    void setCollisionBounds(const float width, const float height, const float depth);
	void setHCollisionBounds(const float hWidth, const float hHeight, const float hDepth);

	// Ask to report a collision to the collidedWith object
	virtual CCCollideable* requestCollisionWith(CCCollideable *collidedWith);

	// Ask the collidedWith object if we've collided
	virtual CCCollideable* recieveCollisionFrom(CCCollideable *collisionSource, const float x, const float y, const float z);

    virtual bool reportAttack(CCObject *attackedBy, const float force, const float damage, const float x, const float y, const float z);

    virtual bool isCollideable() { return collisionsEnabled; }
    virtual void setCollideable(const bool toggle) { collisionsEnabled = toggle; }
    virtual bool isMoveable() { return false; }

	virtual void ownObject(CCCollideable *object);
	virtual void unOwnObject(CCCollideable *object);

protected:
	void setOwner(CCCollideable *newOwner);

    // Called when parent object is removed from scene or deactivated
	virtual void removeOwner(CCCollideable *currentOwner);

public:
    inline const char* getDebugName()
    {
#ifdef DEBUGON
        return debugName.buffer;
#endif
        return "";
    }

    inline void setDebugName(const char *name)
    {
#ifdef DEBUGON
        debugName = name;
#endif
    }

    void createMovementInterpolator(const bool updateCollisions);

    CCMovementInterpolator* getMovementInterpolator()
    {
        return movementInterpolator;
    }

    float width()
    {
        return collisionSize.width;
    }

    float height()
    {
        return collisionSize.height;
    }
};
