/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCOctree.h
 * Description : Octree container used for collisions and rendering
 *
 * Created     : 01/03/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCOCTREE_H__
#define __CCOCTREE_H__


#define MAX_TREE_OBJECTS 64

#ifdef DEBUGON
extern int maxOctreesPerObject;
extern int maxLeafsPerScan;
extern int maxCollideablesPerScan;
#endif

enum CCOctreeLeafs
{
	leaf_bottom_front_left,
	leaf_bottom_front_right,
	leaf_bottom_back_left,
	leaf_bottom_back_right,

	leaf_top_front_left,
	leaf_top_front_right,
	leaf_top_back_left,
	leaf_top_back_right,
};

struct CCOctree
{
	CCOctree *parent;
	CCOctree **leafs;
    CCPtrList<CCCollideable> objects;

	float hSize;
	CCVector3 min, max;

	CCOctree(CCOctree *inParent, const CCVector3 position, const float size);
};

// Delete the octree and all it's leafs
extern void CCOctreeDeleteLeafs(CCOctree *tree);

// Add an object into the octree and create leafs if necessary
extern void CCOctreeAddObject(CCOctree *tree, CCCollideable *collideable);

// Remove an object from an octree and delete the octree if necessary
extern void CCOctreeRemoveObject(CCOctree *tree, CCCollideable *collideable);
extern void CCOctreeRemoveObject(CCCollideable *collideable);

// Remove and add octree
extern void CCOctreeRefreshObject(CCCollideable *collideable);

// Remove unused leafs
extern void CCOctreePruneTree(CCOctree *tree);

// See if the range is in the tree
extern bool CCOctreeIsInLeaf(const CCOctree *leaf, const CCVector3 &targetMin, const CCVector3 &targetMax);

// Traverse the tree
extern bool CCOctreeHasObjects(CCOctree *tree);

// Traverse the tree and find the leafs nodes, from the bottom up so we don't end up with all the leafs
extern void CCOctreeListLeafs(const CCOctree *tree, const CCVector3 &targetMin, const CCVector3 &targetMax, const CCOctree **leafsList, int *numberOfLeafs);

// List all the collideables in the tree
extern void CCOctreeListCollideables(CCCollideable **collideables, int *numberOfCollideables, const CCOctree **leafs, const int numberOfLeafs);
extern void CCOctreeListVisibles(CCPtrList<CCOctree> &leafs, CCPtrList<CCCollideable> &collideables);

// Render the octrees
extern void CCOctreeRender(CCOctree *tree);

// Render the objects in the trees
extern CCCollideable* CCOctreeGetVisibleCollideables(const int i);
extern void CCOctreeScanVisibleCollideables(const float frustum[6][4],
                                            CCPtrList<CCCollideable> &visibleCollideables);
extern void CCScanVisibleCollideables(const float frustum[6][4],
                                      const CCPtrList<CCCollideable> &collideables,
                                      CCPtrList<CCCollideable> &visibleCollideables);
extern void CCRenderVisibleObjects(CCCameraBase *camera, const CCRenderPass pass, const bool alpha);


#endif // __CCOCTREE_H__
