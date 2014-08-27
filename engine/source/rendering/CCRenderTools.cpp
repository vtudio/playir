/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCRenderTools.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"


// Matrix functions
//-----------------
void CCMatrixMulVec(const float matrix[16], const float in[4], float out[4])
{
    int i;
    for( i=0; i<4; ++i )
    {
        out[i] =
        in[0] * matrix[0*4+i] +
        in[1] * matrix[1*4+i] +
        in[2] * matrix[2*4+i] +
        in[3] * matrix[3*4+i];
    }
}


void CCMatrixMulMat(const float a[16], const float b[16], float r[16])
{
    int i, j;
    for( i=0; i<4; ++i )
    {
        for( j=0; j<4; ++j )
        {
             r[i*4+j] = a[i*4+0]*b[0*4+j] +	a[i*4+1]*b[1*4+j] +	a[i*4+2]*b[2*4+j] +	a[i*4+3]*b[3*4+j];
        }
    }
}


bool CCMatrixInvert(const float m[16], float invOut[16])
{
	float inv[16];
	inv[0]	=   m[5]*m[10]*m[15]	- m[5]*m[11]*m[14]	- m[9]*m[6]*m[15]
			  + m[9]*m[7]*m[14]		+ m[13]*m[6]*m[11]	- m[13]*m[7]*m[10];
	inv[4]	= - m[4]*m[10]*m[15]	+ m[4]*m[11]*m[14]	+ m[8]*m[6]*m[15]
			  - m[8]*m[7]*m[14]		- m[12]*m[6]*m[11]	+ m[12]*m[7]*m[10];
	inv[8]	=   m[4]*m[9]*m[15]		- m[4]*m[11]*m[13]	- m[8]*m[5]*m[15]
			  + m[8]*m[7]*m[13]		+ m[12]*m[5]*m[11]	- m[12]*m[7]*m[9];
	inv[12] = - m[4]*m[9]*m[14]		+ m[4]*m[10]*m[13]	+ m[8]*m[5]*m[14]
			  - m[8]*m[6]*m[13]		- m[12]*m[5]*m[10]	+ m[12]*m[6]*m[9];
	inv[1]	= - m[1]*m[10]*m[15]	+ m[1]*m[11]*m[14]	+ m[9]*m[2]*m[15]
			  - m[9]*m[3]*m[14]		- m[13]*m[2]*m[11]	+ m[13]*m[3]*m[10];
	inv[5]	=   m[0]*m[10]*m[15]	- m[0]*m[11]*m[14]	- m[8]*m[2]*m[15]
			  + m[8]*m[3]*m[14]		+ m[12]*m[2]*m[11]	- m[12]*m[3]*m[10];
	inv[9]	= - m[0]*m[9]*m[15]		+ m[0]*m[11]*m[13]	+ m[8]*m[1]*m[15]
			  - m[8]*m[3]*m[13]		- m[12]*m[1]*m[11]	+ m[12]*m[3]*m[9];
	inv[13] =   m[0]*m[9]*m[14]		- m[0]*m[10]*m[13]	- m[8]*m[1]*m[14]
			  + m[8]*m[2]*m[13]		+ m[12]*m[1]*m[10]	- m[12]*m[2]*m[9];
	inv[2]	=   m[1]*m[6]*m[15]		- m[1]*m[7]*m[14]	- m[5]*m[2]*m[15]
			  + m[5]*m[3]*m[14]		+ m[13]*m[2]*m[7]	- m[13]*m[3]*m[6];
	inv[6]	= - m[0]*m[6]*m[15]		+ m[0]*m[7]*m[14]	+ m[4]*m[2]*m[15]
			  - m[4]*m[3]*m[14]		- m[12]*m[2]*m[7]	+ m[12]*m[3]*m[6];
	inv[10] =   m[0]*m[5]*m[15]		- m[0]*m[7]*m[13]	- m[4]*m[1]*m[15]
			  + m[4]*m[3]*m[13]		+ m[12]*m[1]*m[7]	- m[12]*m[3]*m[5];
	inv[14] = - m[0]*m[5]*m[14]		+ m[0]*m[6]*m[13]	+ m[4]*m[1]*m[14]
			  - m[4]*m[2]*m[13]		- m[12]*m[1]*m[6]	+ m[12]*m[2]*m[5];
	inv[3]	= - m[1]*m[6]*m[11]		+ m[1]*m[7]*m[10]	+ m[5]*m[2]*m[11]
			  - m[5]*m[3]*m[10]		- m[9]*m[2]*m[7]	+ m[9]*m[3]*m[6];
	inv[7]	=   m[0]*m[6]*m[11]		- m[0]*m[7]*m[10]	- m[4]*m[2]*m[11]
			  + m[4]*m[3]*m[10]		+ m[8]*m[2]*m[7]	- m[8]*m[3]*m[6];
	inv[11] = - m[0]*m[5]*m[11]		+ m[0]*m[7]*m[9]	+ m[4]*m[1]*m[11]
			  - m[4]*m[3]*m[9]		- m[8]*m[1]*m[7]	+ m[8]*m[3]*m[5];
	inv[15] =   m[0]*m[5]*m[10]		- m[0]*m[6]*m[9]	- m[4]*m[1]*m[10]
			  + m[4]*m[2]*m[9]		+ m[8]*m[1]*m[6]	- m[8]*m[2]*m[5];

	float det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
	if( det == 0.0f )
	{
		return false;
	}

	det = 1.0f / det;

	for( int i=0; i<16; ++i )
	{
		invOut[i] = inv[i] * det;
	}

	return true;
}


// Render functions
//-----------------
void CCRenderSquare(const CCVector3 &start, const CCVector3 &end, const bool outlined)
{
	if( outlined )
	{
		const float vertices[] =
		{
			start.x,    end.y,      end.z,		// Bottom left
			end.x,      end.y,      end.z,		// Bottom right
			start.x,    start.y,    start.z,	// Top left
			end.x,      start.y,    start.z,	// Top right
			start.x,    end.y,      end.z,		// Bottom left
		};

		// draw the square
		GLVertexPointer( 2, GL_FLOAT, 0, vertices, 5 );
		gRenderer->GLDrawArrays( GL_LINE_STRIP, 0, 5 );
	}
	else
	{
		const float vertices[] =
		{
			start.x,    end.y,      end.z,		// Bottom left
			end.x,      end.y,      end.z,		// Bottom right
			start.x,    start.y,    start.z,	// Top left
			end.x,      start.y,    start.z,	// Top right
		};

		// draw the square
		GLVertexPointer( 3, GL_FLOAT, 0, vertices, 4 );
		gRenderer->GLDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
	}
}


void CCRenderSquareYAxisAligned(const CCVector3 &start, const CCVector3 &end)
{
    CCRenderer::CCSetRenderStates( true );

    const float vertices[] =
	{
		start.x, start.y, start.z,	// Top left
		end.x, start.y, end.z,		// Top right
		start.x, end.y, start.z,	// Bottom left
		end.x, end.y, end.z,		// Bottom right
    };

	// draw the square
	GLVertexPointer( 3, GL_FLOAT, 0, vertices, 4 );
	gRenderer->GLDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
}


void CCRenderSquarePoint(const CCPoint &position, const float &size)
{
	CCRenderRectanglePoint( position, size, size );
}


void CCRenderRectanglePoint(const CCPoint &position, const float &sizeX, const float &sizeY, const bool outlined)
{
    static CCVector3 start, end;
	start.x = position.x - sizeX;
	start.y = position.y - sizeY;
	end.x = position.x + sizeX;
	end.y = position.y + sizeY;

	CCRenderSquare( start, end, outlined );
}


void CCRenderLine(const CCVector3 &start, const CCVector3 &end)
{
    CCRenderer::CCSetRenderStates( true );

	const float vertices[] =
	{
		start.x, start.y, start.z,
		end.x, end.y, end.z,
	};

	GLVertexPointer( 3, GL_FLOAT, 0, vertices, 2 );
	gRenderer->GLDrawArrays( GL_LINES, 0, 2 );
}


void CCRenderCube(const bool outline)
{
    CCRenderer::CCSetRenderStates( true );

	// Define the square vertices
	static const float vertices[] =
	{
	// Front
        -0.5, -0.5, -0.5,   // Bottom left  0
        0.5, -0.5, -0.5,    // Bottom right 1
        -0.5, 0.5, -0.5,    // Top left     2
        0.5, 0.5, -0.5,     // Top right    3

        // Back
        -0.5, -0.5, 0.5,    // Bottom left  4
        0.5, -0.5, 0.5,     // Bottom right 5
        -0.5, 0.5, 0.5,     // Top left     6
        0.5, 0.5, 0.5       // Top right    7
	};

	GLVertexPointer( 3, GL_FLOAT, 0, vertices, 8 );

	if( outline )
	{
		static const ushort faces[] =
		{
			0, 1, 3, 2, 0, 4, 5, 1, 5, 7, 3, 7, 6, 2, 6, 4,
		};
		static const uint numberOfFaces = sizeof( faces ) / sizeof( ushort );
		gRenderer->GLDrawElements( GL_LINE_STRIP, numberOfFaces, GL_UNSIGNED_SHORT, faces );
	}
	else
	{
		static const ushort faces[] =
		{
			0, 1, 2, 3, 7, 1, 5, 4, 7, 6, 2, 4, 0, 1,
		};
		static const uint numberOfFaces = sizeof( faces ) / sizeof( ushort );
		gRenderer->GLDrawElements( GL_TRIANGLE_STRIP, numberOfFaces, GL_UNSIGNED_SHORT, faces );
	}
}


void CCRenderCubeMinMax(const CCVector3 min, const CCVector3 max, const bool outline)
{
	// Define the square vertices
	const float vertices[] =
	{
		// Front
		min.x, min.y, min.z,		// Bottom left	0
		max.x, min.y, min.z,		// Bottom right	1
		min.x, max.y, min.z,		// Top Left		2
		max.x, max.y, min.z,		// Top Right	3

		// Back
		min.x, min.y, max.z,		// Bottom left	4
		max.x, min.y, max.z,		// Bottom right	5
		min.x, max.y, max.z,		// Top left		6
		max.x, max.y, max.z,		// Top right	7
	};

    CCRenderer::CCSetRenderStates( true );

	GLVertexPointer( 3, GL_FLOAT, 0, vertices, 8 );

	if( outline )
	{
		static const ushort faces[] =
		{
			0, 1, 3, 2, 0, 4, 5, 1, 5, 7, 3, 7, 6, 2, 6, 4,
		};
		static const uint numberOfFaces = sizeof( faces ) / sizeof( ushort );
		gRenderer->GLDrawElements( GL_LINE_STRIP, numberOfFaces, GL_UNSIGNED_SHORT, faces );
	}
	else
	{
		static const ushort faces[] =
		{
			0, 1, 2, 3, 7, 1, 5, 4, 7, 6, 2, 4, 0, 1,
		};
		static const uint numberOfFaces = sizeof( faces ) / sizeof( ushort );
		gRenderer->GLDrawElements( GL_TRIANGLE_STRIP, numberOfFaces, GL_UNSIGNED_SHORT, faces );
	}
}


// Shader functions
//-----------------
static CCColour currentColour = CCColour();

void CCSetColour(const CCColour &colour)
{
    if( currentColour.equals( colour ) == false )
    {
        currentColour = colour;
        GLColor4f( currentColour.red, currentColour.green, currentColour.blue, currentColour.alpha );
	}
}

void CCSetColour(const float r, const float g, const float b, const float a)
{
    if( currentColour.red != r || currentColour.green != g || currentColour.blue != b || currentColour.alpha != a )
    {
        currentColour.set( r, g, b, a );
        GLColor4f( currentColour.red, currentColour.green, currentColour.blue, currentColour.alpha );
	}
}


const CCColour& CCGetColour()
{
    return currentColour;
}


static const float *currentUVs = NULL;
void CCSetTexCoords(const float *inUVs)
{
	if( currentUVs != inUVs )
	{
		currentUVs = inUVs;
        if( currentUVs != NULL )
        {
            GLTexCoordPointer( 2, GL_FLOAT, 0, currentUVs );
        }
	}
}

void CCDefaultTexCoords()
{
	static const float texCoords[] =
	{
		1.0f, 0.0f,
		0.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};
	CCSetTexCoords( texCoords );
}


void CCInverseTexCoords()
{
	static const float texCoords[] =
	{
		0.0f, -1.0f,
		1.0f, -1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f
	};
	CCSetTexCoords( texCoords );
}


void CCRefreshRenderAttributes()
{
    GLColor4f( currentColour.red, currentColour.green, currentColour.blue, currentColour.alpha );

#ifndef DXRENDERER
    GLTexCoordPointer( 2, GL_FLOAT, 0, currentUVs );

    CCSetUniformVector3( UNIFORM_LIGHTPOSITION, 0.0f, 0.0f, 1.0f );
    CCSetUniformVector4( UNIFORM_LIGHTDIFFUSE, 1.0f, 1.0f, 1.0f, 1.0f );
#endif
}
