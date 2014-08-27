/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCMoveable.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCObjects.h"
#include "CCSceneBase.h"


float CCMoveable::gravityForce = 200.0f;


CCMoveable::CCMoveable()
{
	moveable = true;
	movementSpeed = 75.0f;
	decelerationSpeed = movementSpeed * 2.0f;

	gravity = true;
}


// CCObject
bool CCMoveable::update(const CCTime &time)
{
	super::update( time );
    updateMovement( time.delta );
    return true;
}


// CCCollideable
CCCollideable* CCMoveable::requestCollisionWith(CCCollideable *collidedWith)
{
	return collidedWith->recieveCollisionFrom( this, velocity.x, velocity.y, velocity.z );
}


void CCMoveable::updateMovement(const float delta)
{
    if( moveable )
    {
        const float movementMagnitude = applyMovementDirection( delta );
        velocity = movementVelocity;

        const float additionalMagnitude = CCVector3LengthSquared( additionalVelocity );
        if( additionalMagnitude > 0.0f )
        {
            velocity.add( additionalVelocity );
            const float deceleration = decelerationSpeed * delta;
            CCToTarget( additionalVelocity.x, 0.0f, deceleration );
            CCToTarget( additionalVelocity.y, 0.0f, deceleration );
            CCToTarget( additionalVelocity.z, 0.0f, deceleration );
        }

        const float velocityMagnitude = CCVector3LengthSquared( velocity );
        if( velocityMagnitude > 0.0f )
        {
            applyVelocity( delta, movementMagnitude );

            dirtyModelMatrix();
            updateCollisions = true;
            CCOctreeRefreshObject( this );
        }
    }
}


float CCMoveable::applyMovementDirection(const float delta)
{
	const float movementMagnitude = CCLengthSquared( movementDirection.x, movementDirection.z );
	if( movementMagnitude > 0.0f )
	{
		// Z movement
		const float forwardSpeed = movementSpeed;
		const float zMovementSpeed = movementDirection.z * forwardSpeed;
		float rotationInRadians = CC_DEGREES_TO_RADIANS( rotation.y );
		float xAmount = sinf( rotationInRadians ) * zMovementSpeed;
		float zAmount = cosf( rotationInRadians ) * zMovementSpeed;

		// X Movement
		if( movementDirection.x != 0.0f )
		{
			const float xMovementSpeed = movementDirection.x * movementSpeed;
			rotationInRadians = CC_DEGREES_TO_RADIANS( rotation.y + 90.0f );
			xAmount += sinf( rotationInRadians ) * xMovementSpeed;
			zAmount += cosf( rotationInRadians ) * xMovementSpeed;
		}

		movementVelocity.x = xAmount;
		movementVelocity.z = zAmount;
	}
    else
    {
        movementVelocity = 0.0f;
    }

	return movementMagnitude;
}


void CCMoveable::applyVelocity(const float delta, const float movementMagnitude)
{
	const float velocityX = velocity.x * delta;
	const float velocityZ = velocity.z * delta;
	if( velocityX != 0.0f || velocityZ != 0.0f )
	{
		const CCCollideable *collidedWith = NULL;

		const float velocityVsBoundingX = velocityX * inverseCollisionSize.width;
		const float velocityVsBoundingZ = velocityZ * inverseCollisionSize.width;
		const float absVelocityVsBoundingX = fabsf( velocityVsBoundingX );
		const float absVelocityVsBoundingZ = fabsf( velocityVsBoundingZ );
		if( absVelocityVsBoundingX > 1.0f || absVelocityVsBoundingZ > 1.0f )
		{
			const float furthestIncrement = absVelocityVsBoundingX > absVelocityVsBoundingZ ? absVelocityVsBoundingX : absVelocityVsBoundingZ;
			const uint numberOfIncrements = (uint)( roundf( furthestIncrement + 0.5f ) );

			const float inverseNumberOfIncrements = 1.0f / numberOfIncrements;
			const float incrementsX = velocityX * inverseNumberOfIncrements;
			const float incrementsZ = velocityZ * inverseNumberOfIncrements;
			uint i = 0;
			do
			{
				collidedWith = applyHorizontalVelocity( incrementsX, incrementsZ );
				i++;
			} while( i < numberOfIncrements && collidedWith == NULL );
		}
		else
		{
			collidedWith = applyHorizontalVelocity( velocityX, velocityZ );
		}

		// Deceleration
		if( movementMagnitude == 0.0f )
		{
			const float movementDeceleration = decelerationSpeed * delta;
            CCToTarget( movementVelocity.x, 0.0f, movementDeceleration );
            CCToTarget( movementVelocity.z, 0.0f, movementDeceleration );
		}
	}

	// Gravity
	if( gravity )
	{
		const CCCollideable *collidedWith = NULL;
		movementVelocity.y -= gravityForce * delta;
		const float velocityY = velocity.y * delta;

		if( velocityY > 0.0f )
		{
			const float velocityVsBoundingY = velocityY * inverseCollisionSize.height;
			const float absVelocityVsBoundingY = fabsf( velocityVsBoundingY );
			if( absVelocityVsBoundingY > 1.0f )
			{
				uint numberOfIncrements = (uint)( roundf( absVelocityVsBoundingY + 0.5f ) );
				const float velocityIncrements = velocityY / numberOfIncrements;
				uint i = 0;
				do
				{
					collidedWith = applyVerticalVelocity( velocityIncrements );
					i++;
				} while( i < numberOfIncrements && collidedWith == NULL );
			}
			else
			{
				collidedWith = applyVerticalVelocity( velocityY );
			}
		}
		else
		{
			const float velocityVsBoundingY = velocityY * inverseCollisionSize.height;
			const float absVelocityVsBoundingY = fabsf( velocityVsBoundingY );
			if( absVelocityVsBoundingY > 1.0f )
			{
				uint numberOfIncrements = (uint)( roundf( absVelocityVsBoundingY + 0.5f ) );
				const float velocityIncrements = velocityY / numberOfIncrements;
				uint i = 0;
				do
				{
					collidedWith = applyVerticalVelocity( velocityIncrements );
					i++;
				} while( i < numberOfIncrements && collidedWith == NULL );
			}
			else
			{
				collidedWith = applyVerticalVelocity( velocityY );
			}
		}

		reportVerticalCollision( collidedWith );

		// Ensure we have a velocity so we're checked for movement next frame
		if( collidedWith == NULL && movementVelocity.y == 0.0f )
		{
			movementVelocity.y = CC_SMALLFLOAT;
		}
	}
}


float CCMoveable::getCollisionPosition(const float thisObjectPosition, const float thisObjectBounds, const float collidedObjectPosition, const float collidedObjectBounds)
{
	float collisionPosition = collidedObjectPosition;
	if( collisionPosition < thisObjectPosition )
	{
		collisionPosition += collidedObjectBounds;
		collisionPosition += thisObjectBounds;
		collisionPosition += CC_SMALLFLOAT;

		if( collisionPosition > thisObjectPosition )
		{
			collisionPosition = thisObjectPosition;
		}
	}
	else
	{
		collisionPosition -= collidedObjectBounds;
		collisionPosition -= thisObjectBounds;
		collisionPosition -= CC_SMALLFLOAT;

		if( collisionPosition < thisObjectPosition )
		{
			collisionPosition = thisObjectPosition;
		}
	}

	return collisionPosition;
}


CCCollideable* CCMoveable::applyHorizontalVelocity(const float velocityX, const float velocityZ)
{
	position.x += velocityX;
	position.z += velocityZ;

	CCCollideable *collidedWith = CCOctreeCollisionCheck( this, position, true );
	if( collidedWith != NULL )
	{
		position.x -= velocityX;

		CCCollideable *collidedWithZ = collidedWith;
		if( velocityZ != 0.0f )
		{
			collidedWithZ = CCOctreeCollisionCheck( this, position, true );
		}

		if( collidedWithZ != NULL )
		{
			position.z -= velocityZ;
			position.x += velocityX;

			CCCollideable *collidedWithX = collidedWith;
			if( velocityX != 0.0f )
			{
				collidedWithX = CCOctreeCollisionCheck( this, position, true );
			}

			if( collidedWithX != NULL )
			{
				position.x -= velocityX;

				// Moving failed

				// Kill the additional velocity
				additionalVelocity.x = 0.0f;
				additionalVelocity.z = 0.0f;
			}

			// Moving X passed
			else
			{
				float collisionZ = getCollisionPosition( position.z, collisionBounds.z, collidedWith->getPosition().z, collidedWith->collisionBounds.z );
				position.z = collisionZ;

				// Kill the z additional velocity
				additionalVelocity.z = 0.0f;
			}
		}
		// Moving Z passed
		else
		{
			float collisionX = getCollisionPosition( position.x, collisionBounds.x, collidedWith->getPosition().x, collidedWith->collisionBounds.x );
			position.x = collisionX;

			// Kill the x additional velocity
			additionalVelocity.x = 0.0f;
		}
	}

	return collidedWith;
}


CCCollideable* CCMoveable::applyVerticalVelocity(const float increment)
{
	position.y += increment;

	CCCollideable *collidedWith = CCOctreeCollisionCheck( this, position, true );
	if( collidedWith != NULL )
	{
		position.y -= increment;
		const float originalPosition = position.y;

		// Since basic collision check doesn't return the actual closest collision, keep re-running the check until we're collision free
		CCCollideable *currentlyCollidingWith = collidedWith;
		while( currentlyCollidingWith != NULL )
		{
			position.y = originalPosition;
			collidedWith = currentlyCollidingWith;
			position.y = getCollisionPosition( position.y, collisionBounds.y, collidedWith->getPosition().y, collidedWith->collisionBounds.y );
			if( position.y == originalPosition )
			{
				currentlyCollidingWith = NULL;
			}
			else
			{
				currentlyCollidingWith = CCOctreeCollisionCheck( this, position );
			}
		}
	}

	return collidedWith;
}


void CCMoveable::reportVerticalCollision(const CCCollideable *collidedWith)
{
	if( collidedWith )
	{
		movementVelocity.y = 0.0f;
	}
}
