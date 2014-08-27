/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCMoveable.h
 * Description : A scene managed moveable object.
 *
 * Created     : 01/03/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

class CCMoveable : public CCCollideable
{
public:
	typedef CCCollideable super;

	CCMoveable();

	// CCObject
    virtual bool update(const CCTime &time);

	// CCCollideable
	virtual CCCollideable* requestCollisionWith(CCCollideable *collidedWith);
    virtual bool isMoveable() { return true; }

	virtual void updateMovement(const float delta);
    virtual float applyMovementDirection(const float delta);

	virtual void applyVelocity(const float delta, const float movementMagnitude);
	float getCollisionPosition(const float thisObjectPosition, const float thisObjectBounds, const float collidedObjectPosition, const float collidedObjectBounds);
	CCCollideable* applyHorizontalVelocity(const float velocityX, const float velocityZ);
	CCCollideable* applyVerticalVelocity(const float increment);

	virtual void reportVerticalCollision(const CCCollideable *collidedWith);

	static void setGravityForce(const float force) { gravityForce = force; }

	inline void setVelocity(const float x, const float y, const float z) { movementVelocity.x = x; movementVelocity.y = y; movementVelocity.z = z; }
	inline void setAdditionalVelocity(const float x, const float y, const float z) { additionalVelocity.x = x; additionalVelocity.y = y; additionalVelocity.z = z; }
	inline void incrementAdditionalVelocity(const float x, const float y, const float z) { additionalVelocity.x += x; additionalVelocity.y += y; additionalVelocity.z += z; }

	bool moveable;
	float movementSpeed;
	CCVector3 movementDirection;

protected:
	CCVector3 velocity;
	CCVector3 movementVelocity;
	CCVector3 additionalVelocity;
	float decelerationSpeed;

	bool gravity;
	static float gravityForce;
};
