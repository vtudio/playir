/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCTile3DFrameBuffer.h
 * Description : A tile which displays the contents of a frame buffer
 *
 * Created     : 07/02/12
 *-----------------------------------------------------------
 */

#ifndef __CCTILE3DFRAMEBUFFER_H__
#define __CCTILE3DFRAMEBUFFER_H__


class CCTile3DFrameBuffer : public CCTile3DButton
{
public:
    typedef CCTile3DButton super;

    CCTile3DFrameBuffer(CCSceneBase *scene);
    virtual void destruct();

	// CCObject
    virtual bool update(const CCTime &time);

    virtual void renderModel(const bool alpha);

public:
    // Touchable
    virtual bool handleProjectedTouch(const CCCameraProjectionResults &cameraProjectionResults,
                                      const CCCollideable *hitObject,
                                      const CCVector3 &hitPosition,
                                      const CCScreenTouches &touch,
                                      const CCTouchAction touchAction);

    virtual bool handleSceneTouch(const CCCameraProjectionResults &cameraProjectionResults,
                                  const CCCollideable *hitObject,
                                  const CCVector3 &hitPosition,
                                  const CCScreenTouches &screenTouch,
                                  const CCTouchAction touchAction);

    void setFrameBufferID(const int frameBufferID);
    void linkScene(CCSceneBase *scene)
    {
        linkedScenes.add( scene );
        scene->enabled = false;
    }

    static void ResetRenderFlag()
    {
        renderedThisFrame = 0;
    }

protected:
    int frameBufferID;
    CCPtrList<CCSceneBase> linkedScenes;

    bool sceneControlsActive;
    CCScreenTouches touches[CCControls::max_touches];

    bool firstRender;
    static CCPtrList<CCTile3DFrameBuffer> frameBuffers;
    static uint renderedThisFrame;
};


#endif // __CCTILE3DFRAMEBUFFER_H__
