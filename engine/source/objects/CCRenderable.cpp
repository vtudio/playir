/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCRenerable.h
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCRenderable.h"


CCRenderable::CCRenderable()
{
    renderable = true;

    dirtyModelMatrix();
    updateWorldMatrix = true;

	scale = NULL;
    colour = NULL;
}


void CCRenderable::destruct()
{
    DELETE_POINTER( scale );

    DELETE_POINTER( colour );
}


void CCRenderable::dirtyModelMatrix()
{
    updateModelMatrix = true;
}


void CCRenderable::dirtyWorldMatrix()
{
	updateWorldMatrix = true;
}


void CCRenderable::refreshModelMatrix()
{
	if( updateModelMatrix )
	{
		CCMatrixLoadIdentity( modelMatrix );
        CCMatrixTranslate( modelMatrix, position.x, position.y, position.z );

		if( scale != NULL )
		{
			CCMatrixScale( modelMatrix, scale->x, scale->y, scale->z );
		}

		if( rotation.x != 0.0f )
		{
            CCMatrixRotateDegrees( modelMatrix, rotation.x, 1.0f, 0.0f, 0.0f );
		}

		if( rotation.y != 0.0f )
		{
            CCMatrixRotateDegrees( modelMatrix, rotation.y, 0.0f, 1.0f, 0.0f );
		}

		if( rotation.z != 0.0f )
		{
            CCMatrixRotateDegrees( modelMatrix, rotation.z, 0.0f, 0.0f, 1.0f );
		}

		updateModelMatrix = false;
		dirtyWorldMatrix();
	}
}


void CCRenderable::refreshWorldMatrix(const CCMatrix *parentMatrix)
{
    refreshModelMatrix();

	if( updateWorldMatrix )
	{
        if( parentMatrix )
        {
            CCMatrixMultiply( worldMatrix, modelMatrix, *parentMatrix );
        }
        else
        {
            worldMatrix = modelMatrix;
        }
    }
}


void CCRenderable::setPosition(const CCVector3 &vector)
{
    setPositionXYZ( vector.x, vector.y, vector.z );
}


void CCRenderable::setPositionXYZ(const float x, const float y, const float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
    dirtyModelMatrix();
}


void CCRenderable::setPositionX(const float x)
{
	setPositionXYZ( x, position.y, position.z );
}


void CCRenderable::setPositionY(const float y)
{
	setPositionXYZ( position.x, y, position.z );
}


void CCRenderable::setPositionZ(const float z)
{
	setPositionXYZ( position.x, position.y, z );
}


void CCRenderable::setPositionXY(const float x, const float y)
{
	setPositionXYZ( x, y, position.z );
}


void CCRenderable::setPositionXZ(const float x, const float z)
{
	setPositionXYZ( x, position.y, z );
}


void CCRenderable::setPositionYZ(const float y, const float z)
{
	setPositionXYZ( position.x, y, z );
}


void CCRenderable::translate(CCVector3 *vector)
{
	translate( vector->x, vector->y, vector->z );
}


void CCRenderable::translate(const float x, const float y, const float z)
{
	position.x += x;
	position.y += y;
	position.z += z;
    dirtyModelMatrix();
}


void CCRenderable::rotationUpdated()
{
    dirtyModelMatrix();
}


void CCRenderable::setRotation(const CCVector3 &vector)
{
    rotation = vector;
    rotationUpdated();
}


void CCRenderable::setRotationY(const float y)
{
    rotation.y = y;
    rotationUpdated();
}


void CCRenderable::rotateX(const float x)
{
	rotation.x += x;
    CCClampRotation( rotation.x );
    rotationUpdated();
}


void CCRenderable::rotateY(const float y)
{
	rotation.y += y;
    CCClampRotation( rotation.y );
    rotationUpdated();
}


void CCRenderable::rotateZ(const float z)
{
	rotation.z += z;
    CCClampRotation( rotation.z );
    rotationUpdated();
}


void CCRenderable::setScale(const float value)
{
    setScale( value, value, value );
}


void CCRenderable::setScale(const float x, const float y, const float z)
{
    CCVector3FillPtr( &scale, x, y, z );
    dirtyModelMatrix();
}
