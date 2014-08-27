/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCCollisionTools.h
 * Description : Functionality for testing collision.
 *
 * Created     : 01/03/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCCOLLISIONTOOLS_H__
#define __CCCOLLISIONTOOLS_H__


class CCCollideable;
typedef struct CCOctree CCOctree;
#include "CCPathFinderNetwork.h"

struct CCCollisionManager
{
	CCCollisionManager(const float octreeSize);
	~CCCollisionManager();

	CCOctree *octree;
	float pruneOctreeTimer;

    CCPtrList<CCCollideable> collideables;

	void addCollideable(CCCollideable* collideable);
	void removeCollideable(CCCollideable* collideable);
};



enum CCCollisionFlags
{
    collision_none          = 0x000000000,  // No collision
    collision_box           = 0x000000001,  // Anything with a collision box
    collision_static        = 0x000000002,  // Non-moveable objects
    collision_moveable      = 0x000000004,  // Moveable objects
    collision_character     = 0x000000008,  // Characters
    collision_ui            = 0x000000010   // UI
};



// Update the bounding boxes when an object is flagged
extern void CCUpdateCollisions(CCCollideable *collideable, const bool dependantOnFlags=true);

// Collision test bounding boxes
extern bool CCBasicBoxCollisionCheck(const CCVector3 &sourceMin, const CCVector3 &sourceMax,
                                     const CCVector3 &targetMin, const CCVector3 &targetMax);

extern CCCollideable* CCBasicCollisionCheck(CCCollideable *sourceObject, const CCVector3 &targetLocation);

// Is there anything in this location?
extern CCCollideable* CCOctreeCollideableScan(const CCVector3 &min, const CCVector3 &max,
                                              const CCCollideable *sourceObject=NULL,
                                              const uint flags=collision_box);

extern CCCollideable* CCOctreeCollisionCheck(CCCollideable *sourceObject,
                                             const CCVector3 &targetLocation,
                                             const bool requestCollisions=false,
                                             const CCCollisionFlags flags=collision_box);

extern CCCollideable* CCOctreeMovementCollisionCheck(CCCollideable *sourceObject, CCVector3 currentPosition, const CCVector3 &targetPosition);


// Just returns a list of collideables without checking for flags or if they should collide
extern void CCOctreeCollisionCheckAsync(CCCollideable *sourceObject, const CCVector3 &targetLocation, CCPtrList<CCCollideable> &collisions);
extern void CCOctreeMovementCollisionCheckAsync(CCCollideable *sourceObject, CCVector3 currentPosition, const CCVector3 &targetPosition, CCPtrList<CCCollideable> &collisions);

// Collision test a ray
extern CCCollideable* CCBasicLineCollisionCheck(CCCollideable **list,
                                                const int length,
                                                CCCollideable *sourceObject,
                                                const CCVector3 &start,
                                                const CCVector3 &end,
                                                CCVector3 *hitPosition,
                                                const bool collideInsideObjects,
                                                const CCCollisionFlags flags=collision_box,
                                                const bool stopAtAnyCollision = false);

extern CCCollideable* CCBasicLineCollisionCheck(const CCVector3 &start,
                                                const CCVector3 &end,
                                                const float width,
                                                CCCollideable *source=NULL);

extern bool CCBasicLineCollisionCheck(CCCollideable *checkingObject,
                                      CCCollideable *sourceObject,
                                      const CCVector3 &start,
                                      const CCVector3 &end);

// Frustum testing
extern uint CCSphereInFrustum(float frustum[6][4], const float x, const float y, const float z, const float radius);
extern uint CCCubeInFrustum(const float frustum[6][4], const CCVector3 &min, const CCVector3 &max);


#endif // __CCCOLLISIONTOOLS_H__
