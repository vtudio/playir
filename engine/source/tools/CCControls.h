/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCControls.h
 * Description : Cross platform controls interface.
 *
 * Created     : 01/03/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCCONTROLS_H__
#define __CCCONTROLS_H__


#define CC_DOUBLE_TAP_THRESHOLD 0.2f


#ifdef IOS
    #ifdef __OBJC__
        #include "CCGLView.h"
    #else
        #define UITouch void
    #endif
#else
    struct UITouch;
#endif


enum CCTouchAction
{
	touch_pressed,
    touch_movingHorizontal,
    touch_movingVertical,
	touch_moving,
	touch_released,
	touch_lost
};


enum CCTwoTouchAction
{
    twotouch_unassigned,
    twotouch_zooming,
    twotouch_rotating
};


struct CCScreenTouches
{
	CCScreenTouches()
	{
		lastTouch = usingTouch = NULL;
		timeHeld = lastTimeReleased = 0.0f;
	}

    const CCPoint averageLastDeltas() const;

	UITouch *usingTouch, *lastTouch;
    CCPoint startPosition, position, delta, totalDelta, lastTotalDelta;
	float timeHeld, lastTimeReleased;

    enum { max_last_deltas = 50 };
    struct TimedDelta
    {
        TimedDelta()
        {
            time = 0.0f;
        }

        void clear()
        {
            time = 0.0f;
            delta = CCPoint();
        }

        float time;
        CCPoint delta;
    };
    TimedDelta lastDeltas[max_last_deltas];
};



struct CCSensorInclinometer
{
    CCSensorInclinometer()
    {
        updated = false;
    }

    bool updated;
    CCVector3 prevIncline;
    CCVector3 currIncline;
    CCVector3 newIncline;

    CCVector3 change;
};



class CCControls
{
public:
	CCControls();

	void render();

	// Synchronizes the controls
	void update(const CCTime &time);

    // Update our touch logic
    static void UpdateTouch(CCScreenTouches &touch, const CCTime &time);

protected:
   	void unTouch(UITouch *touch);

public:
    static bool DetectZoomGesture(const CCScreenTouches &touch1, const CCScreenTouches &touch2);
    static bool DetectRotateGesture(const CCScreenTouches &touch1, const CCScreenTouches &touch2);
    static bool TouchActionMoving(const CCTouchAction touchAction);

    static const CCPoint& GetTouchMovementThreashold() { return TouchMovementThreashold; }
    static void RefreshTouchMovementThreashold();
    static void SetDPI(const float x, const float y);

    const CCScreenTouches* getScreenTouches() { return screenTouches; }
    const CCSensorInclinometer& getInclinometer() { return inclinometer; }

public:
    enum { max_touches = 2 };

protected:
	bool inUse;
    CCScreenTouches screenTouches[max_touches];
    static CCPoint TouchMovementPixels;
    static CCPoint TouchMovementThreashold;

    CCSensorInclinometer inclinometer;
};


#endif // __CONTROLS_H_
