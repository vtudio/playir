/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCVectors.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"


void CCVector3::set(const char *text)
{
    CCText data = text;
    CCPtrList<char> split;
    data.split( split, "," );
    CCASSERT( split.length == 3 );

    if( split.length == 3 )
    {
        x = (float)atof( split.list[0] );
        y = (float)atof( split.list[1] );
        z = (float)atof( split.list[2] );

        if( x != x )
        {
            CCASSERT( false );
            x = 0.0f;
        }

        if( y != y )
        {
            y = 0.0f;
        }

        if( z != z )
        {
            z = 0.0f;
        }
    }
}


float& CCVector3::operator[](int idx)
{
    switch(idx)
    {
        case 0: return x;
        case 1: return y;
        case 2: return z;
    }
    CCASSERT( true );
    return x;
}


const float& CCVector3::operator[](int idx) const
{
    switch(idx)
    {
        case 0: return x;
        case 1: return y;
        case 2: return z;
    }
    CCASSERT( true );
    return x;
}


bool CCVector3::toTarget(const float target, const float speed)
{
    bool updating = CCToTarget( x, target, speed );
    updating |= CCToTarget( y, target, speed );
    updating |= CCToTarget( z, target, speed );
	return updating;
}


bool CCVector3::toTarget(const CCVector3 &target, const float speed)
{
    bool updating = CCToTarget( x, target.x, speed );
    updating |= CCToTarget( y, target.y, speed );
    updating |= CCToTarget( z, target.z, speed );
	return updating;
}


bool CCVector3::toTarget(const CCVector3 &target, const float speedX, const float speedY, const float speedZ)
{
    bool updating = CCToTarget( x, target.x, speedX );
    updating |= CCToTarget( y, target.y, speedY );
    updating |= CCToTarget( z, target.z, speedZ );
	return updating;
}


void CCVector3Transform(const CCVector3 *translation, const float m[16], CCVector3 *out)
{
#define M(row, col) m[row*4+col]
    out->x = translation->x * M(0,0) + translation->y * M(1,0) + translation->z * M(2,0) +	1.0f * M(3,0);
    out->y = translation->x * M(0,1) + translation->y * M(1,1) + translation->z * M(2,1) +	1.0f * M(3,1);
    out->z = translation->x * M(0,2) + translation->y * M(1,2) + translation->z * M(2,2) +	1.0f * M(3,2);
#undef M
}



void CCVector3::clamp(const CCVector3 &min, const CCVector3 &max)
{
    CCFloatClamp( x, min.x, max.x );
    CCFloatClamp( y, min.y, max.y );
    CCFloatClamp( z, min.z, max.z );
}


void CCVector3::clamp(const float min, const float max)
{
    CCFloatClamp( x, min, max );
    CCFloatClamp( y, min, max );
    CCFloatClamp( z, min, max );
}


void CCVector3::clampDistance(const CCVector3 &target, const float offset)
{
    CCClampDistance( x, target.x, offset );
    CCClampDistance( y, target.y, offset );
    CCClampDistance( z, target.z, offset );
}


bool CCColour::toTarget(const CCColour &target, const float amount)
{
    bool interpolating = CCToTarget( red, target.red, amount );
    interpolating |= CCToTarget( green, target.green, amount );
    interpolating |= CCToTarget( blue, target.blue, amount );
    interpolating |= CCToTarget( alpha, target.alpha, amount );
	return interpolating;
}
