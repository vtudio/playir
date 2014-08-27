/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCTile3DFrameBuffer.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"


CCPtrList<CCTile3DFrameBuffer> CCTile3DFrameBuffer::frameBuffers;
uint CCTile3DFrameBuffer::renderedThisFrame = 0;


CCTile3DFrameBuffer::CCTile3DFrameBuffer(CCSceneBase *scene) :
    CCTile3DButton( scene )
{
    frameBufferID = -1;
    sceneControlsActive = false;

    firstRender = true;
}


void CCTile3DFrameBuffer::destruct()
{
    super::destruct();
}


// CCObject
bool CCTile3DFrameBuffer::update(const CCTime &time)
{
    bool updated = false;

    super::update( time );

    if( visible )
    {
        if( sceneControlsActive )
        {
            // Update our touch logic
            for( int i=0; i<CCControls::max_touches; ++ i)
            {
                CCControls::UpdateTouch( touches[i], time );
            }
        }

        for( int i=0; i<linkedScenes.length; ++i )
        {
            CCSceneBase *scene = linkedScenes.list[i];
            updated |= scene->updateTask( time );
        }
    }

    return updated;
}


void CCTile3DFrameBuffer::renderModel(const bool alpha)
{
    if( firstRender )
    {
        firstRender = false;
        gEngine->renderFrameBuffer( frameBufferID );
    }
    else if( renderedThisFrame < 3 )
    {
        if( frameBuffers.length == 0 )
        {
            renderedThisFrame++;
            gEngine->renderFrameBuffer( frameBufferID );
        }
        else
        {
            // Pop out our requests and render
            CCTile3DFrameBuffer *frameBuffer = frameBuffers.pop();
            renderedThisFrame++;
            gEngine->renderFrameBuffer( frameBuffer->frameBufferID );
        }
    }
    else
    {
        // Queue up all the frame buffer render requests
        if( frameBuffers.find( this ) == -1 )
        {
            frameBuffers.add( this );
        }
    }

    super::renderModel( alpha );
}


bool CCTile3DFrameBuffer::handleProjectedTouch(const CCCameraProjectionResults &cameraProjectionResults,
                                               const CCCollideable *hitObject,
                                               const CCVector3 &hitPosition,
                                               const CCScreenTouches &touch,
                                               const CCTouchAction touchAction)
{
    if( collisionsEnabled == false )
    {
        return false;
    }

    if( sceneControlsActive )
    {
        return handleSceneTouch( cameraProjectionResults, hitObject, hitPosition, touch, touchAction );
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


bool CCTile3DFrameBuffer::handleSceneTouch(const CCCameraProjectionResults &cameraProjectionResults,
                                           const CCCollideable *hitObject,
                                           const CCVector3 &hitPosition,
                                           const CCScreenTouches &screenTouch,
                                           const CCTouchAction touchAction)
{
    CCVector3 relativeHitPosition;
    relativeHitPosition.x = hitPosition.x - position.x;
    relativeHitPosition.y = hitPosition.y - position.y;

    // Top left is 0,0
    float x = ( relativeHitPosition.x / collisionSize.width ) + 0.5f;
    float y = 1.0f - ( ( relativeHitPosition.y / collisionSize.height ) + 0.5f );

    CCScreenTouches &touch = touches[0];

    if( touchAction == touch_released )
    {
        touchReleased = true;

        touch.lastTouch = screenTouch.lastTouch;
        touch.lastTimeReleased = 0.0f;
        touch.lastTotalDelta = touch.totalDelta;
    }
    else if( touching == false || touchAction == touch_pressed )
    {
        touching = true;
        touchingTime = 0.0f;

        // Restart our counters
        touch.totalDelta.x = 0.0f;
        touch.totalDelta.y = 0.0f;
        touch.timeHeld = 0.0f;
        touch.startPosition = CCPoint( x, y );
        for( uint i=0; i<CCScreenTouches::max_last_deltas; ++i )
        {
            touch.lastDeltas[i].clear();
        }
    }
    else
    {
        touch.delta.x += x - touch.position.x;
        touch.delta.y += y - touch.position.y;
        touch.totalDelta.x += touch.delta.x;
        touch.totalDelta.y += touch.delta.y;
    }
    touch.position = CCPoint( x, y );
    touch.usingTouch = screenTouch.usingTouch;

    bool usingControls = false;
    if( hitObject == this && x >= 0.0f && x <= 1.0f && y >= 0.0f && y <= 1.0f )
    {
        for( int i=0; i<linkedScenes.length; ++i )
        {
            if( linkedScenes.list[i]->handleTouches( touch, touches[1], gEngine->time ) )
            {
                usingControls |= true;
                if( touchAction < touch_released )
                {
                    break;
                }
            }
        }
    }

    // Touch lost
    else
    {
        for( int i=0; i<linkedScenes.length; ++i )
        {
            if( linkedScenes.list[i]->handleTouches( touch, touches[1], gEngine->time ) )
            {
            }
        }
    }

    return usingControls;
}


void CCTile3DFrameBuffer::setFrameBufferID(const int frameBufferID)
{
    this->frameBufferID = frameBufferID;
    tileSquare->setFrameBufferID( frameBufferID );
    CCPrimitiveSquareUVs::Setup( &tileSquare->customUVs, 0.0f, 1.0f, 1.0f, 0.0f );
}
