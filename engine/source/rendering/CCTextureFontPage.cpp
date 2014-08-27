/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCTextureFontPage.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCTextureFontPage.h"
#include "CCTextureManager.h"


#define MAX_TEXT_LENGTH 512
#define MAX_TEXT_LINES 8


CCTextureFontPage::CCTextureFontPage()
{
    loaded = false;
}


float CCTextureFontPage::getCharacterWidth(const char character, const float size) const
{
    if( character == '\n' )
    {
    }
    else
    {
        const Letter *letter = getLetter( character );
        if( letter != NULL )
        {
            return letter->size.width * size;
        }
    }

	return 0.0f;
}

float CCTextureFontPage::getWidth(const char *text, const uint length, const float size) const
{
	float totalWidth = 0.0f;
	for( uint i=0; i<length; ++i )
	{
        const char character = text[i];
        if( character == '\n' )
        {
            break;
        }
        else
        {
            const Letter *letter = getLetter( text[i] );
            if( letter != NULL )
            {
                totalWidth += letter->size.width * size;
            }
        }
	}

	return totalWidth;
}


float CCTextureFontPage::getHeight(const char *text, const uint length, const float size) const
{
	float lineHeights[MAX_TEXT_LINES] = { 0.0f };
    int lines = 1;
    {
        int index = 0;
        for( uint i=0; i<length; ++i )
        {
            const char character = text[i];
            if( character == '\n' )
            {
                index++;
                lines++;
            }
            else
            {
                const Letter *letter = getLetter( text[i] );
                if( letter != NULL )
                {
                    lineHeights[index] = MAX( lineHeights[index], letter->size.height * size );
                }
            }
        }
    }

    float height = 0.0f;
    for( int i=0; i<lines; ++i )
    {
        height += lineHeights[i];
    }
	return height;
}


void CCTextureFontPage::renderText(const char *text, const uint length,
                                   const float height, const bool centeredX)
{
#if defined PROFILEON
    CCProfiler profile( "CCTextureFontPage::renderText()" );
#endif
    
    if( !loaded )
    {
        return;
    }

	if( length == 0 )
	{
		return;
	}

    CCASSERT( text != NULL );
    CCASSERT( length < MAX_TEXT_LENGTH );

	const CachedTextMesh *mesh = getTextMesh( text, length, height, centeredX );
	if( mesh != NULL && mesh->vertexCount > 0 )
	{
		// Draw our text mesh
		GLPushMatrix();
		{
			GLTranslatef( 0.0f, mesh->totalLineHeight*0.5f, 0.0f );
			CCRenderer::CCSetRenderStates( true );
			bindTexturePage();
			CCSetTexCoords( mesh->uvs );
			GLVertexPointer( 3, GL_FLOAT, 0, mesh->vertices, mesh->vertexCount );
			gRenderer->GLDrawArrays( GL_TRIANGLES, 0, mesh->vertexCount );
		}
		GLPopMatrix();
	}
}


const CCTextureFontPage::CachedTextMesh* CCTextureFontPage::getTextMesh(const char *text, const uint length, const float height, const bool centeredX)
{
#if defined PROFILEON
    CCProfiler profile( "CCTextureFontPage::getTextMesh()" );
#endif

	for( int i=0; i<cachedMeshes.length; ++i )
	{
		CachedTextMesh *mesh =  cachedMeshes.list[i];
		if( mesh->textHeight == height )
		{
			if( mesh->centeredX == centeredX )
			{
				if( mesh->text.length == length )
				{
					if( CCText::Equals( mesh->text, text ) )
					{
						mesh->lastDrawTime = gEngine->time.lifetime;
						return mesh;
					}
				}
			}
		}
	}

	if( cachedMeshes.length > 50 )
	{
		// Delete the oldest one
		float oldestRenderTime = MAXFLOAT;
		CachedTextMesh *oldestRender = NULL;
		for( int i=0; i<cachedMeshes.length; ++i )
		{
			CachedTextMesh *mesh = cachedMeshes.list[i];
			if( mesh->lastDrawTime < oldestRenderTime )
			{
				oldestRenderTime = mesh->lastDrawTime;
				oldestRender = mesh;
			}
		}

		if( oldestRender != NULL )
		{
			cachedMeshes.remove( oldestRender );
			delete oldestRender;
		}
	}

	CachedTextMesh *mesh = buildTextMesh( text, length, height, centeredX );
	cachedMeshes.add( mesh );
	mesh->lastDrawTime = gEngine->time.lifetime;
	return mesh;
}


CCTextureFontPage::CachedTextMesh* CCTextureFontPage::buildTextMesh(const char *text, const uint length, const float height, const bool centeredX)
{
#if defined PROFILEON
    CCProfiler profile( "CCTextureFontPage::buildTextMesh()" );
#endif

    // Find out our width so we can center the text
    float totalLineHeight = 0.0f;
    CCPoint lineSize[MAX_TEXT_LINES];
    static CCPoint charSize[MAX_TEXT_LINES][MAX_TEXT_LENGTH];

    int lineIndex = 0;
    int characterIndex = 0;
    for( uint i=0; i<length; ++i )
    {
        char character = text[i];
        {
            const Letter *letter = getLetter( character );
            if( letter != NULL )
            {
                CCPoint &size = charSize[lineIndex][characterIndex];
                size.x = letter->size.width * height;
                size.y = letter->size.height * height;

                lineSize[lineIndex].x += size.x;
                lineSize[lineIndex].y = MAX( lineSize[lineIndex].y, size.y );
                characterIndex++;
            }
        }
        if( character == '\n' )
        {
            totalLineHeight += lineSize[lineIndex].y;
            lineIndex++;
            characterIndex = 0;
            CCASSERT( lineIndex < MAX_TEXT_LINES );
        }
    }
    totalLineHeight += lineSize[lineIndex].y;

    CCPtrList<CCPoint> startPositions;
    for( int i=0; i<lineIndex+1; ++i )
    {
        CCPoint *start = new CCPoint();
        startPositions.add( start );
        if( centeredX )
        {
            start->x -= lineSize[i].x * 0.5f;
        }

        for( int j=0; j<i; ++j )
        {
            start->y -= lineSize[j].y;
        }
    }

    static CCPoint currentStart, currentEnd;
    currentStart.x = startPositions.list[0]->x;
    currentStart.y = startPositions.list[0]->y;


	// We will dynamically create meshes from the lines to save draw calls
	float *vertices = (float*)malloc( sizeof( float ) * 3 * 6 * length );
	float *uvs = (float*)malloc( sizeof( float ) * 2 * 6 * length );
	int vertexIndex = 0;
	int texCoordIndex = 0;

    lineIndex = 0;
    characterIndex = 0;
    for( uint i=0; i<length; ++i )
    {
        char character = text[i];
        {
            const Letter *letter = getLetter( character );
            if( letter != NULL )
            {
                CCPoint &size = charSize[lineIndex][characterIndex];

                // Calculate end point
                currentEnd.x = currentStart.x + size.x;
                currentEnd.y = currentStart.y - size.y;

				// Triangle 1
				{
					vertices[vertexIndex++] = currentStart.x;			// Bottom left
					vertices[vertexIndex++] = currentEnd.y;
					vertices[vertexIndex++] = 0.0f;
					uvs[texCoordIndex++] = letter->start.x;
					uvs[texCoordIndex++] = letter->end.y;

					vertices[vertexIndex++] = currentEnd.x;				// Bottom right
					vertices[vertexIndex++] = currentEnd.y;
					vertices[vertexIndex++] = 0.0f;
					uvs[texCoordIndex++] = letter->end.x;
					uvs[texCoordIndex++] = letter->end.y;

					vertices[vertexIndex++] = currentStart.x;			// Top left
					vertices[vertexIndex++] = currentStart.y;
					vertices[vertexIndex++] = 0.0f;
					uvs[texCoordIndex++] = letter->start.x;
					uvs[texCoordIndex++] = letter->start.y;
				}

				// Triangle 2
				{
					vertices[vertexIndex++] = currentEnd.x;				// Bottom right
					vertices[vertexIndex++] = currentEnd.y;
					vertices[vertexIndex++] = 0.0f;
					uvs[texCoordIndex++] = letter->end.x;
					uvs[texCoordIndex++] = letter->end.y;

					vertices[vertexIndex++] = currentEnd.x;				// Top right
					vertices[vertexIndex++] = currentStart.y;
					vertices[vertexIndex++] = 0.0f;
					uvs[texCoordIndex++] = letter->end.x;
					uvs[texCoordIndex++] = letter->start.y;

					vertices[vertexIndex++] = currentStart.x;			// Top left
					vertices[vertexIndex++] = currentStart.y;
					vertices[vertexIndex++] = 0.0f;
					uvs[texCoordIndex++] = letter->start.x;
					uvs[texCoordIndex++] = letter->start.y;
				}

                currentStart.x += size.x;
                characterIndex++;
            }
        }
        if( character == '\n' )
        {
            lineIndex++;
            CCPoint &start = *startPositions.list[lineIndex];
            currentStart.x = start.x;
            currentStart.y = start.y;
            characterIndex = 0;
        }
    }

    startPositions.deleteObjects();

	CachedTextMesh *mesh = new CachedTextMesh();
	mesh->text = text;
	mesh->textHeight = height;
	mesh->centeredX = centeredX;
	mesh->totalLineHeight = totalLineHeight;
	mesh->vertices = vertices;
	mesh->uvs = uvs;
	mesh->vertexCount = vertexIndex/3;

	return mesh;
}


void CCTextureFontPage::renderOutline(CCVector3 start, CCVector3 end, const float multiple) const
{
    const float width = end.x - start.x;
    const float height = end.y - start.y;

    const float outlineWidth = width * multiple;
    const float outlineHeight = height * multiple;

    start.x += width * 0.5f;
    start.y += height * 0.5f;
    end = start;
    start.x -= outlineWidth * 0.5f;
    start.y -= outlineHeight * 0.5f;
    end.x += outlineWidth * 0.5f;
    end.y += outlineHeight * 0.5f;
    CCRenderSquare( start, end );
}


void CCTextureFontPage::view() const
{
	bindTexturePage();

    const CCVector3 start = CCVector3( 0.0f, 0.4f, 0.0f );
    const CCVector3 end = CCVector3( start.x, start.y, 0.0f );

	CCRenderSquare( start, end );

	gEngine->textureManager->setTextureIndex( 1 );
}


const CCTextureFontPage::Letter* CCTextureFontPage::getLetter(const char character) const
{
    if( (int)character > 127 )
    {
        if( (int)character == 194 )
        {
            return &letters[0];
        }
    }
	else if( character >= 0 )
    {
        return &letters[(uint)character];
	}
	else if( character == -62 )
    {
        return &letters[0];
	}

	return NULL;
}
