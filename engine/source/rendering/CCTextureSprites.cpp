/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCTextureSprites.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCTextureSprites.h"
#include "CCTextureBase.h"
#include "CCPrimitives.h"


CCSpriteInfo* CCTextureSprites::getSpriteInfo(const char *pageName, const CCResourceType resourceType,
                                              const char *spriteName)
{
    CCSpritesPage *page = NULL;
    for( int i=0; i<pages.length; ++i )
    {
        CCSpritesPage *itr = pages.list[i];
        if( itr->name == pageName )
        {
            page = itr;
            break;
        }
    }

    if( page == NULL )
    {
        page = new CCSpritesPage();
        page->name = pageName;
        page->loadData( resourceType );
    }

    return page->getSpriteInfo( spriteName );
}


void CCTextureSprites::setUVs(struct CCPrimitiveSquareUVs **uvs,
                              const char *pageName, const CCResourceType resourceType,
                              const char *spriteName)
{
    CCSpriteInfo *sprite = getSpriteInfo( pageName, resourceType, spriteName );
    sprite->setUVs( uvs );
}



void CCSpritesPage::loadData(const CCResourceType resourceType)
{
//    textureIndex = gEngine->textureManager->assignTextureIndex( name.buffer, resourceType, false, false, false );
//    const CCTextureBase *texture = gEngine->textureManager->getTextureIndex( textureIndex );
//    CCASSERT( texture != NULL );
//    const float textureWidth = texture->getImageWidth();
//    const float textureHeight = texture->getImageHeight();
//
//    CCText xmlFilename = name;
//    xmlFilename.stripExtension();
//    xmlFilename += ".xml";
//    XMLDocument *xml = new XMLDocument();
//    XMLNode *xmlRoot = xml->loadRootNode( xmlFilename.buffer );
//    while( xmlRoot != NULL )
//    {
//        CCSpriteInfo *sprite = new CCSpriteInfo();
//        sprites.add( sprite );
//        sprite->name = xmlRoot->tag();
//
//        XMLNode *internals = xmlRoot->getRoot();
//		while( internals != NULL )
//		{
//			if( internals->tagIs( "frame" ) )
//			{
//				sprite->x1 = internals->attributeFloat( "x1", 0.0f, true );
//				sprite->y1 = internals->attributeFloat( "y1", 0.0f, true );
//				sprite->x2 = internals->attributeFloat( "x2", 0.0f, true );
//				sprite->y2 = internals->attributeFloat( "y2", 0.0f, true );
//
//				const float width = sprite->x2 - sprite->x1;
//				const float height = sprite->y2 - sprite->y1;
//				sprite->width = width * textureWidth;
//				sprite->height = height * textureHeight;
//				sprite->aspectRatio = width / height;
//				break;
//			}
//			internals = internals->next();
//		}
//        xmlRoot = xmlRoot->next();
//    }
//    delete xml;
}


CCSpriteInfo* CCSpritesPage::getSpriteInfo(const char *spriteName)
{
    CCSpriteInfo *sprite = NULL;
    for( int i=0; i<sprites.length; ++i )
    {
        CCSpriteInfo *itr = sprites.list[i];
        if( itr->name == spriteName )
        {
            sprite = itr;
            break;
        }
    }
    CCASSERT( sprite != NULL );
    return sprite;
}


void CCSpritesPage::setUVs(CCPrimitiveSquareUVs **uvs, const char *spriteName)
{
    CCSpriteInfo *sprite = getSpriteInfo( spriteName );
    sprite->setUVs( uvs );
}


void CCSpriteInfo::setUVs(CCPrimitiveSquareUVs **uvs)
{
	CCPrimitiveSquareUVs::Setup( uvs, x1, y1, x2, y2 );
}
