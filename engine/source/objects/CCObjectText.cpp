/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCObjectText.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCObjects.h"
#include "CCTextureManager.h"
#include "CCTextureFontPage.h"


CCObjectText::CCObjectText(CCCollideable *inParent)
{
    colour = new CCColour();
    setTransparent( true );
    readDepth = false;

    shader = "alphacolour";
    centered = true;
    endMarker = false;

    setFont( "HelveticaNeueLight" );

    parent = inParent;
    if( inParent )
    {
        inParent->addChild( this );
    }
}


CCObjectText::CCObjectText(const long jsID)
{
    if( jsID != -1 )
    {
        this->jsID = jsID;
    }

    colour = NULL;      // Colour handled in JS
    setTransparent( true );
    readDepth = false;

    shader = NULL;      // No shaders yet
    centered = true;
    endMarker = false;

    parent = NULL;
}


void CCObjectText::destruct()
{
    super::destruct();
}


void CCObjectText::renderObject(const CCCameraBase *camera, const bool alpha)
{
#if defined PROFILEON
    CCProfiler profile( "CCObjectText::renderObject()" );
#endif

    if( renderable )
    {
        GLPushMatrix();
        {
            refreshModelMatrix();
            GLMultMatrixf( modelMatrix );

            if( shader != NULL )
            {
                gRenderer->setShader( shader );
            }

            if( colour != NULL )
            {
                CCSetColour( *colour );
            }

            if( alpha == false || CCGetColour().alpha > 0.0f )
            {
                fontPage->renderText( text.buffer, text.length, height, centered );

                if( endMarker )
                {
                    gEngine->textureManager->setTextureIndex( 1 );
                    const float x = ( fontPage->getWidth( text.buffer, text.length, height ) + fontPage->getCharacterWidth( ' ', height ) ) * 0.5f;
                    const float y = height * 0.45f;
                    const CCVector3 start = CCVector3( x, -y, 0.0f );
                    const CCVector3 end = CCVector3( x, y, 0.0f );
                    CCRenderLine( start, end );
                }
            }
        }
        GLPopMatrix();
    }
}


void CCObjectText::setText(const char *text, const float height, const char *font)
{
    this->text = text;

    if( height != -1.0f )
    {
        setHeight( height );
    }

    if( font != NULL )
    {
        setFont( font );
    }
}


float CCObjectText::getWidth()
{
    return fontPage->getWidth( text.buffer, text.length, height );
}


float CCObjectText::getHeight()
{
    return fontPage->getHeight( text.buffer, text.length, height );
}


void CCObjectText::setHeight(const float height)
{
    this->height = height;
}


void CCObjectText::setCentered(const bool centered)
{
    if( centered )
    {
        setPositionX( 0.0f );
    }
    else if( parent != NULL )
    {
        setPositionX( -parent->collisionBounds.x );
    }

	this->centered = centered;
}


void CCObjectText::setFont(const char *font)
{
    for( int i=0; i<gEngine->textureManager->fontPages.length; ++i )
    {
        CCTextureFontPage *page = gEngine->textureManager->fontPages.list[i];
        const char *name = page->getName();
        if( CCText::Equals( font, name ) )
        {
            fontPage = page;
            return;
        }
    }
    
    //CCASSERT( false );
    fontPage = gEngine->textureManager->fontPages.list[0];
}
