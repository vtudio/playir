/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCMatrix.h
 * Description : OpenGL rendering matrix functions.
 *               Based on OpenGL ES2.0 programming book
 *               http://code.google.com/p/opengles-book-samples/
 *
 *
 * Created     : 20/09/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCMATRIX_H__
#define __CCMATRIX_H__


struct CCMatrix
{
	inline float* data() { return (float*)&m[0][0]; }
    float m[4][4];
};

extern void CCMatrixLoadIdentity(CCMatrix &result);

extern void CCMatrixMultiply(CCMatrix &result, const CCMatrix &srcA, const CCMatrix &srcB);

extern bool CCMatrixInverse(CCMatrix &result, CCMatrix &source);
extern void CCMatrixTranspose(CCMatrix &result, CCMatrix &source);

extern void CCMatrixFrustum(CCMatrix &result, float left, float right, float bottom, float top, float nearZ, float farZ);
extern void CCMatrixPerspective(CCMatrix &result, float fovy, float aspect, float nearZ, float farZ);
extern void CCMatrixOrtho(CCMatrix &result, float left, float right, float bottom, float top, float nearZ, float farZ);

extern void CCMatrixScale(CCMatrix &result, float sx, float sy, float sz);
extern void CCMatrixTranslate(CCMatrix &result, float tx, float ty, float tz);
extern void CCMatrixPosition(CCMatrix &result, float tx, float ty, float tz);
extern void CCMatrixRotateDegrees(CCMatrix &result, float angle, float x, float y, float z);


#endif // __CCMATRIX_H__
