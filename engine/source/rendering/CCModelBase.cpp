/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCModelBase.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCPrimitives.h"


CCModelBase::CCModelBase(const long jsID)
{
    if( jsID != -1 )
    {
        this->jsID = jsID;
    }
    
    shader = "basic";
}


void CCModelBase::destruct()
{
	models.deleteObjectsAndList();
	primitives.deleteObjectsAndList();

	super::destruct();
}


void CCModelBase::render(const bool alpha)
{
#if defined PROFILEON
    CCProfiler profile( "CCModelBase::render()" );
#endif

    if( renderable )
    {
        GLPushMatrix();
        {
            refreshModelMatrix();
            GLMultMatrixf( modelMatrix );

            gRenderer->setShader( shader );

            if( colour != NULL )
            {
                CCSetColour( *colour );
            }

            if( alpha == false || CCGetColour().alpha > 0.0f )
            {
                for( int i=0; i<primitives.length; ++i )
                {
                    CCPrimitiveBase *primitive = primitives.list[i];
                    primitive->render();
                }
            }

            for( int i=0; i<models.length; ++i )
            {
                CCModelBase *model = models.list[i];
                model->render( alpha );
                if( colour != NULL )
                {
                    CCSetColour( *colour );
                }
            }
        }
        GLPopMatrix();
    }
}


void CCModelBase::addModel(CCModelBase *model, const int index)
{
    CCASSERT( model != this );
    models.add( model );

    if( index != -1 )
    {
        models.reinsert( model, index );
    }
}


void CCModelBase::removeModel(CCModelBase *model)
{
    CCASSERT( model != this );
    models.remove( model );
}


void CCModelBase::addPrimitive(CCPrimitiveBase *primitive)
{
	primitives.add( primitive );
}
