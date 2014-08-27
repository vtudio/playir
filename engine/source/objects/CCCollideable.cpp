/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCCollideable.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCObjects.h"
#include "CCSceneBase.h"
#include "CCTextureManager.h"
#include "CCMovementInterpolator.h"


CCCollideable::CCCollideable(const char *objectID)
{
    octreeRender = true;

	drawOrder = 100;
    collisionsEnabled = true;
	collideableType = 0;
	setSquareCollisionBounds( 1.0f );

	octrees.allocate( 32 );

    visible = false;

	owner = NULL;

    if( objectID != NULL )
    {
        this->objectID = objectID;
    }

    movementInterpolator = NULL;
}


void CCCollideable::destruct()
{
	super::destruct();
}


// CCRenderable
void CCCollideable::setPositionXYZ(const float x, const float y, const float z)
{
	super::setPositionXYZ( x, y, z );
    if( collideableType != collision_none )
    {
        updateCollisions = true;
        CCOctreeRefreshObject( this );
    }
}


void CCCollideable::translate(const float x, const float y, const float z)
{
	super::translate( x, y, z );
    if( collideableType != collision_none )
    {
        updateCollisions = true;
        CCOctreeRefreshObject( this );
    }
}


// CCObject
void CCCollideable::setScene(CCSceneBase *scene)
{
	super::setScene( scene );
	CCAddFlag( collideableType, collision_box );
    scene->addCollideable( this );
}


void CCCollideable::removeFromScene()
{
    inScene->removeCollideable( this );
	super::removeFromScene();
}


void CCCollideable::deactivate()
{
	super::deactivate();
	CCRemoveFlag( collideableType, collision_box );
	CCOctreeRemoveObject( this );

    if( owner != NULL )
	{
		owner->unOwnObject( this );
		owner = NULL;
	}

	for( int i=0; i<owns.length; ++i )
	{
		owns.list[i]->removeOwner( this );
	}
	owns.freeList();
}


bool CCCollideable::shouldCollide(CCCollideable *collideWith, const bool initialCall)
{
	// Ask myself if we should collide
    if( super::shouldCollide( collideWith, initialCall ) == false )
    {
        return false;
	}

	// Ask my owner if I should collide with this object
	if( owner != NULL && owner != parent )
    {
        if( owner->shouldCollide( collideWith, initialCall ) == false )
        {
            return false;
		}
	}

	// Ask the other object if we should collide
	if( initialCall )
    {
        return collideWith->shouldCollide( this, false );
	}

    // Yeah let's collide baby
    return true;
}


void CCCollideable::renderModel(const bool alpha)
{
	super::renderModel( alpha );

	if( alpha == transparent &&
        gRenderer->renderFlags & render_collisionBoxes &&
        CCHasFlag( collideableType, collision_box ) )
	{
		if( transparent == false )
		{
			CCRenderer::CCSetBlend( true );
		}

		renderCollisionBox();

		if( transparent == false )
		{
			CCRenderer::CCSetBlend( false );
		}
	}
}


void CCCollideable::renderCollisionBox()
{
	GLPushMatrix();
	{
        if( scale != NULL )
        {
            GLScalef( 1.0f / scale->x, 1.0f / scale->y, 1.0f / scale->z );
        }
		GLScalef( collisionSize.width, collisionSize.height, collisionSize.width );
		//glColor4f( 1.0f, 1.0f, 0.0f, 0.5f );
        //RenderCube( true );

		gEngine->textureManager->setTextureIndex( 1 );
		GLRotatef( -rotation.y, 0.0f, 1.0f, 0.0f );
        static CCColour colour( 1.0f, 0.0f, 0.0f, 0.5f );
		CCSetColour( colour );
        CCRenderCube( true );
	}
	GLPopMatrix();
}


void CCCollideable::setSquareCollisionBounds(const float size)
{
    setCollisionBounds( size, size, size );
}


void CCCollideable::setSquareCollisionBounds(const float width, const float height)
{
    setCollisionBounds( width, height, width );
}



void CCCollideable::setHSquareCollisionBounds(const float hSize)
{
	setHCollisionBounds( hSize, hSize, hSize );
}


void CCCollideable::setHSquareCollisionBounds(const float hWidth, const float hHeight)
{
	setHCollisionBounds( hWidth, hHeight, hWidth );
}


void CCCollideable::setCollisionBounds(const float width, const float height, const float depth)
{
    setHCollisionBounds( width * 0.5f, height * 0.5f, depth * 0.5f );
}


void CCCollideable::setHCollisionBounds(const float hWidth, const float hHeight, const float hDepth)
{
	collisionBounds.x = hWidth;
	collisionBounds.y = hHeight;
	collisionBounds.z = hDepth;

	collisionSize.width = hWidth > hDepth ? hWidth : hDepth;
    collisionSize.width *= 2.0f;
    collisionSize.height = hHeight * 2.0f;
    
	inverseCollisionSize.width = 1.0f / ( hWidth > hDepth ? hWidth : hDepth );
	inverseCollisionSize.height = 1.0f / collisionBounds.y;

    updateCollisions = true;
}


CCCollideable* CCCollideable::requestCollisionWith(CCCollideable *collidedWith)
{
	return collidedWith->recieveCollisionFrom( this, 0.0f, 0.0f, 0.0f );
}


CCCollideable* CCCollideable::recieveCollisionFrom(CCCollideable *collisionSource, const float x, const float y, const float z)
{
	return this;
}


bool CCCollideable::reportAttack(CCObject *attackedBy, const float force, const float damage, const float x, const float y, const float z)
{
    return false;
}


void CCCollideable::ownObject(CCCollideable *object)
{
	owns.add( object );
	object->setOwner( this );
}


void CCCollideable::unOwnObject(CCCollideable *object)
{
	if( owns.remove( object ) )
	{
		if( owns.length == 0 )
		{
			owns.freeList();
		}
	}
}


void CCCollideable::setOwner(CCCollideable *newOwner)
{
	owner = newOwner;
}


void CCCollideable::removeOwner(CCCollideable *currentOwner)
{
	if( currentOwner == owner )
	{
		owner = NULL;
	}
}


void CCCollideable::createMovementInterpolator(const bool updateCollisions)
{
    if( movementInterpolator == NULL )
    {
        movementInterpolator = new CCMovementInterpolator( this, updateCollisions );
    }
}