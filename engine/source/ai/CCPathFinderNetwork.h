/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCPathFinderNetwork
 * Description : Network of nodes used for path finding
 *
 * Created     : 03/05/10
 *-----------------------------------------------------------
 */

#ifndef __CCPATHFINDERNETWORK_H__
#define __CCPATHFINDERNETWORK_H__


class CCPathFinderNetwork
{
public:
	CCPathFinderNetwork();
	~CCPathFinderNetwork();

	void view();

	void addNode(const CCVector3 point, CCCollideable *parent=NULL);
	void addCollideable(CCCollideable *collideable, const CCVector3 &extents);
	void linkDistantNodes();
	void removeCollideable(CCCollideable *collideable);
	void addFillerNodes(CCCollideable *collideable);
    void removeFillerNodes();

	void clear();

	// Connect our nodes
	void connect();

    uint findClosestNodes(const CCVector3 &position, const float &radius, const CCVector3 **vectors, const uint &length);

	struct PathNode
	{
		PathNode()
        {
            parent = NULL;
        }

		CCVector3 point;

		struct PathConnection
		{
			float distance;
			float angle;
			const PathNode *node;
		};

        CCCollideable *parent;
		CCPtrList<PathConnection> connections;
	};
	const PathNode* findClosestNodeToPathTarget(CCCollideable *objectToPath, const CCVector3 &position, const bool withConnections);
	const PathNode* findClosestNode(const CCVector3 &position);

	struct Path
	{
		Path()
		{
			endDirection = 0;
			distance = 0.0f;
		}

		int directions[50];
		int endDirection;
		float distance;
	};
	bool findPath(CCCollideable *objectToPath, Path &pathResult, const PathNode *fromNode, const PathNode *toNode);

protected:
	template <typename T, int TLENGTH> struct NodesList : public CCPtrList<T>
	{
		NodesList()
		{
			this->allocate( TLENGTH );
		}

        bool add(T *node)
		{
			CCPtrList<T>::add( node );
			if( this->length < this->allocated )
			{
				return true;
			}
			return false;
		}
	};

	// Used for path finding
	Path path;
	const PathNode *pathingFrom;
    bool followPath(CCCollideable *objectToPath,
                    Path &path, const int currentDirection,
                    const float currentDistance,
                    NodesList<const PathNode, 50> &previousNode,
                    const PathNode *fromNode, const PathNode *toNode);

	NodesList<PathNode, 500> nodes;
    bool connectingNodes;
};


#endif // __CCPATHFINDERNETWORK_H__
