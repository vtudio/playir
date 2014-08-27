/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCFrameBufferManager.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCFrameBufferManager.h"


CCFrameBufferObject::CCFrameBufferObject()
{
    name = "default";
}


CCFrameBufferObject::CCFrameBufferObject(const char *inName, const int inWidth, const int inHeight)
{
    name = inName;
    width = inWidth;
    height = inHeight;
}


void CCFrameBufferObject::bindRenderTexture()
{
    gEngine->textureManager->bindTexture( this );
}


GLuint CCFrameBufferObject::getFrameBufferHandle()
{
#ifdef QT

    return frameBuffer->handle();

#else

    return frameBuffer;

#endif
}



CCFrameBufferManager::CCFrameBufferManager()
{
    currentFBOIndex = -1;
}


CCFrameBufferManager::~CCFrameBufferManager()
{
    fbos.deleteObjectsAndList();
}


void CCFrameBufferManager::setup()
{
    gRenderer->createDefaultFrameBuffer( defaultFBO );
}



float CCFrameBufferManager::getWidth(const int fboIndex)
{
    if( fboIndex == -1 )
    {
        return (float)defaultFBO.width;
    }
    else
    {
        return (float)fbos.list[fboIndex]->width;
    }
}


float CCFrameBufferManager::getHeight(const int fboIndex)
{
    if( fboIndex == -1 )
    {
        return (float)defaultFBO.height;
    }
    else
    {
        return (float)fbos.list[fboIndex]->height;
    }
}


int CCFrameBufferManager::findFrameBuffer(const char *name)
{
    for( int i=0; i<fbos.length; ++i )
    {
        CCASSERT( fbos.list[i] != NULL );
        const CCText &currentName = fbos.list[i]->getName();
        if( currentName == name )
        {
            return i;
        }
    }
    return -1;
}


int CCFrameBufferManager::newFrameBuffer(const char *name, const int size, const bool depthBuffer, const bool stencilBuffer)
{
    int newIndex = fbos.length;
    CCFrameBufferObject *newFrameBuffer = new CCFrameBufferObject( name, size, size );
    fbos.add( newFrameBuffer );

    createFrameBuffer( *newFrameBuffer, depthBuffer, stencilBuffer );

    return newIndex;
}


void CCFrameBufferManager::deleteFrameBuffer(const int fboIndex)
{
    if( fboIndex == currentFBOIndex )
    {
        bindDefaultFrameBuffer();
    }

    CCASSERT( fboIndex < fbos.length );
    CCFrameBufferObject *frameBuffer = fbos.list[fboIndex];
    destroyFrameBuffer( *frameBuffer );
    delete frameBuffer;
    fbos.remove( frameBuffer );
}


void CCFrameBufferManager::createFrameBuffer(CCFrameBufferObject &fbo, const bool useDepthBuffer, const bool useStencilBuffer)
{
#if defined( IOS ) || defined( ANDROID )

    GLuint frameBuffer = 0;
    GLuint depthBuffer = 0;
    GLuint stencilBuffer = 0;
    GLuint renderTexture = 0;

    // Create default framebuffer object
	glGenFramebuffers( 1, &frameBuffer );
	glBindFramebuffer( GL_FRAMEBUFFER, frameBuffer );

    glGenTextures( 1, &renderTexture );
	glBindTexture( GL_TEXTURE_2D, renderTexture );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, fbo.width, fbo.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0 );

	if ( useDepthBuffer )
	{
        glGenRenderbuffers( 1, &depthBuffer );
        glBindRenderbuffer( GL_RENDERBUFFER, depthBuffer );
        glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, fbo.width, fbo.height );
        glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer );
    }

    if ( useStencilBuffer )
    {
        glGenRenderbuffers( 1, &stencilBuffer );
        glBindRenderbuffer( GL_RENDERBUFFER, stencilBuffer );
        glRenderbufferStorage( GL_RENDERBUFFER, GL_STENCIL_INDEX8, fbo.width, fbo.height );
        glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencilBuffer );
    }

	if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
	{
		DEBUGLOG( "Failed to make complete framebuffer object %x", glCheckFramebufferStatus( GL_FRAMEBUFFER ) );
        CCASSERT( false );
	}

    fbo.setFrameBuffer( frameBuffer );
    fbo.setRenderTexture( renderTexture );
    fbo.depthBuffer = depthBuffer;
    fbo.stencilBuffer = stencilBuffer;

    glBindTexture( GL_TEXTURE_2D, 0 );
    bindDefaultFrameBuffer();

#elif defined( QT )

    QGLFramebufferObject::Attachment attachment = QGLFramebufferObject::NoAttachment;
    if( useDepthBuffer )
    {
        if( useStencilBuffer )
        {
            attachment = QGLFramebufferObject::CombinedDepthStencil;
        }
        else
        {
            attachment = QGLFramebufferObject::Depth;
        }
    }

    QGLFramebufferObject *newFrameBuffer = new QGLFramebufferObject( fbo.width, fbo.height, attachment );
    CCASSERT_MESSAGE( newFrameBuffer->isValid(), "Failed to create frame buffer\n" );

    fbo.setFrameBuffer( newFrameBuffer );
    fbo.setRenderTexture( newFrameBuffer->texture() );

#else

    CCASSERT( false );

#endif
}



void CCFrameBufferManager::destroyFrameBuffer(CCFrameBufferObject &fbo)
{
#if defined( IOS ) || defined( ANDROID )

    GLuint frameBuffer = fbo.getFrameBuffer();
    if( frameBuffer )
    {
        glDeleteFramebuffers( 1, &frameBuffer );
        fbo.setFrameBuffer( 0 );
    }

    if( fbo.renderBuffer )
    {
        glDeleteRenderbuffers( 1, &fbo.renderBuffer );
        fbo.renderBuffer = 0;
    }

    if( fbo.depthBuffer )
    {
        glDeleteRenderbuffers( 1, &fbo.depthBuffer );
        fbo.depthBuffer = 0;
    }

    if( fbo.stencilBuffer )
    {
        glDeleteRenderbuffers( 1, &fbo.stencilBuffer );
        fbo.stencilBuffer = 0;
    }

    GLuint texture = fbo.getRenderTexture();
    if( texture )
    {
        glDeleteTextures( 1, &texture );
    }

#elif defined( QT )

    QGLFramebufferObject *fboObject = fbo.getFrameBuffer();
    DELETE_POINTER( fboObject );

#else

    CCASSERT( false );

#endif
}


void CCFrameBufferManager::destoryAllFrameBuffers()
{
    for( int i=0; i<fbos.length; ++i )
    {
        destroyFrameBuffer( *fbos.list[i] );
    }
    destroyFrameBuffer( defaultFBO );
}


void CCFrameBufferManager::bindFrameBuffer(const int fboIndex)
{
    if( currentFBOIndex != fboIndex )
    {
        CCFrameBufferObject &fbo = *fbos.list[fboIndex];

#if defined( IOS ) || defined( ANDROID )

        glBindFramebuffer( GL_FRAMEBUFFER, fbo.getFrameBuffer() );
        DEBUG_OPENGL();

#elif defined( QT )

        fbo.getFrameBuffer()->bind();

#else

        CCASSERT( false );

#endif

        currentFBOIndex = fboIndex;

        // Recreate the texture
#ifndef DXRENDERER
        glBindTexture( GL_TEXTURE_2D, getFrameBufferTexture( fboIndex ) );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, getWidth( fboIndex ), getHeight( fboIndex ), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
        gRenderer->GLBindTexture( GL_TEXTURE_2D, gEngine->textureManager->getCurrentGLTexture() );
#endif
    }
}


void CCFrameBufferManager::bindDefaultFrameBuffer()
{
    if( currentFBOIndex >= 0 )
    {

#if defined( IOS ) || defined( ANDROID )

        glBindFramebuffer( GL_FRAMEBUFFER, defaultFBO.getFrameBuffer() );
    	DEBUG_OPENGL();

#elif defined( QT )

        CCFrameBufferObject *currentFBO = fbos.list[currentFBOIndex];
        currentFBO->getFrameBuffer()->release();

#else

        CCASSERT( false );

#endif

        currentFBOIndex = -1;
    }
}


void CCFrameBufferManager::bindFrameBufferTexture(const int fboIndex)
{
    fbos.list[fboIndex]->bindRenderTexture();
}


GLuint CCFrameBufferManager::getFrameBufferTexture(const int fboIndex)
{
    return fbos.list[fboIndex]->getRenderTexture();
}


GLuint CCFrameBufferManager::getFrameBufferHandle(const int fboIndex)
{
    return fbos.list[fboIndex]->getFrameBufferHandle();
}
