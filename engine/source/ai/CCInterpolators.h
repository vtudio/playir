/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCInterpolators.h
 * Description : Interpolators for various curves.
 *
 * Created     : 30/04/11
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCINTERPOLATORS_H__
#define __CCINTERPOLATORS_H__


class CCInterpolator : public CCUpdater
{
public:
    CCLAMBDA_SIGNAL onInterpolated;
    
protected:
    float speed;
    bool updating;
    
    
    
public:
    CCInterpolator()
    {
        speed = 1.0f;
        
        updating = false;
    }

    virtual ~CCInterpolator()
    {
#ifdef DEBUGON
        destructCalled = true;
#endif

        onInterpolated.deleteObjectsAndList();
    }

    void setDuration(const float duration);
    
    bool isUpdating()
    {
        return updating;
    }
};



// CCInterpolatorSin2Curve
class CCInterpolatorSin2Curve : public CCInterpolator
{
public:
    CCInterpolatorSin2Curve()
    {
        current = NULL;
        amount = 1.0f;
    }

    CCInterpolatorSin2Curve(float *inCurrent, const float inTarget)
    {
        setup( inCurrent, inTarget );
    }

    void setup(float *inCurrent, const float inTarget);

    // Call when ready to start the interpolation
    void ready();

    bool incrementAmount(const float delta);
    void setAmount(const float amount)
    {
        this->amount = amount;
    }
    virtual float calculatePercentage();
    void updateInterpolation(const float percentage);
    virtual bool update(const float delta);
    void finish()
    {
        amount = 1.0f;
        current = NULL;
    }
    
    bool equals(float *inCurrent, const float inTarget);
    
    inline const float* getCurrent() const { return current; }
    inline float getAmount() const { return amount; }
    inline float getTarget() const { return target; }

protected:
    float *current;
    float target;
    float start;
    float length;
    float amount;
};



// CCInterpolatorX2Curve
class CCInterpolatorX2Curve : public CCInterpolatorSin2Curve
{
public:
    float calculatePercentage();
};


class CCInterpolatorX3Curve : public CCInterpolatorSin2Curve
{
public:
    float calculatePercentage();
};


class CCInterpolatorSinCurve : public CCInterpolatorSin2Curve
{
public:
    float calculatePercentage();
};


class CCInterpolatorLinear : public CCInterpolatorSin2Curve
{
public:
    float calculatePercentage();
};



// CCInterpolatorV3
template <typename T>
class CCInterpolatorV3 : public CCInterpolator
{
protected:
    T x,y,z;
    
    
    
public:
    CCInterpolatorV3() {}

    CCInterpolatorV3(CCVector3 *inCurrent, const CCVector3 target, CCLambdaCallback *inCallback=NULL)
    {
        setup( inCurrent, target, inCallback );
    }

    void setup(CCVector3 *inCurrent, const CCVector3 target, CCLambdaCallback *inCallback=NULL)
    {
        CCASSERT( inCurrent != NULL );

        x.setup( &inCurrent->x, target.x );
        y.setup( &inCurrent->y, target.y );
        z.setup( &inCurrent->z, target.z );

        onInterpolated.deleteObjects();
        if( inCallback != NULL )
        {
            onInterpolated.add( inCallback );
        }
    }

    void ready()
    {
        x.ready();
        y.ready();
        z.ready();
    }

    bool update(const float delta)
    {
        float deltaSpeed = delta * speed;
        updating = x.update( deltaSpeed );
        updating |= y.update( deltaSpeed );
        updating |= z.update( deltaSpeed );

        if( updating == false )
        {
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

        return updating;
    }
    
    bool equals(CCVector3 *inCurrent, const CCVector3 target)
    {
        if( x.equals( &inCurrent->x, target.x ) &&
           y.equals( &inCurrent->y, target.y ) &&
           z.equals( &inCurrent->z, target.z ) )
        {
            return true;
        }
        return false;
    }

    const CCVector3 getAmount() const { return CCVector3( x.getAmount(), y.getAmount(), z.getAmount() ); }
    const CCVector3 getTarget() const { return CCVector3( x.getTarget(), y.getTarget(), z.getTarget() ); }
};



// CCInterpolatorListV3
template <typename T>
class CCInterpolatorListV3 : public CCInterpolator
{
public:
    CCPtrList< CCInterpolatorV3< T > > interpolators;
    
    
    
public:
    virtual ~CCInterpolatorListV3()
    {
        clear();
    }

    void clear()
    {
        interpolators.deleteObjectsAndList();
    }


    // replace: Deletes all the other pending interpolations and pushes this target to the front
    bool pushV3(CCVector3 *inCurrent, const CCVector3 target, const bool replace=false, CCLambdaCallback *inCallback=NULL)
    {
        updating = true;
        
        if( interpolators.length > 0 )
        {
            if( replace )
            {
                bool found = false;
                for( int i=0; i<interpolators.length; ++i )
                {
                    CCInterpolatorV3<T> *interpolator = interpolators.list[i];
                    if( interpolator->equals( inCurrent, target ) )
                    {
                        found = true;
                        if( i != 0 )
                        {
                            interpolator->ready();
                        }
                    }
                    else
                    {
                        interpolator->finish();
                    }
                }

                if( found )
                {
                    return false;
                }
            }
            else
            {
                for( int i=0; i<interpolators.length; ++i )
                {
                    CCInterpolatorV3<T> *interpolator = interpolators.list[i];
                    if( interpolator->equals( inCurrent, target ) )
                    {
                        if( inCallback != NULL )
                        {
                            inCallback->safeRun();
                            delete inCallback;
                        }
                        return false;
                    }
                }
            }
        }

        if( *inCurrent != target )
        {
            interpolators.add( new CCInterpolatorV3<T>( inCurrent, target, inCallback ) );
        }
        else if( inCallback != NULL )
        {
            inCallback->safeRun();
            delete inCallback;
        }
        return true;
    }

    bool update(const float delta);

    bool finished() { return interpolators.length == 0; }

    const CCVector3 getTarget() const
    {
        if( interpolators.length > 0 )
        {
            return interpolators.last()->getTarget();
        }
        return CCVector3();
    }
};



// CCInterpolatorLinearColour
class CCInterpolatorLinearColour : public CCInterpolator
{
protected:
    bool updating;
    CCColour *current;
    CCColour target;
    
    
    
public:
    CCInterpolatorLinearColour()
    {
        current = NULL;
        updating = false;
    }

    CCInterpolatorLinearColour(CCColour *inCurrent, const CCColour inTarget)
    {
        setup( inCurrent, inTarget );
    }

    bool equals(CCColour *inCurrent, const CCColour inTarget)
    {
        // Ignore if we're already doing this
        if( current != inCurrent || target != inTarget )
        {
            return false;
        }
        return true;
    }

    void setup(CCColour *inCurrent, const CCColour inTarget)
    {
        current = inCurrent;
        target = inTarget;
        updating = true;

        if( onInterpolated.length > 0 )
        {
            onInterpolated.deleteObjects();
        }
    }

    bool update(const float delta);

    void setTarget(const CCColour inTarget, CCLambdaCallback *inCallback=NULL)
    {
        setup( current, inTarget );
        if( inCallback != NULL )
        {
            onInterpolated.add( inCallback );
        }
    }

    void setTarget(const CCColour *inTarget, CCLambdaCallback *inCallback=NULL)
    {
        setTarget( *inTarget, inCallback );
    }

    void setTargetColour(const float grey)
    {
        target.red = grey;
        target.green = grey;
        target.blue = grey;
        updating = true;
    }

    void setTargetAlpha(const float inTargetAlpha, CCLambdaCallback *inCallback=NULL)
    {
        if( target.alpha != inTargetAlpha )
        {
            target.alpha = inTargetAlpha;
            setTarget( target, inCallback );
        }
        else
        {
            if( inCallback != NULL )
            {
                onInterpolated.add( inCallback );
            }
        }
    }

    inline const CCColour& getTarget() const { return target; }
};



class CCTimer : public CCUpdater
{
public:
    CCTimer()
    {
        time = 0.0f;
        updating = false;
        interval = 0.0f;
    }

    ~CCTimer()
    {
        onTime.deleteObjects();
    }

    bool update(const float delta);
    virtual void finish();
    void start(const float timeout);
    void stop();
    void restart();

    bool updating;
    float time;
    float interval;
    CCLAMBDA_SIGNAL onTime;
};


class CCTimerMS : public CCTimer
{
public:
    virtual void start(const float timeout);
};


#endif // __CCInterpolators_H__
