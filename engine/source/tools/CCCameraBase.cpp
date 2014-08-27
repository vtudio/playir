/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCCameraBase.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCObjects.h"
#include "CCAppManager.h"

#ifdef IOS
#include <OpenGLES/ES1/gl.h>
#endif

CCCameraBase *CCCameraBase::CurrentCamera = NULL;


CCCameraBase::CCCameraBase()
{
    enabled = true;
    frameBufferID = -1;

    updating = true;
    updatedPosition = false;
    updateFOV = false;

    actualAspectRatio = 1.0f;
    orientedAspectRatio = 1.0f;
    perspective = 60.0f;
    zNear = 10.0f;
    zFar = 40000.0f;

    CCMatrixLoadIdentity( viewMatrix );

    // Initialise frustum
    for( uint x=0; x<6; ++x )
    {
        for( uint y=0; y<4; ++y )
        {
             frustum[x][y] = 0.0f;
        }
    }

    collideables = NULL;
    
    alwaysOnTop = false;

    currentPush = 0;
    CCCameraBase::CurrentCamera = this;
    GLResetMatrix();
}


CCCameraBase::~CCCameraBase()
{
    if( this == CCCameraBase::CurrentCamera )
    {
        CCCameraBase::CurrentCamera = NULL;
    }
}


void CCCameraBase::setupViewport(float x, float y, float width, float height, const int frameBufferID)
{
    this->frameBufferID = frameBufferID;

    viewportX = x;
    viewportY = y;
    viewportX2 = x+width;
    viewportY2 = y+height;
    viewportWidth = width;
    viewportHeight = height;
    invViewportWidth = 1.0f / viewportWidth;
    invViewportHeight = 1.0f / viewportHeight;

    if( frameBufferID == -1 )
    {
        const CCTarget<float> &Orientation = CCAppManager::GetOrientation();
        if( Orientation.target == 270.0f )
        {
            CCFloatSwap( width, height );
            CCFloatSwap( x, y );
            x = 1.0f - width - x;
        }
        else if( Orientation.target == 90.0f )
        {
            CCFloatSwap( width, height );
            CCFloatSwap( x, y );
            y = 1.0f - height - y;
        }
        else if( Orientation.target == 180.0f )
        {
            x = 1.0f - width - x;
            y = 1.0f - height - y;
        }
        else
        {
        }
    }

    const float invY = ( 1.0f-height ) - y;

    const float frameBufferWidth = gRenderer->frameBufferManager.getWidth( frameBufferID );
    const float frameBufferHeight = gRenderer->frameBufferManager.getHeight( frameBufferID );
    const float definedWidth = width * frameBufferWidth;
    const float definedHeight = height * frameBufferHeight;
    viewport[0] = (int)( x * frameBufferWidth );
    viewport[1] = (int)( invY * frameBufferHeight );
    viewport[2] = (int)( definedWidth );
    viewport[3] = (int)( definedHeight );

    orientedAspectRatio = actualAspectRatio = definedWidth / definedHeight;
    if( frameBufferID == -1 )
    {
        if( CCAppManager::IsPortrait() == false )
        {
            orientedAspectRatio = definedHeight / definedWidth;
        }
    }

    setPerspective( perspective );
}


void CCCameraBase::recalcViewport()
{
    setupViewport( viewportX, viewportY, viewportWidth, viewportHeight, frameBufferID );
}


void CCCameraBase::setViewport()
{
    gRenderer->GLViewport( viewport[0], viewport[1], viewport[2], viewport[3] );
    gRenderer->GLScissor( viewport[0], viewport[1], viewport[2], viewport[3] );
}


void CCCameraBase::setPerspective(const float perspective)
{
    this->perspective = perspective;
    GluPerspective( perspective, actualAspectRatio );
}


bool CCCameraBase::project3D(float x, float y)
{
    if( frameBufferID == -1 )
    {
        CCAppManager::ProjectOrientation( x, y );
    }

	x *= viewport[2];
    x += viewport[0];
	y *= viewport[3];
    y += viewport[1];

    if( GluUnProject( x, y, 0.0f, projectionResults.vNear ) &&
        GluUnProject( x, y, 1.0f, projectionResults.vFar ) )
	{
		// Figure out our ray's direction
        projectionResults.vDirection = projectionResults.vFar;
        projectionResults.vDirection.sub( projectionResults.vNear );
        projectionResults.vDirection.unitize();

        projectionResults.vLookAt = projectionResults.vNear;
        CCVector3 projectionOffset = CCVector3MulResult( projectionResults.vDirection, projectionResults.vNear.z );
        projectionResults.vLookAt.add( projectionOffset );
		return true;
	}

	return false;
}


void CCCameraBase::project3DY(CCVector3 *result, float x, float y, float offset)
{
    if( project3D( x, y ) )
	{
		// Cast the ray from our near plane
		if( offset == -1.0f )
		{
            offset = projectionResults.vNear.y / fabsf( projectionResults.vDirection.y );
		}

        *result = CCVector3MulResult( projectionResults.vDirection, offset );
        result->add( projectionResults.vNear );
	}
}


void CCCameraBase::project3DZ(CCVector3 *result, float x, float y, float offset)
{
    if( project3D( x, y ) )
    {
        // Cast the ray from our near plane
        if( offset == -1.0f )
        {
            offset = projectionResults.vNear.z / fabsf( projectionResults.vDirection.z );
        }

        *result = CCVector3MulResult( projectionResults.vDirection, offset );
        result->add( projectionResults.vNear );
    }
}


void CCCameraBase::scanFOVBounds()
{
    for( uint i=0; i<max_fov_scans; ++i )
    {
        if( i == scan_flat )
        {
            CCMatrixLoadIdentity( viewMatrix );
            CCMatrixRotateDegrees( viewMatrix, CCAppManager::GetOrientation().current, 0.0f, 0.0f, 1.0f );
            GluLookAt( viewMatrix,
                       position.x, position.y, position.z,	// Position
                       lookAt.x, lookAt.y, lookAt.z,		// Looking At
                       0.0f, 1.0f, 0.0f );					// Up
        }

        static CCVector3 minPlane, maxPlane;
        project3DZ( &minPlane, 0.0f, 1.0f );
        project3DZ( &maxPlane, 1.0f, 0.0f );

        FOVBounds &bounds = fovBounds[i];
        bounds.size.x = fabsf( maxPlane.x - minPlane.x );
        bounds.size.y = fabsf( maxPlane.y - minPlane.y );
        bounds.min.x = minPlane.x;
        bounds.min.y = minPlane.y;
        bounds.max.x = maxPlane.x;
        bounds.max.y = maxPlane.y;
    }
}


void CCCameraBase::setCurrentCamera()
{
    CCCameraBase::CurrentCamera = this;
}


void CCCameraBase::update()
{
#if defined PROFILEON
    CCProfiler profile( "CCCameraBase::update()" );
#endif

    setCurrentCamera();

    if( updatedPosition )
    {
        updatedPosition = false;
    }

    if( updateFOV )
    {
        updateFOV = false;
        scanFOVBounds();
    }

	CCMatrixLoadIdentity( viewMatrix );
    if( frameBufferID == -1 )
    {
        CCMatrixRotateDegrees( viewMatrix, CCAppManager::GetOrientation().current, 0.0f, 0.0f, 1.0f );
    }
    GluLookAt( viewMatrix,
			   rotatedPosition.x, rotatedPosition.y, rotatedPosition.z,	// Position
			   lookAt.x, lookAt.y, lookAt.z,                            // Looking At
			   0.0f, 1.0f, 0.0f );										// Up
    ExtractFrustum();

    updateVisibleCollideables();
}


void CCCameraBase::setLookAt(const CCVector3 &newLookAt)
{
    lookAt = newLookAt;
}


void CCCameraBase::setLookAtY(const float y)
{
    setLookAt( CCVector3( lookAt.x, y, lookAt.z ) );
}


void CCCameraBase::setWidth(const float width)
{
    float offset = width;
    if( CCAppManager::IsPortrait() )
    {
		offset /= frustumSize.width;
	}
	else
	{
		offset /= frustumSize.height;
	}
    setOffset( CCVector3( 0.0f, 0.0f, offset ) );
}


void CCCameraBase::setOffset(const CCVector3 &offsetTarget)
{
    updateOffset( offsetTarget );
}


void CCCameraBase::updateOffset(const CCVector3 &offsetTarget)
{
	offset = offsetTarget;

	position = lookAt;
	position.add( offset );
	rotatedPosition = position;

	CCRotateAboutX( rotatedPosition, rotation.x, position, lookAt );
	CCRotateAboutY( rotatedPosition, rotation.y, rotatedPosition, lookAt );
}


bool CCCameraBase::setLookAt(const CCVector3 &lookAtTarget, const CCVector3 &offsetTarget)
{
    if( updating )
    {
        updating = false;

        if( CCVector3Equals( lookAt, lookAtTarget ) == false || CCVector3Equals( offset, offsetTarget ) == false )
        {
            updatedPosition = true;
            lookAt = lookAtTarget;

            updateOffset( offsetTarget );
            return true;
        }
    }

    return false;
}


const CCVector3 CCCameraBase::getDirection()
{
    CCVector3 direction = lookAt;
    direction.sub( rotatedPosition );
    CCVector3Normalize( direction );
    return direction;
}


void CCCameraBase::updateControls()
{
    const CCScreenTouches *screenTouches = gEngine->controls->getScreenTouches();
    for( int i=0; i<CCControls::max_touches; ++i )
    {
        const CCScreenTouches &screenTouch = screenTouches[i];
        CCScreenTouches &cameraTouch = cameraTouches[i];
        cameraTouch = screenTouch;

#ifdef DEBUGON
        if( cameraTouch.usingTouch != NULL )
        {
            cameraTouch.usingTouch = cameraTouch.usingTouch;
        }
#endif

        cameraTouch.startPosition.x -= viewportX;
        cameraTouch.startPosition.x *= invViewportWidth;
        cameraTouch.startPosition.y -= viewportY;
        cameraTouch.startPosition.y *= invViewportHeight;

        cameraTouch.position.x -= viewportX;
        cameraTouch.position.x *= invViewportWidth;
        cameraTouch.position.y -= viewportY;
        cameraTouch.position.y *= invViewportHeight;

        cameraTouch.delta.x *= invViewportWidth;
        cameraTouch.delta.y *= invViewportHeight;
        cameraTouch.totalDelta.x *= invViewportWidth;
        cameraTouch.totalDelta.y *= invViewportHeight;
        cameraTouch.lastTotalDelta.x *= invViewportWidth;
        cameraTouch.lastTotalDelta.y *= invViewportHeight;

        for( int deltaIndex=0; deltaIndex<CCScreenTouches::max_last_deltas; ++deltaIndex )
        {
            cameraTouch.lastDeltas[deltaIndex] = screenTouch.lastDeltas[deltaIndex];
            cameraTouch.lastDeltas[deltaIndex].delta.x *= invViewportWidth;
            cameraTouch.lastDeltas[deltaIndex].delta.y *= invViewportHeight;
        }
    }
}


void CCCameraBase::setRotationX(const float inRotation)
{
    rotation.x = inRotation;
    updating = true;
}


void CCCameraBase::setRotationY(const float inRotation)
{
    rotation.y = inRotation;
    updating = true;
}


void CCCameraBase::setRotationZ(const float inRotation)
{
    rotation.z = inRotation;
    updating = true;
}


void CCCameraBase::incrementRotationX(const float increment)
{
    rotation.x += increment;
    setRotationX( rotation.x );
}


void CCCameraBase::incrementRotationY(const float increment)
{
    rotation.y += increment;
    setRotationY( rotation.y );
}


void CCCameraBase::incrementRotationZ(const float increment)
{
    rotation.z += increment;
    setRotationZ( rotation.z );
}


static int (*VisibleSortFunction)(const void *, const void *) = NULL;
void CCCameraBase::SetVisibleSortFunction(int (*callback)(const void *, const void *) )
{
	VisibleSortFunction = callback;
}


void CCCameraBase::setCollideables(const CCPtrList<CCCollideable> *inCollideableList)
{
    collideables = inCollideableList;
}


void CCCameraBase::updateVisibleCollideables()
{
#if defined PROFILEON
    CCProfiler profile( "CCCameraBase::updateVisibleCollideables()" );
#endif

    // Update visible objects
    if( collideables != NULL )
    {
        CCScanVisibleCollideables( frustum, *collideables, visibleCollideables );
    }
    else
    {
        CCOctreeScanVisibleCollideables( frustum, visibleCollideables );
    }

    for( int i=0; i<visibleCollideables.length; ++i )
    {
        sortedVisibleCollideables[i] = i;
    }

    qsort( sortedVisibleCollideables, visibleCollideables.length, sizeof( int ), VisibleSortFunction );
}


// glu based camera functionality
void CCCameraBase::GluPerspective(float fovy, float aspect)
{
    frustumMax.y = zNear * tanf( fovy * CC_PI / 360.0f );
	frustumMin.y = -frustumMax.y;
	frustumMax.x = frustumMax.y * aspect;
	frustumMin.x = frustumMin.y * aspect;

	const float zNearScale = 1.0f / zNear;
	frustumSize.width = ( frustumMax.x + -frustumMin.x ) * zNearScale;
	frustumSize.height = ( frustumMax.y + -frustumMin.y ) * zNearScale;

    if( gRenderer->openGL2() )
    {
        CCMatrixLoadIdentity( projectionMatrix );
        CCMatrixFrustum( projectionMatrix, frustumMin.x, frustumMax.x, frustumMin.y, frustumMax.y, zNear, zFar );
    }
    else
    {
#ifdef IOS

        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        glFrustumf( frustumMin.x, frustumMax.x, frustumMin.y, frustumMax.y, zNear, zFar );
        glGetFloatv( GL_PROJECTION_MATRIX, projectionMatrix.data() );

#endif
    }
}


void CCCameraBase::GluLookAt(CCMatrix &viewMatrix,
                             float eyex, float eyey, float eyez,
                             float centerx, float centery, float centerz,
                             float upx, float upy, float upz)
{
	float x[3], y[3], z[3];
	float mag;

	/* Make rotation matrix */

	/* Z vector */
	z[0] = eyex - centerx;
	z[1] = eyey - centery;
	z[2] = eyez - centerz;
	mag = sqrtf( z[0] * z[0] + z[1] * z[1] + z[2] * z[2] );
	if( mag )
	{	/* mpichler, 19950515 */
		z[0] /= mag;
		z[1] /= mag;
		z[2] /= mag;
	}

	/* Y vector */
	y[0] = upx;
	y[1] = upy;
	y[2] = upz;

	/* X vector = Y cross Z */
	x[0] = y[1] * z[2] - y[2] * z[1];
	x[1] = -y[0] * z[2] + y[2] * z[0];
	x[2] = y[0] * z[1] - y[1] * z[0];

	/* Recompute Y = Z cross X */
	y[0] = z[1] * x[2] - z[2] * x[1];
	y[1] = -z[0] * x[2] + z[2] * x[0];
	y[2] = z[0] * x[1] - z[1] * x[0];

	/* mpichler, 19950515 */
	/* cross product gives area of parallelogram, which is < 1.0 for
	 * non-perpendicular unit-length vectors; so normalize x, y here
	 */

	mag = sqrtf( x[0] * x[0] + x[1] * x[1] + x[2] * x[2] );
	if( mag )
	{
		x[0] /= mag;
		x[1] /= mag;
		x[2] /= mag;
	}

	mag = sqrtf( y[0] * y[0] + y[1] * y[1] + y[2] * y[2] );
	if( mag )
	{
		y[0] /= mag;
		y[1] /= mag;
		y[2] /= mag;
	}

	CCMatrix m;
    m.m[0][0] = x[0];
	m.m[1][0] = x[1];
	m.m[2][0] = x[2];
	m.m[3][0] = 0.0f;
	m.m[0][1] = y[0];
	m.m[1][1] = y[1];
	m.m[2][1] = y[2];
	m.m[3][1] = 0.0f;
	m.m[0][2] = z[0];
	m.m[1][2] = z[1];
	m.m[2][2] = z[2];
	m.m[3][2] = 0.0f;
	m.m[0][3] = 0.0f;
	m.m[1][3] = 0.0f;
	m.m[2][3] = 0.0f;
	m.m[3][3] = 1.0f;

	CCMatrixMultiply( viewMatrix, m, viewMatrix );

	/* Translate Eye to Origin */
	CCMatrixTranslate( viewMatrix, -eyex, -eyey, -eyez );
}


static bool gluUnProject(float winX, float winY, float winZ,
                         const float viewMatrix[16],
                         const float projMatrix[16],
                         const int viewport[4],
                         float *objX, float *objY, float *objZ)
{
	float finalMatrix[16];
	float in[4];
	float out[4];

	CCMatrixMulMat( viewMatrix, projMatrix, finalMatrix );
	if( CCMatrixInvert( finalMatrix, finalMatrix ) == false )
	{
		return false;
	}

	in[0] = winX;
	in[1] = winY;
	in[2] = winZ;
	in[3] = 1.0f;

	/* Map x and y from window coordinates */
	in[0] = ( in[0] - viewport[0] ) / viewport[2];
	in[1] = ( in[1] - viewport[1] ) / viewport[3];

	/* Map to range -1 to 1 */
	in[0] = in[0] * 2 - 1;
	in[1] = in[1] * 2 - 1;
	in[2] = in[2] * 2 - 1;

	CCMatrixMulVec( finalMatrix, in, out );
	if( out[3] == 0.0 )
	{
		return false;
	}

	out[0] /= out[3];
	out[1] /= out[3];
	out[2] /= out[3];
	*objX = out[0];
	*objY = out[1];
	*objZ = out[2];

	return true;
}


bool CCCameraBase::GluUnProject(const float x, const float y, const float z, CCVector3 &result)
{
    return gluUnProject( x, y, z, getViewMatrix().data(), projectionMatrix.data(), viewport, &result.x, &result.y, &result.z );
}


static bool gluProject(float objX, float objY, float objZ,
                       const float viewMatrix[16],
                       const float projMatrix[16],
                       const int viewport[4],
                       float *winX, float *winY, float *winZ,
                       const bool projectOrientation=true)
{
    float in[4];
    float out[4];

    in[0] = objX;
    in[1] = objY;
    in[2] = objZ;
    in[3] = 1.0f;

	CCMatrixMulVec( viewMatrix, in, out );
    CCMatrixMulVec( projMatrix, out, in );

	if( in[3] == 0.0)
	{
		return false;
	}

    in[0] /= in[3];
    in[1] /= in[3];
    in[2] /= in[3];
    /* Map x, y and z to range 0-1 */
    in[0] = in[0] * 0.5f + 0.5f;
    in[1] = in[1] * 0.5f + 0.5f;
    in[2] = in[2] * 0.5f + 0.5f;

    /* Map x,y to viewport */
    //in[0] = in[0] * viewport[2] + viewport[0];
    //in[1] = in[1] * viewport[3] + viewport[1];

	in[1] = 1.0f - in[1];

	if( in[3] < 0.0 )
	{
		in[0] *= -1.0f;
		in[1] *= -1.0f;
	}

    if( projectOrientation )
    {
        CCAppManager::ProjectOrientation( in[0], in[1] );
    }

    *winX = in[0];
    *winY = in[1];
    *winZ = in[2];

    return true;
}


bool CCCameraBase::GluProject(CCRenderable *object, CCVector3 &result)
{
	return gluProject( object->getConstPosition().x, object->getConstPosition().y, object->getConstPosition().z,
					   getViewMatrix().data(),
					   projectionMatrix.data(),
					   viewport,
					   &result.x, &result.y, &result.z,
                       frameBufferID==-1 );
}


void CCCameraBase::ExtractFrustum()
{
    const float *proj = projectionMatrix.data();
	const float *modv = viewMatrix.data();
	float clip[16];
	float t;

	/* Get the current MODELVIEW matrix from OpenGL */
	//glGetFloatv( GL_MODELVIEW_MATRIX, modv );

	/* Combine the two matrices (multiply projection by modelview) */
	clip[ 0] = modv[ 0] * proj[ 0] + modv[ 1] * proj[ 4] + modv[ 2] * proj[ 8] + modv[ 3] * proj[12];
	clip[ 1] = modv[ 0] * proj[ 1] + modv[ 1] * proj[ 5] + modv[ 2] * proj[ 9] + modv[ 3] * proj[13];
	clip[ 2] = modv[ 0] * proj[ 2] + modv[ 1] * proj[ 6] + modv[ 2] * proj[10] + modv[ 3] * proj[14];
	clip[ 3] = modv[ 0] * proj[ 3] + modv[ 1] * proj[ 7] + modv[ 2] * proj[11] + modv[ 3] * proj[15];

	clip[ 4] = modv[ 4] * proj[ 0] + modv[ 5] * proj[ 4] + modv[ 6] * proj[ 8] + modv[ 7] * proj[12];
	clip[ 5] = modv[ 4] * proj[ 1] + modv[ 5] * proj[ 5] + modv[ 6] * proj[ 9] + modv[ 7] * proj[13];
	clip[ 6] = modv[ 4] * proj[ 2] + modv[ 5] * proj[ 6] + modv[ 6] * proj[10] + modv[ 7] * proj[14];
	clip[ 7] = modv[ 4] * proj[ 3] + modv[ 5] * proj[ 7] + modv[ 6] * proj[11] + modv[ 7] * proj[15];

	clip[ 8] = modv[ 8] * proj[ 0] + modv[ 9] * proj[ 4] + modv[10] * proj[ 8] + modv[11] * proj[12];
	clip[ 9] = modv[ 8] * proj[ 1] + modv[ 9] * proj[ 5] + modv[10] * proj[ 9] + modv[11] * proj[13];
	clip[10] = modv[ 8] * proj[ 2] + modv[ 9] * proj[ 6] + modv[10] * proj[10] + modv[11] * proj[14];
	clip[11] = modv[ 8] * proj[ 3] + modv[ 9] * proj[ 7] + modv[10] * proj[11] + modv[11] * proj[15];

	clip[12] = modv[12] * proj[ 0] + modv[13] * proj[ 4] + modv[14] * proj[ 8] + modv[15] * proj[12];
	clip[13] = modv[12] * proj[ 1] + modv[13] * proj[ 5] + modv[14] * proj[ 9] + modv[15] * proj[13];
	clip[14] = modv[12] * proj[ 2] + modv[13] * proj[ 6] + modv[14] * proj[10] + modv[15] * proj[14];
	clip[15] = modv[12] * proj[ 3] + modv[13] * proj[ 7] + modv[14] * proj[11] + modv[15] * proj[15];

	/* Extract the numbers for the RIGHT plane */
	frustum[frustum_right][0] = clip[ 3] - clip[ 0];
	frustum[frustum_right][1] = clip[ 7] - clip[ 4];
	frustum[frustum_right][2] = clip[11] - clip[ 8];
	frustum[frustum_right][3] = clip[15] - clip[12];

	/* Normalize the result */
	t = sqrtf( frustum[frustum_right][0] * frustum[frustum_right][0] + frustum[frustum_right][1] * frustum[frustum_right][1] + frustum[frustum_right][2] * frustum[frustum_right][2] );
	frustum[frustum_right][0] /= t;
	frustum[frustum_right][1] /= t;
	frustum[frustum_right][2] /= t;
	frustum[frustum_right][3] /= t;

	/* Extract the numbers for the LEFT plane */
	frustum[frustum_left][0] = clip[ 3] + clip[ 0];
	frustum[frustum_left][1] = clip[ 7] + clip[ 4];
	frustum[frustum_left][2] = clip[11] + clip[ 8];
	frustum[frustum_left][3] = clip[15] + clip[12];

	/* Normalize the result */
	t = sqrtf( frustum[frustum_left][0] * frustum[frustum_left][0] + frustum[frustum_left][1] * frustum[frustum_left][1] + frustum[frustum_left][2] * frustum[frustum_left][2] );
	frustum[frustum_left][0] /= t;
	frustum[frustum_left][1] /= t;
	frustum[frustum_left][2] /= t;
	frustum[frustum_left][3] /= t;

	/* Extract the BOTTOM plane */
	frustum[frustum_bottom][0] = clip[ 3] + clip[ 1];
	frustum[frustum_bottom][1] = clip[ 7] + clip[ 5];
	frustum[frustum_bottom][2] = clip[11] + clip[ 9];
	frustum[frustum_bottom][3] = clip[15] + clip[13];

	/* Normalize the result */
	t = sqrtf( frustum[frustum_bottom][0] * frustum[frustum_bottom][0] + frustum[frustum_bottom][1] * frustum[frustum_bottom][1] + frustum[frustum_bottom][2] * frustum[frustum_bottom][2] );
	frustum[frustum_bottom][0] /= t;
	frustum[frustum_bottom][1] /= t;
	frustum[frustum_bottom][2] /= t;
	frustum[frustum_bottom][3] /= t;

	/* Extract the TOP plane */
	frustum[frustum_top][0] = clip[ 3] - clip[ 1];
	frustum[frustum_top][1] = clip[ 7] - clip[ 5];
	frustum[frustum_top][2] = clip[11] - clip[ 9];
	frustum[frustum_top][3] = clip[15] - clip[13];

	/* Normalize the result */
	t = sqrtf( frustum[frustum_top][0] * frustum[frustum_top][0] + frustum[frustum_top][1] * frustum[frustum_top][1] + frustum[frustum_top][2] * frustum[frustum_top][2] );
	frustum[frustum_top][0] /= t;
	frustum[frustum_top][1] /= t;
	frustum[frustum_top][2] /= t;
	frustum[frustum_top][3] /= t;

	/* Extract the FAR plane */
	frustum[frustum_far][0] = clip[ 3] - clip[ 2];
	frustum[frustum_far][1] = clip[ 7] - clip[ 6];
	frustum[frustum_far][2] = clip[11] - clip[10];
	frustum[frustum_far][3] = clip[15] - clip[14];

	/* Normalize the result */
	t = sqrtf( frustum[frustum_far][0] * frustum[frustum_far][0] + frustum[frustum_far][1] * frustum[frustum_far][1] + frustum[frustum_far][2] * frustum[frustum_far][2] );
	frustum[frustum_far][0] /= t;
	frustum[frustum_far][1] /= t;
	frustum[frustum_far][2] /= t;
	frustum[frustum_far][3] /= t;

	/* Extract the NEAR plane */
	frustum[frustum_near][0] = clip[ 3] + clip[ 2];
	frustum[frustum_near][1] = clip[ 7] + clip[ 6];
	frustum[frustum_near][2] = clip[11] + clip[10];
	frustum[frustum_near][3] = clip[15] + clip[14];

	/* Normalize the result */
	t = sqrtf( frustum[frustum_near][0] * frustum[frustum_near][0] + frustum[frustum_near][1] * frustum[frustum_near][1] + frustum[frustum_near][2] * frustum[frustum_near][2] );
	frustum[frustum_near][0] /= t;
	frustum[frustum_near][1] /= t;
	frustum[frustum_near][2] /= t;
	frustum[frustum_near][3] /= t;
}


void GLResetMatrix()
{
	if( CCCameraBase::CurrentCamera != NULL )
	{
        CCASSERT( CCCameraBase::CurrentCamera->currentPush == 0 );
		CCCameraBase::CurrentCamera->currentPush = 0;
		GLLoadIdentity();
	}
}


void GLPushMatrix()
{
	if( CCCameraBase::CurrentCamera != NULL )
	{
		CCCameraBase::CurrentCamera->currentPush++;
		CCCameraBase::CurrentCamera->pushedMatrix[CCCameraBase::CurrentCamera->currentPush] = CCCameraBase::CurrentCamera->pushedMatrix[CCCameraBase::CurrentCamera->currentPush-1];
        CCASSERT( CCCameraBase::CurrentCamera->currentPush < MAX_PUSHES );
	}
}

void GLPopMatrix()
{
	if( CCCameraBase::CurrentCamera != NULL )
	{
		CCCameraBase::CurrentCamera->currentPush--;
	}
}


void GLLoadIdentity()
{
	if( CCCameraBase::CurrentCamera != NULL )
	{
		CCMatrix identityMatrix;
		CCMatrixLoadIdentity( identityMatrix );
		CCCameraBase::CurrentCamera->pushedMatrix[CCCameraBase::CurrentCamera->currentPush] = identityMatrix;
	}
}


void GLMultMatrixf(CCMatrix &matrix)
{
    if( CCCameraBase::CurrentCamera != NULL )
	{
		CCMatrixMultiply( CCCameraBase::CurrentCamera->pushedMatrix[CCCameraBase::CurrentCamera->currentPush], matrix, CCCameraBase::CurrentCamera->pushedMatrix[CCCameraBase::CurrentCamera->currentPush] );
	}
}


void GLTranslatef(float tx, float ty, float tz)
{
    if( CCCameraBase::CurrentCamera != NULL )
	{
		CCMatrixTranslate( CCCameraBase::CurrentCamera->pushedMatrix[CCCameraBase::CurrentCamera->currentPush], tx, ty, tz );
	}
}


void GLScalef(float sx, float sy, float sz)
{
    if( CCCameraBase::CurrentCamera != NULL )
	{
		CCMatrixScale( CCCameraBase::CurrentCamera->pushedMatrix[CCCameraBase::CurrentCamera->currentPush], sx, sy, sz );
	}
}


void GLRotatef(float angle, float x, float y, float z)
{
    if( CCCameraBase::CurrentCamera != NULL )
	{
		CCMatrixRotateDegrees( CCCameraBase::CurrentCamera->pushedMatrix[CCCameraBase::CurrentCamera->currentPush], angle, x, y, z );
	}
}
