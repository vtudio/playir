/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCTextureFontPage.h
 * Description : Handles rendering text.
 *
 * Created     : 01/03/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCTEXTUREFONTPAGE_H__
#define __CCTEXTUREFONTPAGE_H__


#include "CCTextureBase.h"

class CCTextureFontPage
{
protected:
    bool loaded;
    CCText name;

    typedef struct
    {
        CCPoint start, end;
        CCSize size;
    } Letter;
    enum { num_letters = 256 };
    Letter letters[num_letters];

    class CachedTextMesh
	{
	public:
		CachedTextMesh()
		{
			textHeight = 0.0f;
			centeredX = false;
			totalLineHeight = 0.0f;
			vertices = NULL;
			uvs = NULL;
			vertexCount = 0;
			lastDrawTime = 0.0f;
		}

		~CachedTextMesh()
		{
			if( vertices != NULL )
			{
				gRenderer->derefVertexPointer( ATTRIB_VERTEX, vertices );
				free( vertices );
			}

			if( uvs != NULL )
			{
				free( uvs );
			}
		}

		// input
		CCText text;
		float textHeight;
		bool centeredX;

		// calculated
		float totalLineHeight;

		// render
		float *vertices;
		float *uvs;
		uint vertexCount;
        
		// life management
		float lastDrawTime;
	};
	CCPtrList<CachedTextMesh> cachedMeshes;


    
public:
    CCTextureFontPage();
    virtual ~CCTextureFontPage() {}

    inline const char* getName() const { return name.buffer; }
    float getCharacterWidth(const char character, const float size) const;
    float getWidth(const char *text, const uint length, const float size) const;
    float getHeight(const char *text, const uint length, const float size) const;

    void renderText(const char *text, const uint length, const float height=1.0f, const bool centeredX=true);

protected:
	const CachedTextMesh* getTextMesh(const char *text, const uint length, const float height, const bool centeredX);
	CachedTextMesh* buildTextMesh(const char *text, const uint length, const float height, const bool centeredX);

public:
    void renderOutline(CCVector3 start, CCVector3 end, const float multiple) const;
    void view() const;
    const Letter* getLetter(const char character) const;

protected:
    virtual void bindTexturePage() const = 0;
};


#endif // __TEXTUREFONTPAGE_H__
