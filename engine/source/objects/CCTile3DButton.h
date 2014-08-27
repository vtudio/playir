/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCTile3DButton.h
 * Description : A dynamic button widget.
 *
 * Created     : 14/09/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCTILE3DBUTTON_H__
#define __CCTILE3DBUTTON_H__


class CCTile3DButton : public CCTile3D
{
    typedef CCTile3D super;

protected:
    CCModelBase *tileModel;
    CCPrimitiveSquare *tileSquare;

    CCObjectText *textObject;

    CCInterpolatorV3<CCInterpolatorSin2Curve> tileScaleInterpolator;

    bool touchMovementAllowed;

    bool touchRotationAllowed;
    CCInterpolatorListV3<CCInterpolatorLinear> touchRotationInterpolator;
    float touchRotationMagnitude;
    float touchRotationSpeed;

    // Touch Position interpolation
    float touchDepressRange;
    float touchDepressDepth;
    CCVector3 touchDepressPosition;
    CCInterpolatorListV3<CCInterpolatorSin2Curve> touchDepressInterpolator;

public:
    CCInterpolatorListV3<CCInterpolatorX3Curve> *tileRotationInterpolator;



public:
    CCTile3DButton(CCSceneBase *scene);

    virtual void setupTile(const float size=1.0f, const char *text=NULL);
    virtual void setupTile(const float width, const float height, const char *text=NULL);

    // Create tile with width and textures aspect ratio
    void setupTextured(const char *textureName, CCResourceType resourceType, CCLambdaCallback *onDownloadCallback,
                       const CCTextureLoadOptions options=CCTextureLoadOptions());
    void setupTexturedWidth(const float width, const char *textureName, const CCResourceType resourceType,
                            const CCTextureLoadOptions options=CCTextureLoadOptions());
    void setupTexturedHeight(const float heigth, const char *textureName, const CCResourceType resourceType,
                             const CCTextureLoadOptions options=CCTextureLoadOptions());
    void setupTexturedFit(const float width, const float height, const char *textureName,
                          const CCTextureLoadOptions options=CCTextureLoadOptions());

    // Create tile with the text height
    void setupText(const char *text, const float height, const bool centered=true, const bool backgroundTile=false);

private:
    void construct(CCSceneBase *scene);
public:
    virtual void destruct();

	// CCRenderable
	virtual void refreshModelMatrix();

    virtual void setPositionXYZ(const float x, const float y, const float z=0.0f);

	// CCObject
    virtual void removeFromScene();
    virtual bool update(const CCTime &time);

    virtual void renderModel(const bool alpha);

public:
    CCObjectText* getTextObject() { return textObject; }
    const CCText& getText() { return textObject->getText(); }

    void setTileSize(const float size=1.0f);
    virtual void setTileSize(const float width, const float height);
    void setTileTexture(const char *textureName, CCResourceType resourceType, CCLambdaCallback *onDownloadCallback=NULL,
                        const CCTextureLoadOptions options=CCTextureLoadOptions());
    void setTileTexturedSize();
    void setTileTexturedWidth(const float width);
    void setTileTexturedHeight(const float height);
    void setTileTexturedFit(const float width, const float height, const bool crop=false);

    void setTileScale(const CCVector3 inScale, const bool interpolate=false, CCLambdaCallback *onInterpolated=NULL);

    void adjustTextureUVs(const float x1, const float y1, const float x2, const float y2);
    void resetTileUVs();
    void flipTileY();

    inline CCPrimitiveSquare* getTileSquare() { return tileSquare; }

    virtual void setText(const char *text, const bool resizeTile=false, const float height=-1.0f)
    {
        textObject->setText( text, height );
        if( resizeTile )
        {
            const float width = textObject->getWidth();
            const float height = textObject->getHeight();
            setCollisionBounds( width, height, CC_SMALLFLOAT );
            CCUpdateCollisions( this );
            if( tileSquare != NULL )
            {
                tileSquare->setScale( collisionSize.width, collisionSize.height );
            }
        }
    }

    void setTextPosition(const float x, const float y);

    void setTextFont(const char *font, const bool resizeTile=false)
    {
        textObject->setFont( font );
        if( resizeTile )
        {
            const float width = textObject->getWidth();
            const float height = textObject->getHeight();
            setCollisionBounds( width, height, CC_SMALLFLOAT );
            CCUpdateCollisions( this );
            if( tileSquare != NULL )
            {
                tileSquare->setScale( collisionSize.width, collisionSize.height );
            }
        }
    }

    virtual void setTextColour(const CCColour &inColour, const bool interpolate=false, CCLambdaCallback *inCallback=NULL);
	void setTextColourAlpha(const float inAlpha, const bool interpolate=false, CCLambdaCallback *inCallback=NULL);

    virtual void setTextHeight(const float height, const bool resizeTile=false)
    {
        textObject->setHeight( height );

        if( resizeTile )
        {
            const float width = textObject->getWidth();
            setCollisionBounds( width, height, CC_SMALLFLOAT );
            CCUpdateCollisions( this );
            if( tileSquare != NULL )
            {
                setTileSize( width, height );
            }
        }
    }

    void setTextBlinking(const bool toggle);

    void setReadDepth(const bool toggle) { readDepth = toggle; }

    // Touchable
    virtual bool handleProjectedTouch(const CCCameraProjectionResults &cameraProjectionResults,
                                      const CCCollideable *hitObject,
                                      const CCVector3 &hitPosition,
                                      const CCScreenTouches &touch,
                                      const CCTouchAction touchAction);

    // Called when the tile is touched
    virtual void touchActionPressed(const float x, const float y, const CCScreenTouches &touch, const CCTouchAction touchAction);

    // Called when a touch is moved over this tile
    virtual void touchActionMoved(const float x, const float y, const CCScreenTouches &touch, const CCTouchAction touchAction);

    // Called when the tile is released
    virtual void touchActionRelease(const CCTouchAction touchAction);

protected:
    // Callbacks
    virtual void handleTouchRelease();

public:
    inline void allowTouchRotation(const bool allow) { touchRotationAllowed = allow; }
    const CCVector3 getRotationTarget()
    {
        if( touchRotationInterpolator.interpolators.length > 0 )
        {
            return touchRotationInterpolator.interpolators.list[touchRotationInterpolator.interpolators.length-1]->getTarget();
        }
        return rotation;
    }

    void allowTouchMovement(const bool allow=true)
    {
        touchMovementAllowed = allow;
    }

    void setTouchRotationSpeed(const float speed)
    {
        touchRotationSpeed = speed;
    }

    void setTouchDepressRange(const float range)
    {
        touchDepressRange = range;
    }

    void setTouchDepressDepth(const float depth)
    {
        touchDepressDepth = depth;
    }

    inline CCInterpolatorV3<CCInterpolatorSin2Curve>& getTileScaleInterpolator() { return tileScaleInterpolator; }
};


#endif // __CCTILE3DBUTTON_H__
