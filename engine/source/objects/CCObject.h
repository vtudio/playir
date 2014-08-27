/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCObject.h
 * Description : A scene managed object.
 *
 * Created     : 01/03/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

class CCObject : public CCRenderable
{
	typedef CCRenderable super;

protected:
    CCText objectID;        // Used for serverside communication

public:
    CCSceneBase *inScene;
    uint deleteMe;

    CCObject *parent;

    CCRenderPass renderPass;
    bool octreeRender;

protected:
    CCModelBase *model;
    bool readDepth, writeDepth;
    bool disableCulling, frontCulling;
    
    bool transparent, transparentParent;
    CCInterpolatorLinearColour *colourInterpolator;

    // If I have children, they will be offset from my position
    CCObjectPtrList<CCObject> children;
    CCObjectPtrList<CCUpdater> updaters;

    

public:
    CCObject(const long jsID=-1);
    virtual void destruct();

    const char* getServerObjectID() const
    {
        return objectID.buffer;
    }

    // CCRenderable
    virtual void dirtyWorldMatrix();

    virtual void setScene(CCSceneBase *scene);
    virtual void removeFromScene();

    // Delete the object in 2 frames.
    void deleteLater();

    inline bool isActive() { return deleteMe == 0; }
    virtual void deactivate() {};

    void addChild(CCObject *object, const int index=-1);
    bool removeChild(CCObject *object);

    // Remove an object from our child list and add it into the scene
    void moveChildToScene(CCObject *object, CCSceneBase *scene);

    void addUpdater(CCUpdater *updater);
    void removeUpdater(CCUpdater *updater);

    virtual bool shouldCollide(CCCollideable *collideWith, const bool initialCall);

    virtual bool update(const CCTime &time);
    virtual void renderObject(const CCCameraBase *camera, const bool alpha);
    
    virtual void renderModel(const bool alpha);

public:
    bool isTransparent() { return transparent; }
    void setTransparent(const bool toggle=true);
    void setTransparentParent(const bool toggle=true);

    CCInterpolatorLinearColour& getColourInterpolator();
	virtual void setColour(const CCColour &inColour, const bool interpolate=false, CCLambdaCallback *inCallback=NULL);
	void setColourAlpha(const float inAlpha, const bool interpolate=false, CCLambdaCallback *inCallback=NULL);

    CCModelBase* getModel() { return model; }
    
    void setModel(CCModelBase *model);
    void setReadDepth(const bool toggle);
    void setWriteDepth(const bool toggle);
    void setCulling(const bool toggle);
    void setFrontCulling(const bool toggle);
};
