/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCSceneBase.h
 * Description : Handles the drawing and updating of objects.
 *
 * Created     : 01/03/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCSCENEBASE_H__
#define __CCSCENEBASE_H__


class CCObject;
class CCCameraAppUI;

class CCSceneBase : public CCBaseType
{
public:
	CCSceneBase();
	virtual void destruct();
    virtual void deleteLater();

	virtual void setup() {};
	virtual void restart() {}

    void deleteLinkedScenesLater();
    inline bool shouldDelete() { return deleteMe; }

    // Called by the Engine to let the scene fetch and handle the controls
    virtual bool updateControls(const CCTime &time);

    // Called by updateControls or a user object perhaps rendering the scene in a frame buffer with specific touches to handle the controls with
    virtual bool handleTouches(const CCScreenTouches &touch1, const CCScreenTouches &touch2, const CCTime &time) { return false; }

    virtual bool shouldHandleBackButton() { return false; }
    virtual void handleBackButton() {}

    bool update(const CCTime &time);
    bool updateTask(const CCTime &time);

protected:
    virtual bool updateScene(const CCTime &time);
    virtual bool updateCamera(const CCTime &time);

public:
    virtual const CCCameraBase* getCamera() { return NULL; }
    virtual void resize() {}
    virtual void resized() {}

public:
    virtual bool render(const CCCameraBase *inCamera, const CCRenderPass pass, const bool alpha);

protected:
    virtual void renderObjects(const CCCameraBase *inCamera, const CCRenderPass pass, const bool alpha);

public:
	// For sorted objects, we get passed the object to draw here
	virtual void renderVisibleObject(CCObject *object, const CCCameraBase *inCamera, const CCRenderPass pass, const bool alpha);

    virtual bool postRender(const CCCameraBase *inCamera, const CCRenderPass pass, const bool alpha) { return false; }

	// Add object to the scene and place in the created list
	void addObject(CCObject *object);
	void removeObject(CCObject* object);

    void addCollideable(CCCollideable *collideable);
    void removeCollideable(CCCollideable *collideable);

    virtual void removeTile(class CCTile3DButton *tile) {}

    // Tells the scene to inform on deletes
    void setParent(CCSceneBase *inParent);
    virtual void deletingChild(CCSceneBase *inScene) {}

public:
    // Passes on renders and controls to child scenes
    void addChildScene(CCSceneBase *inScene);
    void removeChildScene(CCSceneBase *inScene);

    // Linked scenes are deleted along with this scene
    void linkScene(CCSceneBase *inScene);
    void unlinkScene(CCSceneBase *inScene);

    // Runs on native thread
    virtual void appPaused() {}
    virtual void appResumed() {}

public:
    bool enabled;

protected:
    bool deleteMe;

protected:
    CCPtrList<CCObject> objects;
    CCPtrList<CCCollideable> collideables;

    CCSceneBase *parentScene;
    CCPtrList<CCSceneBase> childScenes;
    CCPtrList<CCSceneBase> linkedScenes;

    float lifetime;
};


#endif // __CCSCENEBASE_H__
