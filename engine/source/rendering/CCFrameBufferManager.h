/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCFrameBufferManager.h
 * Description : Manages the creating and switching of frame buffers.
 *
 * Created     : 01/08/11
 * Author(s)   : Chris Wilson, Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCFRAMEBUFFERMANAGER_H__
#define __CCFRAMEBUFFERMANAGER_H__


#ifdef QT
#include <QGLFramebufferObject>
#define FBOType QGLFramebufferObject*
#else
#define FBOType GLuint
#endif

#include "CCTextureBase.h"


class CCFrameBufferObject : public CCTextureName
{
public:
    CCFrameBufferObject();
    CCFrameBufferObject(const char *inName, const int inWidth, const int inHeight);

    void setFrameBuffer(FBOType fbo) { frameBuffer = fbo; }
    FBOType getFrameBuffer() { return frameBuffer; }

    GLuint getRenderTexture() { return glName; }
    void setRenderTexture(GLuint inTexture) { glName = inTexture; }
    void bindRenderTexture();

    const CCText& getName() { return name; }
    GLuint getFrameBufferHandle();

#ifndef QT
    GLuint renderBuffer, depthBuffer, stencilBuffer;
#endif

    int width;
    int height;

private:
    CCText name;

    FBOType frameBuffer;
};



class CCFrameBufferManager
{
    friend class CCRenderer;
    friend class CCDeviceRenderer;

public:
    CCFrameBufferManager();
    ~CCFrameBufferManager();

    void setup();

    float getWidth(const int fboIndex);
    float getHeight(const int fboIndex);
    int getNumberOfFBOs() { return fbos.length; }

    // Creates framebuffer and returns index of frame buffer object to be used with other calls
    int findFrameBuffer(const char *name);
    int newFrameBuffer(const char *name, const int size, const bool depthBuffer, const bool stencilBuffer);
    void deleteFrameBuffer(const int fboIndex);

protected:
    void createFrameBuffer(CCFrameBufferObject &fbo, const bool useDepthBuffer, const bool useStencilBuffer);
    void destroyFrameBuffer(CCFrameBufferObject &fbo);
    void destoryAllFrameBuffers();

public:
    // Sets the currently active framebuffer
    void bindFrameBuffer(const int fboIndex);
    void bindDefaultFrameBuffer();

    // Sets the active texture to the texture bound to the given frame buffer
    void bindFrameBufferTexture(const int fboIndex);

#ifndef QT
    // Returns the renderbuffer attached to the default framebuffer
    GLuint getDefaultRenderBuffer() { return defaultFBO.renderBuffer; }
#endif

    // Returns the OpenGL handle of the texture attached to the given frame buffer
    GLuint getFrameBufferTexture(const int fboIndex);

    // Returns the OpenGL handle for given frame buffer
    GLuint getFrameBufferHandle(const int fboIndex);

private:
    CCFrameBufferObject defaultFBO;
    int currentFBOIndex;
    CCPtrList<CCFrameBufferObject> fbos;
};


#endif // __CCFRAMEBUFFERMANAGER_H__
