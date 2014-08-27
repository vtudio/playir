/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCCameraBase.h
 * Description : Common functionality for scene cameras.
 *
 * Created     : 01/03/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCCAMERABASE_H__
#define __CCCAMERABASE_H__


#include "CCVectors.h"
#include "CCMatrix.h"

enum CCCameraFrustumPlanes
{
	frustum_right,
	frustum_left,
	frustum_bottom,
	frustum_top,
	frustum_far,
	frustum_near,
	frustum_max,
};

struct CCCameraProjectionResults
{
    CCVector3 vNear;
    CCVector3 vFar;
    CCVector3 vDirection;
    CCVector3 vLookAt;
};

class CCCameraBase
{
public:
	CCCameraBase();
    virtual ~CCCameraBase();

    void setupViewport(float x=0.0f, float y=0.0f, float width=1.0f, float height=1.0f, const int frameBufferID=-1);
    void recalcViewport();
    void setViewport();
    virtual void setPerspective(const float perspective);

    bool project3D(float x, float y);
    void project3DY(CCVector3 *result, float x, float y, float offset=-1.0f);
    void project3DZ(CCVector3 *result, float x, float y, float offset=-1.0f);

    void scanFOVBounds();

    struct FOVBounds
    {
        CCPoint min;
        CCPoint max;
        CCPoint size;
	};
	enum FOVScanTypes
	{
		scan_flat,
		max_fov_scans
	};
    const FOVBounds* getFOVBounds() const { return fovBounds; }

    void setCurrentCamera();
	virtual void update();

	// Update lookAt and offset
    virtual void setLookAt(const CCVector3 &newLookAt);
    void setLookAtY(const float y);
	const CCVector3& getLookAt() const { return lookAt; }

    void setWidth(const float width);
    virtual void setOffset(const CCVector3 &newOffset);
    const CCVector3& getOffset() { return offset; }
protected:
	void updateOffset(const CCVector3 &offsetTarget);

public:
    bool setLookAt(const CCVector3 &lookAtTarget, const CCVector3 &offsetTarget);

    inline const CCSize& getFrustumSize() { return frustumSize; }
    inline CCVector3& getRotationVector() { return rotation; }

    virtual void setRotationX(const float inRotation);
    virtual void setRotationY(const float inRotation);
    virtual void setRotationZ(const float inRotation);
    void incrementRotationX(const float increment);
    void incrementRotationY(const float increment);
    void incrementRotationZ(const float increment);

    bool isUpdating() { return updating; }
	void flagUpdate() { updating = true; }
    void flagFOVScan() { updateFOV = true; }

    inline int getFrameBufferID() { return frameBufferID; }

	const CCVector3 getDirection();

    inline const CCVector3& getRotatedPosition() const { return rotatedPosition; }
    inline float getRotationY() const { return rotation.y; }

	virtual CCMatrix& getViewMatrix() { return viewMatrix; }

    void updateControls();

public:
    static void SetVisibleSortFunction(int (*callback)(const void *, const void *) );

    // Don't use the octrees for scanning the visible objects
    // Optimization if you know what objects the camera will be drawing
    void setCollideables(const CCPtrList<CCCollideable> *inCollideableList);
protected:
	virtual void updateVisibleCollideables();

    // glu-based camera functionality
	void GluPerspective(float fovy, float aspect);

    // Orient camera matrix to look at
    void GluLookAt(CCMatrix &viewMatrix,
                   float eyex, float eyey, float eyez,
                   float centerx, float centery, float centerz,
                   float upx, float upy, float upz);

    // Unproject a 2d screen space point into 3d space
    bool GluUnProject(const float x, const float y, const float z, CCVector3 &result);

    // Project a 3d point into 2d screen space
    bool GluProject(class CCRenderable *object, CCVector3 &result);

    void ExtractFrustum();

public:
    bool isEnabled() { return enabled; }
    bool hasUpdated() { return updatedPosition; }

    const CCMatrix& getProjectionMatrix() { return projectionMatrix; }
    const CCScreenTouches* getRelativeTouches() { return cameraTouches; }
    const CCCameraProjectionResults& getProjectionResults() { return projectionResults; }

    float getViewportX() { return viewportX; }
    float getViewportY() { return viewportY; }
    float getViewportX2() { return viewportX2; }
    float getViewportY2() { return viewportY2; }
    float getViewportWidth() { return viewportWidth; }
    float getViewportHeight() { return viewportHeight; }

    float getAspectRatio() { return orientedAspectRatio; }

protected:
	bool enabled;
    int frameBufferID;

    // Are we updating
    bool updating;

    // Has the camera been updated this frame
    bool updatedPosition;

    // Should we update our FOV
    bool updateFOV;
	FOVBounds fovBounds[max_fov_scans];

    int viewport[4];
    float viewportX, viewportY, viewportX2, viewportY2,
          viewportWidth, viewportHeight, invViewportWidth, invViewportHeight;
    float actualAspectRatio;
    float orientedAspectRatio;
    float perspective;
    float zNear, zFar;
	CCMatrix projectionMatrix;

	CCVector3 offset;
	CCVector3 position;
	CCVector3 rotatedPosition;

	CCVector3 lookAt;
	CCVector3 rotation;

	CCMatrix viewMatrix;

	float frustum[frustum_max][4];
    CCPoint frustumMin, frustumMax;
    CCSize frustumSize;

    // Are touches releative to the camera
    CCScreenTouches cameraTouches[CCControls::max_touches];

    // The results of a projection scan
    CCCameraProjectionResults projectionResults;

    // List of objects that the camera will render, if NULL render everything
    const CCPtrList<CCCollideable> *collideables;

public:
    static CCCameraBase *CurrentCamera;
#define MAX_PUSHES 15
    CCMatrix pushedMatrix[MAX_PUSHES];
    int currentPush;

#define MAX_VISIBLE_COLLIDEABLES 2048
    CCPtrList<CCCollideable> visibleCollideables;
    int sortedVisibleCollideables[MAX_VISIBLE_COLLIDEABLES];

    bool alwaysOnTop;
};


// Attempt to simulate OpenGL 1.1 interface to model matrix
extern void GLResetMatrix();
extern void GLPushMatrix();
extern void GLPopMatrix();
extern void GLLoadIdentity();
extern void GLMultMatrixf(CCMatrix &matrix);
extern void GLTranslatef(float tx, float ty, float tz);
extern void GLScalef(float sx, float sy, float sz);
extern void GLRotatef(float angle, float x, float y, float z);


#endif // __CCCAMERABASE_H__
