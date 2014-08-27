/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCControls.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCControls.h"
#include "CCDeviceControls.h"
#include "CCAppManager.h"


const CCPoint CCScreenTouches::averageLastDeltas() const
{
    CCPoint averageDeltas;
    int numberOfDeltas = 0;
    const float lifetime = gEngine->time.lifetime;
    for( int i=0; i<max_last_deltas; ++i )
    {
        const TimedDelta &lastDelta = lastDeltas[i];
        const float difference = lifetime - lastDelta.time;
        if( difference < 0.25f )
        {
            averageDeltas.x += lastDelta.delta.x;
            averageDeltas.y += lastDelta.delta.y;
            numberOfDeltas++;
        }
        else
        {
            break;
        }
    }
    if( numberOfDeltas > 1 )
    {
        averageDeltas.x /= numberOfDeltas;
        averageDeltas.y /= numberOfDeltas;
    }
    return averageDeltas;
}



CCPoint CCControls::TouchMovementPixels( 3.0f, 3.0f );
CCPoint CCControls::TouchMovementThreashold;


CCControls::CCControls()
{
    inUse = false;

    RefreshTouchMovementThreashold();
}


void CCControls::render()
{
}


void CCControls::update(const CCTime &time)
{
    for( uint i=0; i<max_touches; ++i )
	{
        UpdateTouch( screenTouches[i], time );
	}

    if( inclinometer.updated )
    {
        inclinometer.updated = false;

        inclinometer.prevIncline = inclinometer.currIncline;
        inclinometer.currIncline = inclinometer.newIncline;

        static bool firstUpdate = true;
        if( firstUpdate )
        {
            firstUpdate = false;
        }
        else
        {
            inclinometer.change.x = CCDirectionBetweenAngles( inclinometer.currIncline.x, inclinometer.prevIncline.x );
            inclinometer.change.y = CCDirectionBetweenAngles( inclinometer.currIncline.y, inclinometer.prevIncline.y );
            inclinometer.change.z = CCDirectionBetweenAngles( inclinometer.currIncline.z, inclinometer.prevIncline.z );
        }

        // Orientation checker
#ifdef QT
//        if( inclinometer.currIncline.z >= 45.0f && inclinometer.currIncline.z < 135.0f  )
//        {
//            CCAppManager::SetIfNewOrientation( 90.0f );
//        }
//        else if( inclinometer.currIncline.z < -45.0f && inclinometer.currIncline.z > -135.0f )
//        {
//            CCAppManager::SetIfNewOrientation( 270.0f );
//        }
//        else
//        {
//            CCAppManager::SetIfNewOrientation( 0.0f );
//        }
#endif
    }
}


void CCControls::UpdateTouch(CCScreenTouches &touch, const CCTime &time)
{
    if( touch.usingTouch != NULL )
    {
        touch.timeHeld += time.real;

        // Only update last deltas if touch is being held
        for( uint i=CCScreenTouches::max_last_deltas-1; i>0; --i )
        {
            touch.lastDeltas[i] = touch.lastDeltas[i-1];
        }
        touch.lastDeltas[0].time = time.lifetime;
        touch.lastDeltas[0].delta = touch.delta;
    }

    if( touch.lastTouch != NULL )
    {
        if( touch.usingTouch != NULL )
        {
            touch.usingTouch = NULL;
        }
        else
        {
            touch.lastTouch = NULL;
        }
    }
    else
    {
        touch.lastTimeReleased += time.real;
    }

    touch.delta.x = 0.0f;
    touch.delta.y = 0.0f;
}


void CCControls::unTouch(UITouch *touch)
{
    UITouch *uiTouch = touch;
    for( uint i=0; i<max_touches; ++i )
	{
        CCScreenTouches &screenTouch = screenTouches[i];
		if( screenTouch.usingTouch == uiTouch )
        {
            CCNativeThreadLock();
            screenTouch.lastTouch = uiTouch;

			screenTouch.lastTimeReleased = 0.0f;
			screenTouch.lastTotalDelta = screenTouch.totalDelta;
            CCNativeThreadUnlock();

            DEBUGLOG( "CCControls::touchEnd() %i\n", i );
            if( gEngine != NULL )
            {
#ifdef IOS
                CCLAMBDA_2( ReRunCallback, CCAppEngine, gEngine, int, i, {
#endif
                    gEngine->touchEnd( i );
#ifdef IOS
                });
                gEngine->nativeToEngineThread( new ReRunCallback( gEngine, i ) );
#endif
            }
		}
	}
}


bool CCControls::DetectZoomGesture(const CCScreenTouches &touch1, const CCScreenTouches &touch2)
{
    if( touch1.delta.x != 0.0f || touch1.delta.y != 0.0f ||
        touch2.delta.x != 0.0f || touch2.delta.y != 0.0f )
    {
        const CCScreenTouches *topTouch = &touch1,
        *bottomTouch = &touch2;
        if( touch1.position.y < touch2.position.y )
        {
            topTouch = &touch2;
            bottomTouch = &touch1;
        }
        const CCScreenTouches *rightTouch = &touch1,
        *leftTouch = &touch2;
        if( touch1.position.x < touch2.position.x )
        {
            rightTouch = &touch2;
            leftTouch = &touch1;
        }

        if( leftTouch->totalDelta.x < 0.0f && rightTouch->totalDelta.x > 0.0f )
        {
            const float total = -leftTouch->totalDelta.x + rightTouch->totalDelta.x;
            if( total > TouchMovementThreashold.x * 2.0f )
            {
                return true;
            }
        }
        else if( leftTouch->totalDelta.x > 0.0f && rightTouch->totalDelta.x < 0.0f )
        {
            const float total = leftTouch->totalDelta.x + -rightTouch->totalDelta.x;
            if( total > TouchMovementThreashold.x * 2.0f )
            {
                return true;
            }
        }

        if( bottomTouch->totalDelta.y < 0.0f && topTouch->totalDelta.y > 0.0f )
        {
            const float total = -bottomTouch->totalDelta.x + topTouch->totalDelta.x;
            if( total > TouchMovementThreashold.y * 2.0f )
            {
                return true;
            }
        }
        else if( bottomTouch->totalDelta.y > 0.0f && topTouch->totalDelta.y < 0.0f )
        {
            const float total = bottomTouch->totalDelta.y + -topTouch->totalDelta.y;
            if( total > TouchMovementThreashold.y * 2.0f )
            {
                return true;
            }
        }
    }

    return false;
}


bool CCControls::DetectRotateGesture(const CCScreenTouches &touch1, const CCScreenTouches &touch2)
{
    if( touch1.delta.x != 0.0f || touch1.delta.y != 0.0f ||
        touch2.delta.x != 0.0f || touch2.delta.y != 0.0f )
    {
        const float absX1 = fabsf( touch1.totalDelta.x );
        const float absX2 = fabsf( touch2.totalDelta.x );
        const float absY1 = fabsf( touch1.totalDelta.y );
        const float absY2 = fabsf( touch2.totalDelta.y );
        if( absX1 > absY1 && absX2 > absY2 )
        {
            if( touch1.totalDelta.x > TouchMovementThreashold.x && touch2.totalDelta.x > TouchMovementThreashold.x )
            {
                return true;
            }

            if( touch1.totalDelta.x < -TouchMovementThreashold.x && touch2.totalDelta.x < -TouchMovementThreashold.x )
            {
                return true;
            }
        }
        else if( absY1 > absX1 && absY2 > absX2 )
        {
            if( touch1.totalDelta.y > TouchMovementThreashold.y && touch2.totalDelta.y > TouchMovementThreashold.y )
            {
                return true;
            }

            if( touch1.totalDelta.y < -TouchMovementThreashold.y && touch2.totalDelta.y < -TouchMovementThreashold.y )
            {
                return true;
            }
        }
    }

    return false;
}


bool CCControls::TouchActionMoving(const CCTouchAction touchAction)
{
    if( touchAction >= touch_movingHorizontal && touchAction <= touch_moving )
    {
        return true;
    }
    return false;
}


void CCControls::RefreshTouchMovementThreashold()
{
	const CCSize &inverseScreenSize = gRenderer->getInverseScreenSize();
    if( CCAppManager::IsPortrait() )
    {
        TouchMovementThreashold.x = TouchMovementPixels.x * inverseScreenSize.width;
        TouchMovementThreashold.y = TouchMovementPixels.y * inverseScreenSize.height;
    }
    else
    {
        TouchMovementThreashold.x = TouchMovementPixels.y * inverseScreenSize.height;
        TouchMovementThreashold.y = TouchMovementPixels.x * inverseScreenSize.width;
    }
}


void CCControls::SetDPI(const float x, const float y)
{
	TouchMovementPixels.x = x * 0.1f;
	TouchMovementPixels.y = y * 0.1f;
}
