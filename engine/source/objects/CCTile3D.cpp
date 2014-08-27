/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCTile3D.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCObjects.h"
#include "CCTextureFontPage.h"
#include "CCMovementInterpolator.h"


CCTile3D::CCTile3D()
{
    setDebugName( "Tile3D" );
}


void CCTile3D::destruct()
{
    // Delete objects from CCTouchable
    onPress.deleteObjectsAndList();
    onMove.deleteObjectsAndList();
    onRelease.deleteObjectsAndList();
    onLoss.deleteObjectsAndList();

    super::destruct();
}


// CCRenderable
void CCTile3D::dirtyModelMatrix()
{
    super::dirtyModelMatrix();
    for( int i=0; i<attachments.length; ++i )
    {
        CCObject *attachment = attachments.list[i];
        attachment->dirtyModelMatrix();
    }
}


void CCTile3D::setPositionXYZ(const float x, const float y, const float z)
{
    if( position.x != x || position.y != y || position.z != z )
    {
        CCVector3 distance = position;
        super::setPositionXYZ( x, y, z );
        distance.sub( position );

        for( int i=0; i<attachments.length; ++i )
        {
            CCObject *attachment = attachments.list[i];
            attachment->translate( -distance.x, -distance.y, -distance.z );
        }

        CCOctreeRefreshObject( this );

        if( movementInterpolator != NULL )
        {
            movementInterpolator->clear();
        }
    }
}


void CCTile3D::translate(const float x, const float y, const float z)
{
    super::translate( x, y, z );

    for( int i=0; i<attachments.length; ++i )
    {
        CCObject *attachment = attachments.list[i];
        attachment->translate( x, y, z );
    }

    CCOctreeRefreshObject( this );
}


// Positioning Tiles
void CCTile3D::positionTileY(float &y)
{
    y -= collisionBounds.y;
    translate( 0.0f, y, 0.0f );
    y -= collisionBounds.y;
}


void CCTile3D::positionTileBelow(CCTile3D *fromTile)
{
    setPosition( fromTile->getConstPosition() );
    translate( 0.0f, -( fromTile->collisionBounds.y + collisionBounds.y ), 0.0f );
}


void CCTile3D::positionTileAbove(CCTile3D *fromTile)
{
    setPosition( fromTile->getConstPosition() );
    translate( 0.0f, fromTile->collisionBounds.y + collisionBounds.y, 0.0f );
}


void CCTile3D::positionTileRight(CCTile3D *fromTile)
{
    setPosition( fromTile->getConstPosition() );
    translate( fromTile->collisionBounds.x + collisionBounds.x, 0.0f, 0.0f );
}


void CCTile3D::positionTileLeft(CCTile3D *fromTile)
{
    setPosition( fromTile->getConstPosition() );
    translate( -( fromTile->collisionBounds.x + collisionBounds.x ), 0.0f, 0.0f );
}


void CCTile3D::setTileMovement(const CCVector3 target)
{
    movementInterpolator->setMovement( target );
}


void CCTile3D::setTileMovementX(const float x)
{
    movementInterpolator->setMovementX( x );
}


void CCTile3D::translateTileMovementX(const float x)
{
    movementInterpolator->translateMovementX( x );
}


void CCTile3D::setTileMovementY(const float y)
{
    movementInterpolator->setMovementY( y );
}


void CCTile3D::setTileMovementXY(const float x, const float y)
{
    movementInterpolator->setMovementXY( x, y );
}


void CCTile3D::setTileMovementYZ(const float y, const float z)
{
    movementInterpolator->setMovementYZ( y, z );
}


void CCTile3D::setTileMovementBelow(const CCTile3D *fromTile)
{
    CCVector3 target = fromTile->getTileMovementTarget();
    target.y -= ( fromTile->collisionBounds.y + collisionBounds.y );
    movementInterpolator->setMovement( target );
}


const CCVector3 CCTile3D::getTileMovementTarget() const
{
    return movementInterpolator->getMovementTarget();
}