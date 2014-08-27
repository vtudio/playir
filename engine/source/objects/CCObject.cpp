/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCObject.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCObjects.h"
#include "CCSceneBase.h"


CCObject::CCObject(const long jsID)
{
    if( jsID != -1 )
    {
        this->jsID = jsID;
    }

    inScene = NULL;
    deleteMe = 0;

	parent = NULL;

	model = NULL;

    renderPass = render_main;
    octreeRender = false;
    readDepth = true;
    writeDepth = true;
    disableCulling = false;
    frontCulling = false;

    transparent = false;
    transparentParent = false;

    colourInterpolator = NULL;
}


void CCObject::destruct()
{
    if( colourInterpolator != NULL )
    {
        DELETE_POINTER( colourInterpolator );
    }

    if( inScene != NULL )
    {
        removeFromScene();
    }
    else if( parent == NULL )
    {
        CCASSERT( false );
    }
    else
    {
        parent->removeChild( this );
    }

    updaters.deleteObjectsAndList();
    children.deleteObjectsAndList();

    DELETE_OBJECT( model );

    super::destruct();
}


// RenerableBase
void CCObject::dirtyWorldMatrix()
{
    super::dirtyWorldMatrix();

    for( int i=0; i<children.length; ++i )
    {
        children.list[i]->dirtyWorldMatrix();
    }
}


void CCObject::setScene(CCSceneBase *scene)
{
    scene->addObject( this );
}


void CCObject::removeFromScene()
{
	deactivate();
    inScene->removeObject( this );
}


void CCObject::deleteLater()
{
    // Mark to delete in 2 frames
    deleteMe = 2;
    deactivate();
}


void CCObject::addChild(CCObject *object, const int index)
{
    children.add( object );

    if( index >= 0 )
    {
        children.reinsert( object, index );
    }

    object->parent = this;

    if( transparentParent == false )
    {
        if( object->transparentParent )
        {
            setTransparentParent( true );
        }
    }
}


bool CCObject::removeChild(CCObject *object)
{
    if( children.remove( object ) )
    {
        if( children.length == 0 )
        {
            children.freeList();
        }

        return true;
    }

    return false;
}


void CCObject::moveChildToScene(CCObject *object, CCSceneBase *scene)
{
    object->translate( &position );
    object->setScene( scene );
    CCASSERT( removeChild( object ) );
}


void CCObject::addUpdater(CCUpdater *updater)
{
    updaters.add( updater );
}


void CCObject::removeUpdater(CCUpdater *updater)
{
    updaters.remove( updater );
}


bool CCObject::shouldCollide(CCCollideable *collideWith, const bool initialCall)
{
    // It's not me is it?
    if( this == collideWith )
    {
        return false;
    }

    // Ask my parent if we should collide
    if( parent != NULL )
    {
        return parent->shouldCollide( collideWith, initialCall );
    }

    // Yeah, let's collide baby
    return true;
}


bool CCObject::update(const CCTime &time)
{
    bool updated = false;

    for( int i=0; i<updaters.length; ++i )
    {
        updated |= updaters.list[i]->update( time.delta );
    }

    for( int i=0; i<children.length; ++i )
    {
        updated |= children.list[i]->update( time );
    }

    if( colourInterpolator != NULL )
    {
        updated |= colourInterpolator->update( time.delta );
    }

    return updated;
}


void CCObject::renderObject(const CCCameraBase *camera, const bool alpha)
{
    if( renderable )
    {
        if( alpha == false || transparentParent )
        {
            GLPushMatrix();
            {
                refreshModelMatrix();
                GLMultMatrixf( modelMatrix );

                if( alpha == transparent )
                {
                    if( colour != NULL )
                    {
                        CCSetColour( *colour );
                    }
                    renderModel( alpha );
                }

                for( int i=0; i<children.length; ++i )
                {
                    children.list[i]->renderObject( camera, alpha );
                }
            }
            GLPopMatrix();
        }
    }
}


void CCObject::renderModel(const bool alpha)
{
    if( model )
    {
        if( disableCulling )
        {
            CCRenderer::CCSetCulling( false );
        }

        if( frontCulling )
        {
            CCRenderer::CCSetFrontCulling();
        }

        if( alpha )
        {
            if( readDepth )
            {
                CCRenderer::CCSetDepthRead( true );
                if( writeDepth )
                {
                    CCRenderer::CCSetDepthWrite( true );
                    model->render( alpha );
                    CCRenderer::CCSetDepthWrite( false );
                }
                else
                {
                    model->render( alpha );
                }
                CCRenderer::CCSetDepthRead( false );
            }
            else
            {
                model->render( alpha );
            }
        }
        else
        {
            if( readDepth )
            {
                if( writeDepth )
                {
                    model->render( alpha );
                }
                else
                {
                    CCRenderer::CCSetDepthWrite( false );
                    model->render( alpha );
                    CCRenderer::CCSetDepthWrite( true );
                }
            }
            else
            {
                CCRenderer::CCSetDepthRead( false );
                model->render( alpha );
                CCRenderer::CCSetDepthRead( true );
            }
        }

        if( frontCulling )
        {
            CCRenderer::CCSetBackCulling();
        }

        if( disableCulling )
        {
            CCRenderer::CCSetCulling( true );
        }
    }
}


void CCObject::setTransparent(const bool toggle)
{
    transparent = toggle;
    setTransparentParent( toggle );
}


void CCObject::setTransparentParent(const bool toggle)
{
    transparentParent = toggle;
    if( parent != NULL )
    {
        parent->setTransparentParent( toggle );
    }
}


CCInterpolatorLinearColour& CCObject::getColourInterpolator()
{
    if( colourInterpolator == NULL )
    {
        colourInterpolator = new CCInterpolatorLinearColour();
        if( getColour() != NULL )
        {
            colourInterpolator->setup( getColour(), *getColour() );
        }
    }

    return *colourInterpolator;
}


void CCObject::setColour(const CCColour &inColour, const bool interpolate, CCLambdaCallback *inCallback)
{
	if( colour == NULL )
	{
		colour = new CCColour();
	}

    if( interpolate )
    {
        getColourInterpolator().setup( getColour(), inColour );
        if( inCallback )
        {
            getColourInterpolator().setTarget( inColour, inCallback );
        }
    }
    else
    {
        *colour = inColour;
        if( colourInterpolator != NULL )
        {
            colourInterpolator->setup( getColour(), *getColour() );
        }
    }
}


void CCObject::setColourAlpha(const float inAlpha, const bool interpolate, CCLambdaCallback *inCallback)
{
    if( colour == NULL )
    {
        colour = new CCColour();
    }

    if( interpolate )
    {
        getColourInterpolator().setup( getColour(), *getColour() );
        getColourInterpolator().setTargetAlpha( inAlpha, inCallback );
    }
    else
    {
        colour->alpha = inAlpha;
        if( colourInterpolator != NULL )
        {
            colourInterpolator->setup( getColour(), *getColour() );
        }
    }
}


void CCObject::setModel(CCModelBase *model)
{
    this->model = model;
}


void CCObject::setReadDepth(const bool toggle)
{
    readDepth = toggle;
}


void CCObject::setWriteDepth(const bool toggle)
{
    writeDepth = toggle;
}


void CCObject::setCulling(const bool toggle)
{
    disableCulling = !toggle;
}


void CCObject::setFrontCulling(const bool toggle)
{
    frontCulling = toggle;
}
