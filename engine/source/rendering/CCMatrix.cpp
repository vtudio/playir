/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCMatrix.cpp
 *-----------------------------------------------------------
 */

#include "CCMatrix.h"

#include "CCMathTools.h"


void CCMatrixLoadIdentity(CCMatrix &result)
{
    memset( &result, 0x0, sizeof( CCMatrix ) );
    result.m[0][0] = 1.0f;
    result.m[1][1] = 1.0f;
    result.m[2][2] = 1.0f;
    result.m[3][3] = 1.0f;
}

void CCMatrixMultiply(CCMatrix &result, const CCMatrix &srcA, const CCMatrix &srcB)
{
    static CCMatrix tmp;
    for( int i=0; i<4; ++i )
    {
        tmp.m[i][0] = (srcA.m[i][0] * srcB.m[0][0]) + (srcA.m[i][1] * srcB.m[1][0]) + (srcA.m[i][2] * srcB.m[2][0]) + (srcA.m[i][3] * srcB.m[3][0]);
        tmp.m[i][1] = (srcA.m[i][0] * srcB.m[0][1]) + (srcA.m[i][1] * srcB.m[1][1]) + (srcA.m[i][2] * srcB.m[2][1]) + (srcA.m[i][3] * srcB.m[3][1]);
        tmp.m[i][2] = (srcA.m[i][0] * srcB.m[0][2]) + (srcA.m[i][1] * srcB.m[1][2]) + (srcA.m[i][2] * srcB.m[2][2]) + (srcA.m[i][3] * srcB.m[3][2]);
        tmp.m[i][3] = (srcA.m[i][0] * srcB.m[0][3]) + (srcA.m[i][1] * srcB.m[1][3]) + (srcA.m[i][2] * srcB.m[2][3]) + (srcA.m[i][3] * srcB.m[3][3]);
    }
    memcpy( &result, &tmp, sizeof( CCMatrix ) );
}


bool CCMatrixInverse(CCMatrix &result, CCMatrix &source)
{
    float *m = source.data();
    float *inv = result.data();

    inv[0]  = m[5]*m[10]*m[15]  - m[5]*m[11]*m[14]  - m[9]*m[6]*m[15]   + m[9]*m[7]*m[14]   + m[13]*m[6]*m[11]  - m[13]*m[7]*m[10];
    inv[4]  = -m[4]*m[10]*m[15] + m[4]*m[11]*m[14]  + m[8]*m[6]*m[15]   - m[8]*m[7]*m[14]   - m[12]*m[6]*m[11]  + m[12]*m[7]*m[10];
    inv[8]  = m[4]*m[9]*m[15]   - m[4]*m[11]*m[13]	- m[8]*m[5]*m[15]   + m[8]*m[7]*m[13]	+ m[12]*m[5]*m[11]	- m[12]*m[7]*m[9];
    inv[12] = -m[4]*m[9]*m[14]  + m[4]*m[10]*m[13]	+ m[8]*m[5]*m[14]   - m[8]*m[6]*m[13]   - m[12]*m[5]*m[10]	+ m[12]*m[6]*m[9];
    inv[1]	= -m[1]*m[10]*m[15] + m[1]*m[11]*m[14]	+ m[9]*m[2]*m[15]   - m[9]*m[3]*m[14]   - m[13]*m[2]*m[11]	+ m[13]*m[3]*m[10];
    inv[5]	= m[0]*m[10]*m[15]	- m[0]*m[11]*m[14]	- m[8]*m[2]*m[15]   + m[8]*m[3]*m[14]   + m[12]*m[2]*m[11]	- m[12]*m[3]*m[10];
    inv[9]	= -m[0]*m[9]*m[15]  + m[0]*m[11]*m[13]	+ m[8]*m[1]*m[15]   - m[8]*m[3]*m[13]	- m[12]*m[1]*m[11]	+ m[12]*m[3]*m[9];
    inv[13] = m[0]*m[9]*m[14]   - m[0]*m[10]*m[13]	- m[8]*m[1]*m[14]   + m[8]*m[2]*m[13]	+ m[12]*m[1]*m[10]	- m[12]*m[2]*m[9];
    inv[2]	= m[1]*m[6]*m[15]   - m[1]*m[7]*m[14]	- m[5]*m[2]*m[15]   + m[5]*m[3]*m[14]	+ m[13]*m[2]*m[7]	- m[13]*m[3]*m[6];
    inv[6]	= -m[0]*m[6]*m[15]  + m[0]*m[7]*m[14]	+ m[4]*m[2]*m[15]   - m[4]*m[3]*m[14]	- m[12]*m[2]*m[7]	+ m[12]*m[3]*m[6];
    inv[10] = m[0]*m[5]*m[15]   - m[0]*m[7]*m[13]	- m[4]*m[1]*m[15]   + m[4]*m[3]*m[13]	+ m[12]*m[1]*m[7]	- m[12]*m[3]*m[5];
    inv[14] = -m[0]*m[5]*m[14]  + m[0]*m[6]*m[13]	+ m[4]*m[1]*m[14]   - m[4]*m[2]*m[13]	- m[12]*m[1]*m[6]	+ m[12]*m[2]*m[5];
    inv[3]	= -m[1]*m[6]*m[11]  + m[1]*m[7]*m[10]	+ m[5]*m[2]*m[11]   - m[5]*m[3]*m[10]	- m[9]*m[2]*m[7]	+ m[9]*m[3]*m[6];
    inv[7]	= m[0]*m[6]*m[11]   - m[0]*m[7]*m[10]	- m[4]*m[2]*m[11]   + m[4]*m[3]*m[10]	+ m[8]*m[2]*m[7]	- m[8]*m[3]*m[6];
    inv[11] = -m[0]*m[5]*m[11]  + m[0]*m[7]*m[9]	+ m[4]*m[1]*m[11]   - m[4]*m[3]*m[9]	- m[8]*m[1]*m[7]	+ m[8]*m[3]*m[5];
    inv[15] = m[0]*m[5]*m[10]   - m[0]*m[6]*m[9]	- m[4]*m[1]*m[10]   + m[4]*m[2]*m[9]	+ m[8]*m[1]*m[6]	- m[8]*m[2]*m[5];

    float det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
    if( det == 0.0f )
    {
        return false;
    }

    det = 1.0f / det;
    for( int i=0; i<16; ++i )
    {
        inv[i] *= det;
    }

    return true;
}


void CCMatrixTranspose(CCMatrix &result, CCMatrix &source)
{
    for( int i=0; i<4; ++i )
    {
        for( int j=0; j<4; ++j )
        {
            result.m[i][j] = source.m[j][i];
        }
    }
}


void CCMatrixFrustum(CCMatrix &result, float left, float right, float bottom, float top, float nearZ, float farZ)
{
    float       deltaX = right - left;
    float       deltaY = top - bottom;
    float       deltaZ = farZ - nearZ;
    CCMatrix    frust;

    if( (nearZ <= 0.0f) || (farZ <= 0.0f) || (deltaX <= 0.0f) || (deltaY <= 0.0f) || (deltaZ <= 0.0f) )
        return;

    frust.m[0][0] = 2.0f * nearZ / deltaX;
    frust.m[0][1] = frust.m[0][2] = frust.m[0][3] = 0.0f;

    frust.m[1][1] = 2.0f * nearZ / deltaY;
    frust.m[1][0] = frust.m[1][2] = frust.m[1][3] = 0.0f;

    frust.m[2][0] = (right + left) / deltaX;
    frust.m[2][1] = (top + bottom) / deltaY;
    frust.m[2][2] = -(nearZ + farZ) / deltaZ;
    frust.m[2][3] = -1.0f;

    frust.m[3][2] = -2.0f * nearZ * farZ / deltaZ;
    frust.m[3][0] = frust.m[3][1] = frust.m[3][3] = 0.0f;

    CCMatrixMultiply( result, frust, result );
}


void CCMatrixPerspective(CCMatrix &result, float fovy, float aspect, float nearZ, float farZ)
{
    float frustumW, frustumH;

    frustumH = tanf( fovy / 360.0f * CC_PI ) * nearZ;
    frustumW = frustumH * aspect;

    CCMatrixFrustum( result, -frustumW, frustumW, -frustumH, frustumH, nearZ, farZ );
}


void CCMatrixOrtho(CCMatrix &result, float left, float right, float bottom, float top, float nearZ, float farZ)
{
    float       deltaX = right - left;
    float       deltaY = top - bottom;
    float       deltaZ = farZ - nearZ;
    CCMatrix    ortho;

    if ( (deltaX == 0.0f) || (deltaY == 0.0f) || (deltaZ == 0.0f) )
        return;

    CCMatrixLoadIdentity( ortho );
    ortho.m[0][0] = 2.0f / deltaX;
    ortho.m[3][0] = -(right + left) / deltaX;
    ortho.m[1][1] = 2.0f / deltaY;
    ortho.m[3][1] = -(top + bottom) / deltaY;
    ortho.m[2][2] = -2.0f / deltaZ;
    ortho.m[3][2] = -(nearZ + farZ) / deltaZ;

    CCMatrixMultiply( result, ortho, result );
}


void CCMatrixScale(CCMatrix &result, float sx, float sy, float sz)
{
    result.m[0][0] *= sx;
    result.m[0][1] *= sx;
    result.m[0][2] *= sx;
    result.m[0][3] *= sx;

    result.m[1][0] *= sy;
    result.m[1][1] *= sy;
    result.m[1][2] *= sy;
    result.m[1][3] *= sy;

    result.m[2][0] *= sz;
    result.m[2][1] *= sz;
    result.m[2][2] *= sz;
    result.m[2][3] *= sz;
}


void CCMatrixTranslate(CCMatrix &result, float tx, float ty, float tz)
{
    result.m[3][0] += ( result.m[0][0] * tx + result.m[1][0] * ty + result.m[2][0] * tz );
    result.m[3][1] += ( result.m[0][1] * tx + result.m[1][1] * ty + result.m[2][1] * tz );
    result.m[3][2] += ( result.m[0][2] * tx + result.m[1][2] * ty + result.m[2][2] * tz );
    result.m[3][3] += ( result.m[0][3] * tx + result.m[1][3] * ty + result.m[2][3] * tz );
}


void CCMatrixPosition(CCMatrix &result, float tx, float ty, float tz)
{
    CCMatrixLoadIdentity( result );
    CCMatrixTranslate( result, tx, ty, tz );
}


void CCMatrixRotateDegrees(CCMatrix &result, float angle, float x, float y, float z)
{
    angle = -angle;
    const float mag = sqrtf( x * x + y * y + z * z );

    const float sinAngle = sinf( angle * CC_PI / 180.0f );
    const float cosAngle = cosf( angle * CC_PI / 180.0f );
    if ( mag > 0.0f )
    {
        float xx, yy, zz, xy, yz, zx, xs, ys, zs;
        CCMatrix rotMat;

        x /= mag;
        y /= mag;
        z /= mag;

        xx = x * x;
        yy = y * y;
        zz = z * z;
        xy = x * y;
        yz = y * z;
        zx = z * x;
        xs = x * sinAngle;
        ys = y * sinAngle;
        zs = z * sinAngle;
        const float oneMinusCos = 1.0f - cosAngle;

        rotMat.m[0][0] = (oneMinusCos * xx) + cosAngle;
        rotMat.m[0][1] = (oneMinusCos * xy) - zs;
        rotMat.m[0][2] = (oneMinusCos * zx) + ys;
        rotMat.m[0][3] = 0.0f;

        rotMat.m[1][0] = (oneMinusCos * xy) + zs;
        rotMat.m[1][1] = (oneMinusCos * yy) + cosAngle;
        rotMat.m[1][2] = (oneMinusCos * yz) - xs;
        rotMat.m[1][3] = 0.0f;

        rotMat.m[2][0] = (oneMinusCos * zx) - ys;
        rotMat.m[2][1] = (oneMinusCos * yz) + xs;
        rotMat.m[2][2] = (oneMinusCos * zz) + cosAngle;
        rotMat.m[2][3] = 0.0f;

        rotMat.m[3][0] = 0.0f;
        rotMat.m[3][1] = 0.0f;
        rotMat.m[3][2] = 0.0f;
        rotMat.m[3][3] = 1.0f;

        CCMatrixMultiply( result, rotMat, result );
    }
}
