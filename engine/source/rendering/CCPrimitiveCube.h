/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCPrimitiveCube.h
 * Description : Cube drawable component.
 *
 * Created     : 01/03/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCPRIMITIVECUBE_H__
#define __CCPRIMITIVECUBE_H__


class CCPrimitiveCube : public CCPrimitiveBase
{
public:
	typedef CCPrimitiveBase super;

	CCPrimitiveCube();

	// CCPrimitiveBase
	virtual void renderVertices(const bool textured);
	virtual void renderOutline();

	// Setup square cube
	void setupSquare(const float size);

	// Setup rectangle
	void setupRectangle(const float width, const float height);
	void setupRectangle(const float depth, const float y, const float height, const float width);

	// Setup use the same Z for top and bottom for trapezoid
	void setupTrapezoid(const float depth, const float y, const float height, const float bL, const float bR, const float tL, const float tR);

	// Setup trapezoid
	void setupTrapezoidZ(const float depth, const float bZ, const float tZ, const float y, const float height, const float bL, const float bR, const float tL, const float tR);
	void setupTrapezoidDepths(const float bottomDepth, const float topDepth, const float y, const float height, const float bL, const float bR, const float tL, const float tR);

	void setup(const float bottomDepth, const float topDepth,
               const float bZ, const float tZ,
               const float y, const float height,
               const float leftBottom, const float rightBottom,
               const float leftTop, const float rightTop);
};


#endif // __CCPRIMITIVECUBE_H__