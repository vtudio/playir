/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCTile3D.h
 * Description : Base class for our Tile widgets.
 *
 * Created     : 14/09/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCTILE3D_H__
#define __CCTILE3D_H__


class CCTouchable
{
public:
    CCTouchable()
    {
        touching = false;
        touchReleased = false;
    }

    inline float getTouchingTime() { return touchingTime; }

    virtual bool handleProjectedTouch(const CCCameraProjectionResults &cameraProjectionResults,
                                      const CCCollideable *hitObject,
                                      const CCVector3 &hitPosition,
                                      const CCScreenTouches &touch,
                                      const CCTouchAction touchAction) = 0;

    // Called when the tile is touched
    virtual void touchActionPressed(const float x, const float y, const CCScreenTouches &touch, const CCTouchAction touchAction) = 0;

    // Called when a touch is moved over this tile
    virtual void touchActionMoved(const float x, const float y, const CCScreenTouches &touch, const CCTouchAction touchAction) = 0;

    // Called when the tile is released
    virtual void touchActionRelease(const CCTouchAction touchAction) = 0;

protected:
    // Callbacks
    virtual void handleTouchRelease() = 0;
    virtual void onTouchPress()
    {
        CCLAMBDA_EMIT( onPress );
    }

    virtual void onTouchMove()
    {
        CCLAMBDA_EMIT( onMove );
    }

    virtual void onTouchRelease()
    {
        CCLAMBDA_EMIT( onRelease );
    }

    virtual void onTouchLoss()
    {
        CCLAMBDA_EMIT( onLoss );
    }

protected:
    bool touching;
    float touchingTime;
    bool touchReleased;

public:
    CCLAMBDA_SIGNAL onPress;
    CCLAMBDA_SIGNAL onMove;
    CCLAMBDA_SIGNAL onRelease;
    CCLAMBDA_SIGNAL onLoss;
};



class CCTile3D : public CCCollideable, public CCTouchable, public virtual CCActiveAllocation
{
public:
    typedef CCCollideable super;

    CCTile3D();
    virtual void destruct();

    // CCRenderable
    virtual void dirtyModelMatrix();
    virtual void setPositionXYZ(const float x, const float y, const float z);
    virtual void translate(const float x, const float y, const float z);

    // Positioning Tiles
    void positionTileY(float &y);
    virtual void positionTileBelow(CCTile3D *fromTile);
    void positionTileAbove(CCTile3D *fromTile);
    void positionTileRight(CCTile3D *fromTile);
    void positionTileLeft(CCTile3D *fromTile);

    void setTileMovement(const CCVector3 target);
    void setTileMovementX(const float x);
    void translateTileMovementX(const float x);
    void setTileMovementY(const float y);
    void setTileMovementXY(const float x, const float y);
    void setTileMovementYZ(const float y, const float z);
    void setTileMovementBelow(const CCTile3D *fromTile);
    const CCVector3 getTileMovementTarget() const;

    // Objects which move along with this tile, but contain handle their own collisions
    CCPtrList<CCObject> attachments;
};


#endif // __CCTILE3D_H__
