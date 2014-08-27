/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCMovementInterpolator.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCObjects.h"
#include "CCMovementInterpolator.h"


CCMovementInterpolator::CCMovementInterpolator(CCCollideable *inObject, const bool updateCollisions)
{
    object = inObject;
    object->addUpdater( this );

    updating = false;
    this->updateCollisions = updateCollisions;
}



bool CCMovementInterpolator::update(const float delta)
{
    if( updating )
    {
        if( movementInterpolator.update( delta ) )
        {
            if( updateCollisions )
            {
                object->updateCollisions = true;
                CCOctreeRefreshObject( object );
            }
            object->dirtyModelMatrix();
            return true;
        }
        else
        {
            updating = false;
        }
    }
    return false;
}


void CCMovementInterpolator::clear()
{
    if( updating )
    {
        updating = false;
        movementInterpolator.clear();
    }
}


void CCMovementInterpolator::setMovement(const CCVector3 target, CCLambdaCallback *inCallback)
{
    updating = true;
    movementInterpolator.pushV3( &object->getPosition(), target, true, inCallback );
}


void CCMovementInterpolator::setMovementX(const float x)
{
    setMovement( CCVector3( x, object->getConstPosition().y, object->getConstPosition().z ) );
}


void CCMovementInterpolator::translateMovementX(const float x)
{
    setMovementX( object->getConstPosition().x + x );
}


void CCMovementInterpolator::setMovementY(const float y, CCLambdaCallback *inCallback)
{
    setMovement( CCVector3( object->getConstPosition().x, y, object->getConstPosition().z ), inCallback );
}


void CCMovementInterpolator::setMovementXY(const float x,const float y, CCLambdaCallback *inCallback)
{
    setMovement( CCVector3( x, y, object->getConstPosition().z ), inCallback );
}


void CCMovementInterpolator::setMovementYZ(const float y,const float z, CCLambdaCallback *inCallback)
{
    setMovement( CCVector3( object->getConstPosition().x, y, z ), inCallback );
}


const CCVector3 CCMovementInterpolator::getMovementTarget() const
{
	if( updating && movementInterpolator.interpolators.length > 0 )
    {
        return movementInterpolator.getTarget();
    }
    return object->getConstPosition();
}


void CCMovementInterpolator::setDuration(const float duration)
{
    movementInterpolator.setDuration( duration );
}