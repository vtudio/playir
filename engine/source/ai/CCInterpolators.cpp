/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCInterpolators.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"


void CCInterpolator::setDuration(const float duration)
{
    speed = 1.0f/duration;
}



// CCInterpolatorSin2Curve
void CCInterpolatorSin2Curve::setup(float *inCurrent, const float inTarget)
{
    if( current != inCurrent || target != inTarget )
    {
        current = inCurrent;
        
        target = inTarget;
        ready();
    }
    else if( *current == target )
    {
        amount = 1.0f;
    }
    
    updating = true;
}


void CCInterpolatorSin2Curve::ready()
{
    if( current != NULL )
    {
        start = *current;
        length = target - start;
        if( *current == target )
        {
            amount = 1.0f;
        }
        else
        {
            amount = 0.0f;
        }

        updating = true;
    }
}


bool CCInterpolatorSin2Curve::incrementAmount(const float delta)
{
    if( CCToTarget( amount, 1.0f, delta * speed ) )
    {
        return true;
    }
    return false;
}


float CCInterpolatorSin2Curve::calculatePercentage()
{
    static const float interpolationCurveMultiple = 1.0f / sinf( 2.0f );
    const float percentage = sinf( amount * 2.0f ) * interpolationCurveMultiple;
    return percentage;
}


void CCInterpolatorSin2Curve::updateInterpolation(const float percentage)
{
    const float movement = length * percentage;
    *current = movement + start;
}


bool CCInterpolatorSin2Curve::update(const float delta)
{
    updating = false;
    
    if( current != NULL )
    {
        if( incrementAmount( delta ) )
        {
            const float percentage = calculatePercentage();
            updateInterpolation( percentage );
            updating = true;
            return true;
        }
        else if( *current != target )
        {
            *current = target;
            current = NULL;
            updating = true;
            return true;
        }
        else
        {
            current = NULL;
        }
    }
    else if( onInterpolated.length > 0 )
    {
        CCLAMBDA_SIGNAL pendingCallbacks;
        for( int i=0; i<onInterpolated.length; ++i )
        {
            pendingCallbacks.add( onInterpolated.list[i] );
        }
        onInterpolated.length = 0;
        CCLAMBDA_EMIT_ONCE( pendingCallbacks );
    }

    return updating;
}


bool CCInterpolatorSin2Curve::equals(float *inCurrent, const float inTarget)
{
    // Ignore if we're already doing this
    if( current == inCurrent && target == inTarget )
    {
        return true;
    }
    return false;
}



// CCInterpolatorX2Curve
float CCInterpolatorX2Curve::calculatePercentage()
{
    const float percentage = amount * amount;
    return percentage;
}



float CCInterpolatorX3Curve::calculatePercentage()
{
    const float sudoAmount = amount - 1.0f;
    const float percentage = 1.0f + ( sudoAmount * sudoAmount * sudoAmount );
    return percentage;
}



// CCInterpolatorSinCurve
float CCInterpolatorSinCurve::calculatePercentage()
{
    static const float interpolationCurveMultiple = 1.0f / sinf( CC_HPI );
    const float percentage = sinf( amount * CC_HPI ) * interpolationCurveMultiple;
    return percentage;
}



// CCInterpolatorLinear
float CCInterpolatorLinear::calculatePercentage()
{
    return amount;
}



// CCInterpolatorListV3
template class CCInterpolatorListV3<CCInterpolatorSin2Curve>;
template class CCInterpolatorListV3<CCInterpolatorX2Curve>;
template class CCInterpolatorListV3<CCInterpolatorX3Curve>;
template class CCInterpolatorListV3<CCInterpolatorLinear>;


template <typename T>
bool CCInterpolatorListV3<T>::update(const float delta)
{
    updating = false;
    
    if( interpolators.length > 0 )
    {
        CCInterpolatorV3<T> *interpolator = interpolators.list[0];
        if( interpolator->update( delta * speed ) == false )
        {
            interpolators.remove( interpolator );
            DELETE_OBJECT( interpolator );

            // If there's another interpolation planned, tell it to ready itself to interpolate
            if( interpolators.length > 0 )
            {
                interpolators.list[0]->ready();
            }
            else
            {
                return false;
            }
        }
        updating = true;
    }
    return updating;
}



// CCInterpolatorLinearColour
bool CCInterpolatorLinearColour::update(const float delta)
{
    if( current != NULL )
    {
        if( updating )
        {
            if( current->toTarget( target, delta * speed ) )
            {
                return true;
            }
            else
            {
                updating = false;
            }
        }
        else
        {
            CCASSERT( current->equals( target ) );
            if( onInterpolated.length > 0 )
            {
                CCLAMBDA_SIGNAL pendingCallbacks;
                for( int i=0; i<onInterpolated.length; ++i )
                {
                    pendingCallbacks.add( onInterpolated.list[i] );
                }
                onInterpolated.length = 0;
                CCLAMBDA_EMIT_ONCE( pendingCallbacks );
            }
        }
    }
    return false;
}



// CCTimer
bool CCTimer::update(const float delta)
{
    if( updating )
    {
        const float real = gEngine->time.real;
        time -= real;
        if( time <= 0.0f )
        {
            finish();
        }
        return true;
    }
    return false;
}


void CCTimer::finish()
{
    updating = false;
    CCLAMBDA_EMIT( onTime );
}


void CCTimer::start(const float timeout)
{
    interval = timeout;
    restart();
}


void CCTimer::stop()
{
    updating = false;
}


void CCTimer::restart()
{
    time = interval;
    updating = true;
}



// CCTimerMS
void CCTimerMS::start(const float timeout)
{
    interval = timeout * 0.001f; // Milliseconds to seconds
    restart();
}
