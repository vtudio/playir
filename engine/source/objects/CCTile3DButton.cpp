/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCTile3DButton.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCObjects.h"
#include "CCTextureFontPage.h"
#include "CCFileManager.h"


CCTile3DButton::CCTile3DButton(CCSceneBase *scene)
{
    construct( scene );
}


void CCTile3DButton::setupTile(const float size, const char *text)
{
    setupTile( size, size, text );
}


void CCTile3DButton::setupTile(const float width, const float height, const char *text)
{
    setTileSize( width, height );
    setTileTexture( "white.png", Resource_Packaged );

    if( text )
    {
        textObject->setText( text, height );
    }
}


void CCTile3DButton::setupTextured(const char *textureName, CCResourceType resourceType, CCLambdaCallback *onDownloadCallback, const CCTextureLoadOptions options)
{
    setTileSize();

    CCLAMBDA_2( DownloadedCallback, CCTile3DButton, tile, CCLambdaCallback*, nextCallback,
    {
        tile->setTileTexturedSize();

        if( nextCallback != NULL )
        {
            nextCallback->safeRun();
            delete nextCallback;
        }
    });
    setTileTexture( textureName, resourceType, new DownloadedCallback( this, onDownloadCallback ), options );
}


void CCTile3DButton::setupTexturedWidth(const float width, const char *textureName, const CCResourceType resourceType, const CCTextureLoadOptions options)
{
    setTileSize( width, 1.0f );
    setTileTexture( textureName, Resource_Packaged, NULL, options );
    setTileTexturedWidth( width );
}


void CCTile3DButton::setupTexturedHeight(const float height, const char *textureName, const CCResourceType resourceType, const CCTextureLoadOptions options)
{
    setTileSize();
    setTileTexture( textureName, Resource_Packaged, NULL, options );
    setTileTexturedHeight( height );
}


void CCTile3DButton::setupTexturedFit(const float width, const float height, const char *textureName, const CCTextureLoadOptions options)
{
	if( tileSquare == NULL )
    {
		setTileSize();
	}

    CCLAMBDA_3( DownloadedCallback, CCTile3DButton, tile, float, width, float, height,
    {
        tile->setTileTexturedFit( width, height );
    });

    setTileTexture( textureName, Resource_Unknown, new DownloadedCallback( this, width, height ), options );
}


void CCTile3DButton::setupText(const char *text, const float height, const bool centered, const bool backgroundTile)
{
    textObject->setText( text, height );

    const float width = textObject->getWidth();
    setCollisionBounds( width, height, CC_SMALLFLOAT );

    textObject->setCentered( centered );

    if( centered == false )
    {
        translate( collisionBounds.x, 0.0f, 0.0f );
    }

    if( backgroundTile )
    {
        setTileSize( width, height );
    }
}


void CCTile3DButton::construct(CCSceneBase *scene)
{
    CCAddFlag( collideableType, collision_ui );

    if( scene != NULL )
    {
        setScene( scene );
    }

    setTransparent();
    setReadDepth( false );

    setModel( new CCModelBase() );

    tileModel = new CCModelBase();
    model->addModel( tileModel );
    setColour( CCColour(), false );
    getColourInterpolator().setDuration( 0.5f );
    tileSquare = NULL;

    // Use an object to ensure the model is rendered in the transparent pass
    {
        textObject = new CCObjectText( this );
    }

    allowTouchRotation( false );
    touchRotationMagnitude = 0.0f;
    touchRotationSpeed = 1.0f;

    allowTouchMovement( false );

    touchDepressInterpolator.setDuration( 0.125f );
    setTouchDepressRange( 1.0f );
    setTouchDepressDepth( 3.0f );

    CCUpdateCollisions( this );

    createMovementInterpolator( true );

    tileRotationInterpolator = NULL;
}


void CCTile3DButton::destruct()
{
    super::destruct();

    DELETE_POINTER( tileRotationInterpolator );
}

// CCRenderable
void CCTile3DButton::refreshModelMatrix()
{
	if( updateModelMatrix )
	{
        CCMatrixLoadIdentity( modelMatrix );
        CCMatrixTranslate( modelMatrix, position.x, position.y, position.z );
        CCMatrixTranslate( modelMatrix, touchDepressPosition.x, touchDepressPosition.y, touchDepressPosition.z );

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

		if( touchRotationMagnitude != 0.0f )
        {
            CCMatrixRotateDegrees( modelMatrix, touchRotationMagnitude * 20.0f, 1.0f, 0.0f, 0.0f );
        }

		updateModelMatrix = false;
		dirtyWorldMatrix();
	}
}


void CCTile3DButton::setPositionXYZ(const float x, const float y, const float z)
{
    super::setPositionXYZ( x, y, z );
}


// CCObject
void CCTile3DButton::removeFromScene()
{
    inScene->removeTile( this );
    super::removeFromScene();
}


bool CCTile3DButton::update(const CCTime &time)
{
    bool updated = super::update( time );

    if( scale != NULL )
    {
        if( tileScaleInterpolator.update( time.delta ) )
        {
            dirtyModelMatrix();
            updated = true;
        }
    }

    // Touch depress: On update
    if( touchDepressRange > 0.0f )
    {
        if( touchDepressInterpolator.update( time.delta ) )
        {
            dirtyModelMatrix();
            updated = true;
        }
    }

    if( touchRotationAllowed )
    {
        const float speed = touching || touchReleased ? 3.0f : touchRotationSpeed;
        if( touchRotationInterpolator.update( time.delta * speed ) )
        {
            const float magnitudeSquared = rotation.x * rotation.x + rotation.y * rotation.y;
            touchRotationMagnitude = sqrtf( magnitudeSquared );
            touchRotationMagnitude = MIN( touchRotationMagnitude, 1.0f );

            dirtyModelMatrix();
            updated = true;
        }
    }

    if( tileRotationInterpolator != NULL )
    {
        if( tileRotationInterpolator->interpolators.length > 0 )
        {
            if( tileRotationInterpolator->update( time.delta ) )
            {
                dirtyModelMatrix();
                updated = true;
            }
        }
    }

    if( touching )
    {
        touchingTime += time.real;
    }
    else if( touchReleased )
    {
        if( touchDepressInterpolator.finished() && touchRotationInterpolator.finished() )
        {
            handleTouchRelease();
            updated = true;
        }
    }

    return updated;
}


void CCTile3DButton::renderModel(const bool alpha)
{
    super::renderModel( alpha );
}


void CCTile3DButton::setTileSize(const float size)
{
    setTileSize( size, size );
}


void CCTile3DButton::setTileSize(const float width, const float height)
{
    if( tileSquare == NULL )
    {
        tileSquare = new CCPrimitiveSquare();
        tileModel->addPrimitive( tileSquare );
    }

    setCollisionBounds( width, height, CC_SMALLFLOAT );
    CCUpdateCollisions( this );

    tileSquare->setScale( collisionSize.width, collisionSize.height );
}


void CCTile3DButton::setTileTexture(const char *textureName, CCResourceType resourceType, CCLambdaCallback *onDownloadCallback,
                                    const CCTextureLoadOptions options)
{
    if( tileSquare != NULL )
    {
        tileSquare->setTexture( textureName, resourceType, onDownloadCallback, options );
    }
}


void CCTile3DButton::setTileTexturedSize()
{
    CCASSERT( tileSquare != NULL && tileSquare->getTextureHandleIndex() != -1 );

	class OnLoadCallback : public CCLambdaSafeCallback
	{
	public:
		OnLoadCallback(CCTile3DButton *tile)
		{
			this->tile = tile;
			this->lazyPointer = tile->lazyPointer;
			this->lazyID = tile->lazyID;
		}
	protected:
		void run()
		{
			const CCTextureBase *texture = (const CCTextureBase*)runParameters;
            CCASSERT( texture != NULL );
			if( texture != NULL )
			{
				const float width = texture->getRawWidth();
				const float height = texture->getRawHeight();
				tile->setTileSize( width, height );
			}
		}
	private:
		CCTile3DButton *tile;
	};
    gEngine->textureManager->getTexture( tileSquare->getTextureHandleIndex(), new OnLoadCallback( this ) );
}


void CCTile3DButton::setTileTexturedWidth(const float width)
{
    CCASSERT( tileSquare != NULL && tileSquare->getTextureHandleIndex() != -1 );

	class OnLoadCallback : public CCLambdaSafeCallback
	{
	public:
		OnLoadCallback(CCTile3DButton *tile, float width)
		{
			this->tile = tile;
			this->lazyPointer = tile->lazyPointer;
			this->lazyID = tile->lazyID;
			this->width = width;
		}
	protected:
		void run()
		{
			const CCTextureBase *texture = (CCTextureBase*)runParameters;
            CCASSERT( texture != NULL );
			if( texture != NULL )
			{
				const float aspectRatio = texture->getRawWidth() / texture->getRawHeight();
				const float height = width / aspectRatio;
				tile->setTileSize( width, height );
			}
		}
	private:
		CCTile3DButton *tile;
		float width;
	};
    gEngine->textureManager->getTexture( tileSquare->getTextureHandleIndex(), new OnLoadCallback( this, width ) );
}


void CCTile3DButton::setTileTexturedHeight(const float height)
{
    CCASSERT( tileSquare != NULL && tileSquare->getTextureHandleIndex() != -1 );

	class OnLoadCallback : public CCLambdaSafeCallback
	{
	public:
		OnLoadCallback(CCTile3DButton *tile, float height)
		{
			this->tile = tile;
			this->lazyPointer = tile->lazyPointer;
			this->lazyID = tile->lazyID;
			this->height = height;
		}
	protected:
		void run()
		{
			const CCTextureBase *texture = (CCTextureBase*)runParameters;
            CCASSERT( texture != NULL );
			if( texture != NULL )
			{
				const float aspectRatio = texture->getRawWidth() / texture->getRawHeight();
				const float width = height * aspectRatio;
				tile->setTileSize( width, height );
			}
		}
	private:
		CCTile3DButton *tile;
		float height;
	};
    gEngine->textureManager->getTexture( tileSquare->getTextureHandleIndex(), new OnLoadCallback( this, height ) );
}


void CCTile3DButton::setTileTexturedFit(const float width, const float height, const bool crop)
{
    CCASSERT( tileSquare != NULL && tileSquare->getTextureHandleIndex() != -1 );

	class OnLoadCallback : public CCLambdaSafeCallback
	{
	public:
		OnLoadCallback(CCTile3DButton *tile, float width, float height, bool crop)
		{
			this->tile = tile;
			this->lazyPointer = tile->lazyPointer;
			this->lazyID = tile->lazyID;
			this->width = width;
			this->height = height;
			this->crop = crop;
		}
	protected:
		void run()
		{
			const CCTextureBase *texture = (CCTextureBase*)runParameters;
            CCASSERT( texture != NULL );
			if( texture != NULL )
			{
				const float targetAspectRatio = width / height;
				const float textureAspectRatio = texture->getRawWidth() / texture->getRawHeight();

				if( crop )
				{
					if( textureAspectRatio < targetAspectRatio )
					{
						tile->setTileTexturedWidth( width );
					}
					else
					{
						tile->setTileTexturedHeight( height );
					}
				}
				else
				{
					if( textureAspectRatio > targetAspectRatio )
					{
						tile->setTileTexturedWidth( width );
					}
					else
					{
						tile->setTileTexturedHeight( height );
					}
				}
			}
		}
	private:
		CCTile3DButton *tile;
		float width;
		float height;
		bool crop;
	};
    gEngine->textureManager->getTexture( tileSquare->getTextureHandleIndex(), new OnLoadCallback( this, width, height, crop ) );
}


void CCTile3DButton::setTileScale(const CCVector3 inScale, const bool interpolate, CCLambdaCallback *onInterpolated)
{
    if( scale == NULL )
    {
        setScale( 1.0f );
    }

    if( interpolate )
    {
        tileScaleInterpolator.setup( scale, inScale, onInterpolated );
    }
    else
    {
        *scale = inScale;
        tileScaleInterpolator.setup( scale, *scale );
        dirtyModelMatrix();
    }
}


void CCTile3DButton::adjustTextureUVs(const float x1, const float y1, const float x2, const float y2)
{
    const int textureHandleIndex = tileSquare->getTextureHandleIndex();
    CCTextureHandle *textureHandle = gEngine->textureManager->getTextureHandle( textureHandleIndex );
    const CCTextureBase *texture = textureHandle->texture;
    if( texture != NULL )
    {
        const float width = texture->getImageWidth();
        const float height = texture->getImageHeight();
        const float allocatedWidth = texture->getAllocatedWidth();
        const float allocatedHeight = texture->getAllocatedHeight();

        if( width == allocatedWidth && height == allocatedHeight )
        {
            if( tileSquare->customUVs != NULL )
            {
                DELETE_POINTER( tileSquare->customUVs );
            }
        }
        else
        {
        	CCPrimitiveSquareUVs::Setup( &tileSquare->customUVs,
                                         x1 / allocatedWidth,
                                         y1 / allocatedHeight,
                                         ( width / allocatedWidth ) - (x1 / allocatedWidth ),
                                         ( height / allocatedHeight ) - ( y1 / allocatedHeight )
                                        );
        }
    }
}


void CCTile3DButton::resetTileUVs()
{
    if( tileSquare != NULL )
    {
        if( tileSquare->customUVs != NULL )
        {
            DELETE_POINTER( tileSquare->customUVs );
            tileSquare->adjustTextureUVs();
        }
    }
}


void CCTile3DButton::flipTileY()
{
    if( tileSquare != NULL )
    {
        tileSquare->flipY();
        tileSquare->adjustTextureUVs();
    }
}


void CCTile3DButton::setTextPosition(const float x, const float y)
{
    textObject->setPositionX( x );
    textObject->setPositionY( y );
}


void CCTile3DButton::setTextColour(const CCColour &inColour, const bool interpolate, CCLambdaCallback *inCallback)
{
    if( interpolate )
    {
        textObject->getColourInterpolator().setDuration( 0.5f );
    }
    textObject->setColour( inColour, interpolate, inCallback );
}


void CCTile3DButton::setTextColourAlpha(const float inAlpha, const bool interpolate, CCLambdaCallback *inCallback)
{
    if( interpolate )
    {
        textObject->getColourInterpolator().setDuration( 0.5f );
    }
    textObject->setColourAlpha( inAlpha, interpolate, inCallback );
}


void CCTile3DButton::setTextBlinking(const bool toggle)
{
    if( toggle )
    {
        CCLAMBDA_1( AnimationFunction, CCTile3DButton, tile,
        {
            const float currentAlpha = tile->textObject->getColourInterpolator().getTarget().alpha;
            tile->textObject->setColourAlpha( currentAlpha == 1.0f ? 0.5f : 1.0f, true, new AnimationFunction( tile ) );
        });
        textObject->setColourAlpha( 1.0f, true, new AnimationFunction( this ) );
    }
    else
    {
        textObject->setColourAlpha( 1.0f, true, NULL );
    }
}


bool CCTile3DButton::handleProjectedTouch(const CCCameraProjectionResults &cameraProjectionResults,
                                          const CCCollideable *hitObject,
                                          const CCVector3 &hitPosition,
                                          const CCScreenTouches &touch,
                                          const CCTouchAction touchAction)
{
    if( collisionsEnabled == false )
    {
        return false;
    }

    if( hitObject == this &&
        ( touchAction == touch_pressed || ( touchMovementAllowed && CCControls::TouchActionMoving( touchAction ) ) ) )
    {
        if( touching == false )
        {
            touching = true;
            touchingTime = 0.0f;
            onTouchPress();
        }
    }

    if( touching )
    {
        const float maxTimeHeld = 0.125f;

        if( touchAction == touch_pressed )
        {
            if( touch.timeHeld >= maxTimeHeld )
            {
                CCVector3 relativeHitPosition;
                relativeHitPosition.x = hitPosition.x - position.x;
                relativeHitPosition.y = hitPosition.y - position.y;
                float x = relativeHitPosition.x / collisionBounds.x;
                float y = relativeHitPosition.y / collisionBounds.y;
                touchActionPressed( x, y, touch, touchAction );
            }
        }
        else if( touchMovementAllowed && CCControls::TouchActionMoving( touchAction ) )
        {
            if( hitObject != this )
            {
                return handleProjectedTouch( cameraProjectionResults, hitObject, hitPosition, touch, touch_lost );
            }
            else
            {
                CCVector3 relativeHitPosition;
                relativeHitPosition.x = hitPosition.x - position.x;
                relativeHitPosition.y = hitPosition.y - position.y;
                float x = relativeHitPosition.x / collisionBounds.x;
                float y = relativeHitPosition.y / collisionBounds.y;
                touchActionPressed( x, y, touch, touchAction );
            }
        }
        else
        {
            // Ensure we have a good touch release
            if( touchAction == touch_released )
            {
                if( touchMovementAllowed == false )
                {
                    const float absDeltaX = fabsf( touch.totalDelta.x );
                    const float absDeltaY = fabsf( touch.totalDelta.y );
                    if( absDeltaX > CCControls::GetTouchMovementThreashold().x || absDeltaY > CCControls::GetTouchMovementThreashold().y )
                    {
                        return handleProjectedTouch( cameraProjectionResults, hitObject, hitPosition, touch, touch_lost );
                    }
                }

                // If we have a good first touch, register it.
                CCVector3 relativeHitPosition;
                relativeHitPosition.x = hitPosition.x - position.x;
                relativeHitPosition.y = hitPosition.y - position.y;
                float x = relativeHitPosition.x / collisionBounds.x;
                float y = relativeHitPosition.y / collisionBounds.y;
                touchActionPressed( x, y, touch, touch_pressed );
            }

            touching = false;
            touchActionRelease( touchAction );

            if( touchAction == touch_released )
            {
                return true;
            }
        }
    }

    return false;
}


void CCTile3DButton::touchActionPressed(const float x, const float y, const CCScreenTouches &touch, const CCTouchAction touchAction)
{
    if( touchRotationAllowed )
    {
        float clampedX = x;
        float clampedY = y;
        CCFloatClamp( clampedX, -1.0f, 1.0f );
        CCFloatClamp( clampedY, -1.0f, 1.0f );
        touchRotationInterpolator.pushV3( &rotation, CCVector3( -clampedY, clampedX, 0.0f ), true );
    }

    // Touch depress: On pressed
    if( touchDepressRange > 0.0f )
    {
        // Touch depress range specifies the area which the button is pressed down
        // If it's out of range the button is pushed back up
        if( fabsf( x ) < touchDepressRange && fabsf( y ) < touchDepressRange )
        {
            touchDepressInterpolator.pushV3( &touchDepressPosition, CCVector3( 0.0f, 0.0f, -touchDepressDepth ), true );
        }
        else
        {
            touchDepressInterpolator.pushV3( &touchDepressPosition, CCVector3(), true );
        }
    }

    if( touchAction > touch_pressed && touchAction < touch_released )
    {
        touchActionMoved( x,  y,  touch, touchAction);
    }
}


void CCTile3DButton::touchActionMoved(const float x, const float y, const CCScreenTouches &touch, const CCTouchAction touchAction)
{
    onTouchMove();
}


void CCTile3DButton::touchActionRelease(const CCTouchAction touchAction)
{
    if( touchAction == touch_released )
    {
        touchReleased = true;
    }
    else
    {
        handleTouchRelease();
        if( touchAction == touch_lost )
        {
            onTouchLoss();
        }
    }
}


void CCTile3DButton::handleTouchRelease()
{
    // If the touch has been released successfully we fire our callback
    if( touchReleased )
    {
        touchReleased = false;
        onTouchRelease();
    }

    if( touchRotationAllowed )
    {
        touchRotationInterpolator.pushV3( &rotation, CCVector3() );
    }

    // Touch depress: On releasse
    if( touchDepressRange > 0.0f )
    {
        touchDepressInterpolator.pushV3( &touchDepressPosition, CCVector3(), true );
    }
}
