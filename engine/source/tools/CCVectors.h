/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCVectors.h
 * Description : Contains point respresentative structures.
 *
 * Created     : 01/03/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCVECTORS_H__
#define __CCVECTORS_H__


#include "CCPlatform.h"


struct CCSize
{
    CCSize()
	{
		width = height = 0.0f;
	}

    CCSize(const float inWidth, const float inHeight)
	{
		width = inWidth;
		height = inHeight;
	}

	float width, height;
};


struct CCPoint
{
    CCPoint()
	{
		x = y = 0.0f;
	}

    CCPoint(const float inX, const float inY)
	{
		x = inX;
		y = inY;
	}

	float x, y;
};


struct CCMinMax
{
    CCMinMax()
    {
        reset();
    }

    void reset()
    {
        min = MAXFLOAT;
        max = -MAXFLOAT;
    }

    void consider(const float value)
    {
        min = MIN( min, value );
        max = MAX( max, value );
    }

    float size() const
    {
        return max - min;
    }

    float min;
    float max;
};


struct CCVector3
{
	float	x;
	float	y;
	float	z;

	CCVector3()
	{
		set( 0.0f );
	}

    CCVector3(const float value)
	{
		set( value );
	}

    CCVector3(const CCPoint &other)
	{
		set( other.x, other.y, 0.0f );
	}

    CCVector3(const CCVector3 &other)
	{
		set( other.x, other.y, other.z );
	}

	CCVector3(const float inX, const float inY, const float inZ)
	{
		set( inX, inY, inZ );
	}

    float& operator[](int idx);
    const float& operator[](int idx) const;

    bool operator!=(const CCVector3& other) const
	{
		return x != other.x || y != other.y || z != other.z;
	}

	inline void set(const float value)
	{
		x = y = z = value;
	}

	inline void set(const CCVector3 &vector)
	{
		x = vector.x;
		y = vector.y;
		z = vector.z;
	}

	inline void set(const float inX, const float inY, const float inZ)
	{
		x = inX;
		y = inY;
		z = inZ;
	}

    void set(const char *text);

	inline void zero()
	{
		set( 0.0f );
	}

    inline bool isZero()
    {
        if( x == 0.0f && y == 0.0f && z == 0.0f )
        {
            return true;
        }
        return false;
    }

	inline void mul(const float multiple)
	{
		x *= multiple;
		y *= multiple;
		z *= multiple;
	}

	inline void mul(const CCVector3 &vector)
	{
		x *= vector.x;
		y *= vector.y;
		z *= vector.z;
	}

    inline void div(const CCVector3 &vector)
	{
		x /= vector.x;
		y /= vector.y;
		z /= vector.z;
	}

	inline void add(const float amount)
	{
		x += amount;
		y += amount;
		z += amount;
	}

	inline void add(const CCVector3 &vector)
	{
		x += vector.x;
		y += vector.y;
		z += vector.z;
	}

	inline void sub(const float amount)
	{
		x -= amount;
		y -= amount;
		z -= amount;
	}

	inline void sub(const CCVector3 &vector)
	{
		x -= vector.x;
		y -= vector.y;
		z -= vector.z;
	}

	inline void unitize()
	{
		const float absX = fabsf( x );
		const float absY = fabsf( y );
		const float absZ = fabsf( z );

		float dividedBy = 1.0f;
		if( absX > absY )
		{
			if( absX > absZ )
			{
				dividedBy = 1.0f / absX;
			}
			else
			{
				dividedBy = 1.0f / absZ;
			}
		}
		else if( absY > absZ )
		{
			dividedBy = 1.0f / absY;
		}
		else
		{
			dividedBy = 1.0f / absZ;
		}

		x *= dividedBy;
		y *= dividedBy;
		z *= dividedBy;
	}

    bool toTarget(const float target, const float speed);
    bool toTarget(const CCVector3 &target, const float speed);
    bool toTarget(const CCVector3 &target, const float speedX, const float speedY, const float speedZ);

	void clamp(const CCVector3 &min, const CCVector3 &max);
	void clamp(const float min, const float max);
	void clampDistance(const CCVector3 &target, const float offset);
};

static CCVector3 vectorResult;

static inline void CCVector3FillPtr(CCVector3 **vector, const float x, const float y, const float z)
{
	if( *vector == NULL )
	{
		*vector = (CCVector3*)malloc( sizeof( CCVector3 ) );
	}

	(*vector)->set( x, y, z );
}

static inline void CCVector3FillPtr(CCVector3 **vector, const float value)
{
	if( *vector == NULL )
	{
		*vector = (CCVector3*)malloc( sizeof( CCVector3 ) );
	}

	(*vector)->set( value );
}

static inline bool CCVector3EqualsFloat(const CCVector3 *a, const float &b)
{
	return a->x == b && a->y == b && a->z == b;
}

static inline bool CCVector3Equals(const CCVector3 &a, const CCVector3 &b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

static inline CCVector3 CCVector3MulResult(const CCVector3 &vector, const float &multiple)
{
	vectorResult = vector;
	vectorResult.mul( multiple );
	return vectorResult;
}

static inline CCVector3 CCVector3SubResult(const CCVector3 &vector, const CCVector3 &subtract)
{
	vectorResult = vector;
	vectorResult.sub( subtract );
	return vectorResult;
}

static inline float CCVector3LengthSquared(const CCVector3 &vector)
{
	return ( vector.x * vector.x ) + ( vector.y * vector.y ) + ( vector.z * vector.z );
}

static inline float CCVector3Magnitude(const CCVector3 &vector, const bool squared=true)
{
	if( squared )
	{
		return CCVector3LengthSquared( vector );
	}
	return sqrtf( CCVector3LengthSquared( vector ) );
}

static inline float CCVector3Distance(const CCVector3 &from, const CCVector3 &to, const bool squared=true)
{
    CCVector3 &difference = vectorResult;
	difference = to;
	difference.sub( from );
	difference.mul( 0.5f );

	return CCVector3Magnitude( difference, squared );
}

static inline float CCVector3Distance2D(const CCVector3 &from, const CCVector3 &to, const bool squared=true)
{
    static CCPoint difference;
	difference.x = from.x - to.x;
	difference.y = from.z - to.z;

	const float result = difference.x * difference.x + difference.y * difference.y;
	return squared ? result : sqrtf( result );
}

static inline void CCVector3Normalize(CCVector3 &vector)
{
	const float vectorMagnitude = CCVector3Magnitude( vector );
	if( vectorMagnitude == 0.0f )
	{
		vector.x = 1.0f;
		vector.y = 0.0f;
		vector.z = 0.0f;
        return;
	}

	const float oneOverMagnitude = 1.0f / vectorMagnitude;
	vector.x *= oneOverMagnitude;
	vector.y *= oneOverMagnitude;
	vector.z *= oneOverMagnitude;
}

static inline CCVector3 CCVector3Direction(const CCVector3 &start, const CCVector3 &end)
{
	vectorResult.x = end.x - start.x;
	vectorResult.y = end.y - start.y;
	vectorResult.z = end.z - start.z;
	CCVector3Normalize( vectorResult );
	return vectorResult;
}

static inline CCVector3 CalculateSurfaceNormal(const CCVector3 &v1, const CCVector3 &v2, const CCVector3 &v3)
{
	const CCVector3 u = CCVector3Direction( v2, v1 );
	const CCVector3 v = CCVector3Direction( v3, v1 );

	vectorResult.x = ( u.y * v.z ) - ( u.z * v.y );
	vectorResult.y = ( u.z * v.x ) - ( u.x * v.z );
	vectorResult.z = ( u.x * v.y ) - ( u.y * v.x );
	return vectorResult;
}

extern void CCVector3Transform(const CCVector3 *translation, const float m[16], CCVector3 *out);


struct CCColour
{
	float	red;
	float	green;
	float	blue;
	float	alpha;

	CCColour()
	{
		red = green = blue = alpha = 1.0f;
	}

	CCColour(const float grey)
	{
		set( grey );
		alpha = 1.0f;
	}

    CCColour(const float grey, const float a)
	{
		set( grey );
		alpha = a;
	}

	CCColour(const float r, const float g, const float b, const float a=1.0f)
	{
		set( r, g, b, a );
    }

	CCColour(const int r, const int g, const int b, const int a=255)
	{
		red = r/255.0f;
		green = g/255.0f;
		blue = b/255.0f;
		alpha = a/255.0f;
    }

	inline void set(const float grey)
	{
		red = green = blue = grey;
	}

	inline void set(const float r, const float g, const float b, const float a)
	{
		red = r;
		green = g;
		blue = b;
		alpha = a;
	}

    inline bool equals(const CCColour &other) const
	{
		return red == other.red &&
		green == other.green &&
		blue == other.blue &&
		alpha == other.alpha;
	}

    bool operator!=(const CCColour &other) const
	{
		return red != other.red || green != other.green || blue != other.blue || alpha != other.alpha;
	}

    bool toTarget(const CCColour &target, const float amount);
};

static inline CCColour CCColourInt(const float grey)
{
	static const float multiple = 1.0f / 255.0f;
	return CCColour( grey * multiple );
}

static inline CCColour CCColourInt(const float red, const float green, const float blue, const float alpha)
{
	static const float multiple = 1.0f / 255.0f;
	return CCColour( red * multiple, green * multiple, blue * multiple, alpha );
}


template <typename T> struct CCTarget
{
    T current;
    T target;
};


struct CCFloatTarget
{
	CCFloatTarget()
	{
		current = target = 0.0f;
	}

    inline bool notEqual()
	{
		return current != target;
	}
	float current, target;
};


struct CCVector3Target
{
    inline bool notEqual() const
	{
		return current != target;
	}

    inline bool isTargetZero() const
	{
		return ( target.x == 0.0f && target.y == 0.0f && target.z == 0.0f );
	}

    inline float distance() const
	{
		return CCVector3Distance2D( current, target, false );
	}
	CCVector3 current, target;
};


#endif // __CCVECTORS_H__
