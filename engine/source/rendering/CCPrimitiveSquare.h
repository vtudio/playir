/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCPrimitiveSquare.h
 * Description : Square drawable component.
 *
 * Created     : 01/03/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCPRIMITIVESQUARE_H__
#define __CCPRIMITIVESQUARE_H__


struct CCPrimitiveSquareUVs
{
	static void Setup(CCPrimitiveSquareUVs **uvs, const float x1, const float y1, const float x2, const float y2)
	{
		if( *uvs == NULL )
		{
			*uvs = new CCPrimitiveSquareUVs( x1, y1, x2, y2 );
		}
		else
		{
			(*uvs)->set( x1, y1, x2, y2 );
		}
	}

	CCPrimitiveSquareUVs(const float x1, const float y1, const float x2, const float y2)
	{
		set( x1, y1, x2, y2 );
	}

	void set(const float x1, const float y1, const float x2, const float y2)
	{
        uvs[0] = x2;	// Bottom right
        uvs[1] = y1;
        uvs[2] = x1;	// Bottom left
        uvs[3] = y1;
        uvs[4] = x2;	// Top right
        uvs[5] = y2;
        uvs[6] = x1;	// Top left
        uvs[7] = y2;
	}

	void scroll(const float x, const float y)
	{
		uvs[0] += x;
		uvs[1] += y;
		uvs[2] += x;
		uvs[3] += y;
		uvs[4] += x;
		uvs[5] += y;
		uvs[6] += x;
		uvs[7] += y;

		if( uvs[0] > 1.0f )
		{
			for( uint i=0; i<8; ++i )
			{
				uvs[i] -= 1.0f;
			}
		}
	}

    void flipY()
    {
        const float x1 = uvs[0];
        const float x2 = uvs[2];
        uvs[0] = x2;
		uvs[2] = x1;
		uvs[4] = x2;
		uvs[6] = x1;
    }

	float uvs[8];
};

class CCPrimitiveSquare : public CCPrimitiveBase
{
	typedef CCPrimitiveBase super;

public:
	CCPrimitiveSquareUVs *customUVs;      // Custom UV coordinates
	CCPrimitiveSquareUVs *adjustedUVs;    // Adjusted UV coordinates from our custom UVs based on texture allocation size

    CCVector3 *scale;
    CCVector3 *position;


protected:
    float *customVertexPositionBuffer;

    
public:
	CCPrimitiveSquare(const long primitiveID=-1);

    // CCBaseType
	virtual void destruct();

    // Adjust the model's UVs to match the loaded texture,
    // as non-square textures load into a square texture which means the mapping requires adjustment
    virtual void adjustTextureUVs();

public:
    void setTextureUVs(const float x1, const float y1, const float x2, const float y2);

	virtual void renderVertices(const bool textured);
	virtual void renderOutline();

    void setScale(const float width, const float height, const float depth=1.0f)
    {
        CCVector3FillPtr( &scale, width, height, depth );
    };

    void setPosition(const float x, const float y, const float z)
    {
        CCVector3FillPtr( &position, x, y, z );
    };

    void setCustomVertexPositionBuffer(float *buffer);

    void flipY();
};


#endif // __CCPRIMITIVESQUARE_H__
