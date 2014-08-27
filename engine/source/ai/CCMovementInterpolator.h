/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCMovementInterpolator
 * Description : Handles the movement an object
 *
 * Created     : 09/09/12
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCMOVEMENTINTERPOLATOR_H__
#define __CCMOVEMENTINTERPOLATOR_H__


class CCMovementInterpolator : public CCUpdater
{
public:
    typedef CCUpdater super;

    CCMovementInterpolator(CCCollideable *inObject, const bool updateCollisions);

    bool update(const float delta);
    void clear();

    void setMovement(const CCVector3 target, CCLambdaCallback *inCallback=NULL);
    void setMovementX(const float x);
    void translateMovementX(const float x);
    void setMovementY(const float y, CCLambdaCallback *inCallback=NULL);
    void setMovementXY(const float x, const float y, CCLambdaCallback *inCallback=NULL);
    void setMovementYZ(const float y, const float z, CCLambdaCallback *inCallback=NULL);
    const CCVector3 getMovementTarget() const;

    void setDuration(const float duration);

protected:
    CCCollideable *object;
    bool updating;
    CCInterpolatorListV3<CCInterpolatorX3Curve> movementInterpolator;
    bool updateCollisions;
};


#endif // __CCMOVEMENTINTERPOLATOR_H__

