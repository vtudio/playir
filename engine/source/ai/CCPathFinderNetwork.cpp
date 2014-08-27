/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : AIPathNodeNetwork.cpp
 *-----------------------------------------------------------
 */


#include "CCDefines.h"


CCPathFinderNetwork::CCPathFinderNetwork()
{
    connectingNodes = false;
}


CCPathFinderNetwork::~CCPathFinderNetwork()
{
	nodes.deleteObjectsAndList();
}


void CCPathFinderNetwork::view()
{
    if( nodes.length > 0 && connectingNodes == false )
    {
		gEngine->textureManager->setTextureIndex( 1 );

        const CCColour nodeColour = CCColour( 1.0f, 0.0f, 0.0f, 1.0f );
        const CCColour pathColour = CCColour( 1.0f, 1.0f, 1.0f, 1.0f );

        CCSetColour( nodeColour );

        for( int i=0; i<nodes.length; ++i )
        {
            const PathNode *node = nodes.list[i];
            GLPushMatrix();
            GLTranslatef( node->point.x, node->point.y, node->point.z );
            CCRenderCube( true );
            GLPopMatrix();
        }

        static CCVector3 start, end;
		{
            GLVertexPointer( 3, GL_FLOAT, sizeof( PathNode ), &nodes.list[0]->point, nodes.length );
#ifndef DXRENDERER
            gRenderer->GLDrawArrays( GL_POINTS, 0, nodes.length );
#endif

            for( int i=0; i<nodes.length; ++i )
            {
                const PathNode *node = nodes.list[i];
                const CCPtrList<PathNode::PathConnection> &connections = node->connections;
                for( int j=0; j<connections.length; ++j )
                {
                    const PathNode::PathConnection *connection = connections.list[j];
                    start.set( node->point.x, 2.0f, node->point.z );
                    end.set( connection->node->point.x, 2.0f, connection->node->point.z );
                    CCRenderLine( start, end );
                }
            }
        }

        if( pathingFrom != NULL )
        {
            CCRenderer::CCSetDepthRead( false );

            CCSetColour( pathColour );

            const PathNode *currentNode = pathingFrom;
            for( int i=0; i<path.endDirection; ++i )
            {
                const int connectionIndex = path.directions[i];
                const CCPtrList<PathNode::PathConnection> &connections = currentNode->connections;
                if( connectionIndex < connections.length )
                {
                    const PathNode::PathConnection *connection = connections.list[connectionIndex];
                    const PathNode *toNode = connection->node;
                    CCASSERT( toNode != NULL );
                    start.set( currentNode->point.x, 5.0f, currentNode->point.z );
                    end.set( toNode->point.x, 5.0f, toNode->point.z );
                    CCRenderLine( start, end );
                    currentNode = toNode;
                }
            }

            CCRenderer::CCSetDepthRead( true );
        }
    }
}


void CCPathFinderNetwork::addNode(const CCVector3 point, CCCollideable *parent)
{
	PathNode *node = new PathNode();
	node->point = point;
    node->parent = parent;
	nodes.add( node );
}


void CCPathFinderNetwork::addCollideable(CCCollideable *collideable, const CCVector3 &extents)
{
	const float maxIncrement = 150.0f;
	const float minIncrement = 50.0f;

	const float startX = collideable->aabbMin.x - minIncrement;
	const float endX = collideable->aabbMax.x + minIncrement;
	const float width = endX - startX;
	float numberOfIncrements = roundf( ( width / maxIncrement ) + 0.5f );
	const float spacingX = width / numberOfIncrements;

	const float startZ = collideable->aabbMin.z - minIncrement;
	const float endZ = collideable->aabbMax.z + minIncrement;
	const float depth = endZ - startZ;
	numberOfIncrements = roundf( ( depth / maxIncrement ) + 0.5f  );
	const float spacingZ = depth / numberOfIncrements;

	// Front and back rows
	for( float x=startX; x<endX+1.0f; x+=spacingX )
	{
        if( x > -extents.x && x < extents.x )
        {
            for( float z=startZ; z<endZ+1.0f; z+=depth )
            {
                if( z > -extents.z && z < extents.z )
                {
                    addNode( CCVector3( x, 0.0f, z ), collideable );
                }
            }
		}
	}

	// Left and right rows
	for( float z=startZ+spacingZ; z<endZ+1.0f-spacingZ; z+=depth-spacingZ )
	{
        if( z > -extents.z && z < extents.z )
        {
            for( float x=startX; x<endX+1.0f; x+=spacingX )
            {
                if( x > -extents.x && x < extents.x )
                {
                    addNode( CCVector3( x, 0.0f, z ), collideable );
                }
            }
		}
	}
}


void CCPathFinderNetwork::removeCollideable(CCCollideable *collideable)
{
    pathingFrom = NULL;

	for( int i=0; i<nodes.length; ++i )
	{
		PathNode *node = nodes.list[i];
		if( node->parent == collideable )
		{
			nodes.remove( node );
            delete node;
			--i;
		}
	}
}


void CCPathFinderNetwork::addFillerNodes(CCCollideable *collideable)
{
	CCUpdateCollisions( collideable );

	const float maxIncrement = 50.0f;
	const float minIncrement = 5.0f;

	const float startX = collideable->aabbMin.x + minIncrement;
	const float endX = collideable->aabbMax.x - minIncrement;
	const float width = endX - startX;
	float numberOfIncrements = roundf( ( width / maxIncrement ) + 0.5f  );
    if( numberOfIncrements < 1.0f )
    {
        numberOfIncrements = 1.0f;
    }
	const float spacingX = width / numberOfIncrements;

	const float startZ = collideable->aabbMin.z + minIncrement;
	const float endZ = collideable->aabbMax.z - minIncrement;
	const float depth = endZ - startZ;
	numberOfIncrements = roundf( ( depth / maxIncrement ) + 0.5f  );
    if( numberOfIncrements < 1.0f )
    {
        numberOfIncrements = 1.0f;
    }
	const float spacingZ = depth / numberOfIncrements;

	for( float x=startX; x<endX+1.0f; x+=spacingX )
	{
		for( float z=startZ; z<endZ+1.0f; z+=spacingZ )
		{
			addNode( CCVector3( x, 0.0f, z ) );
		}
	}
}


void CCPathFinderNetwork::linkDistantNodes()
{
	if( nodes.length > 0 )
	{
		const float BIG_FLOAT = 10000.0;
		const PathNode *topLeft = findClosestNode( CCVector3( -BIG_FLOAT, 0.0, -BIG_FLOAT ) );
		const PathNode *topRight = findClosestNode( CCVector3( BIG_FLOAT, 0.0, -BIG_FLOAT ) );
		const PathNode *bottomLeft = findClosestNode( CCVector3( -BIG_FLOAT, 0.0, BIG_FLOAT ) );
		const PathNode *bottomRight = findClosestNode( CCVector3( BIG_FLOAT, 0.0, BIG_FLOAT ) );

        if( topLeft != NULL && topRight != NULL && bottomLeft != NULL && bottomRight != NULL )
        {
            const float startX = topLeft->point.x < bottomLeft->point.x ? topLeft->point.x : bottomLeft->point.x;
            const float endX = topRight->point.x > bottomRight->point.x ? topRight->point.x : bottomRight->point.x;
            const float startZ = topLeft->point.z < topRight->point.z ? topLeft->point.z : topRight->point.z;
            const float endZ = bottomLeft->point.z > bottomRight->point.z ? bottomLeft->point.z : bottomRight->point.z;

            float x = startX;
            float z = startZ;
            float increment = 150.0;
            while( x < endX && z < endZ )
            {
                x += increment;
                if( x >= endX )
                {
                    x = startX + increment;
                    z += increment;
                }

                if( z < endZ )
                {
                    addNode( CCVector3( x, 0.0, z ) );
                }
            }
        }
	}
}


void CCPathFinderNetwork::removeFillerNodes()
{
    pathingFrom = NULL;

    for( int i=0; i<nodes.length; ++i )
    {
        PathNode *node = nodes.list[i];
        if( node->parent == NULL )
        {
            nodes.remove( node );
            delete node;
            --i;
        }
    }
}


void CCPathFinderNetwork::clear()
{
	nodes.deleteObjects();
    pathingFrom = NULL;
}


void CCPathFinderNetwork::connect()
{
    connectingNodes = true;

	removeFillerNodes();
	linkDistantNodes();

    // Reset our connections
	for( int i=0; i<nodes.length; ++i )
	{
		PathNode *node = nodes.list[i];
        CCPtrList<PathNode::PathConnection> &connections = node->connections;
        connections.deleteObjects();
	}

	const float maxNodeDistance = CC_SQUARE( 200.0f );
	for( int i=0; i<nodes.length; ++i )
	{
		PathNode *currentNode = nodes.list[i];
        CCPtrList<PathNode::PathConnection> &connections = currentNode->connections;

		for( int j=0; j<nodes.length; ++j )
		{
			const PathNode *targetNode = nodes.list[j];
			if( currentNode != targetNode )
			{
				const float distance = CCVector3Distance2D( currentNode->point, targetNode->point );
				if( distance < maxNodeDistance )
				{
                    const float angle = CCAngleTowards( currentNode->point, targetNode->point );

                    // Check to see if we already have this angle
                    //if( false )
                    {
                        int angleFoundIndex = -1;
                        for( int k=0; k<connections.length; ++k )
                        {
                            if( angle == connections.list[k]->angle )
                            {
                                angleFoundIndex = k;
                                break;
                            }
                        }

                        // We already have an angle connected closer
                        if( angleFoundIndex != -1 )
                        {
                            if( distance >= connections.list[angleFoundIndex]->distance )
                            {
                                continue;
                            }
                            else
                            {
                                PathNode::PathConnection *connection = connections.list[angleFoundIndex];
                                connections.remove( connection );
                                delete connection;
                            }
                        }
                    }

                    // Insert our node
                    PathNode::PathConnection *newConnection = new PathNode::PathConnection();
                    newConnection->distance = distance;
                    newConnection->angle = angle;
                    newConnection->node = targetNode;
                    connections.add( newConnection );
				}
			}
		}
	}

    connectingNodes = false;
}


uint CCPathFinderNetwork::findClosestNodes(const CCVector3 &position, const float &radius, const CCVector3 **vectors, const uint &length)
{
	uint found = 0;

	for( int i=0; i<nodes.length && found<length; ++i )
	{
		const PathNode *node = nodes.list[i];
		if( node->connections.length > 0 )
		{
			float distance = CCVector3Distance2D( position, node->point );
			if( distance < radius )
			{
				vectors[found++] = &node->point;
			}
		}
	}

	return found;
}


const CCPathFinderNetwork::PathNode* CCPathFinderNetwork::findClosestNodeToPathTarget(CCCollideable *objectToPath, const CCVector3 &position, const bool withConnections)
{
	int closestNode = -1;
	float closestDistance = MAXFLOAT;

	for( int i=0; i<nodes.length; ++i )
	{
		PathNode *node = nodes.list[i];
		if( withConnections == false || node->connections.length > 0 )
		{
			const float distance = CCVector3Distance2D( node->point, position );
			if( distance < closestDistance )
			{
                CCCollideable *hitObject = CCOctreeMovementCollisionCheck( objectToPath, position, node->point );
                if( hitObject != NULL )
                {
                    continue;
                }
                if( hitObject == NULL )
                {
                    closestDistance = distance;
                    closestNode = i;
                }
			}
		}
	}

	if( closestNode != -1 )
	{
		return nodes.list[closestNode];
	}

	return NULL;
}


const CCPathFinderNetwork::PathNode* CCPathFinderNetwork::findClosestNode(const CCVector3 &position)
{
    int closestNode = -1;
	float closestDistance = MAXFLOAT;

	for( int i=0; i<nodes.length; ++i )
	{
		PathNode *node = nodes.list[i];
        const float distance = CCVector3Distance2D( node->point, position );
        if( distance < closestDistance )
        {
            closestDistance = distance;
            closestNode = i;
        }
	}

	if( closestNode != -1 )
	{
		return nodes.list[closestNode];
	}

	return NULL;
}

static const CCPathFinderNetwork::PathNode *pathFromNode;
static const CCPathFinderNetwork::PathNode *pathTargetNode;
static int compare(const void *a, const void *b)
{
	const CCPathFinderNetwork::PathNode::PathConnection &pathA = *pathFromNode->connections.list[ *(int*)a ];
	const CCPathFinderNetwork::PathNode::PathConnection &pathB = *pathFromNode->connections.list[ *(int*)b ];

	const float pathADistance = CCVector3Distance2D( pathA.node->point, pathTargetNode->point );
	const float pathBDistance = CCVector3Distance2D( pathB.node->point, pathTargetNode->point );

	return (int)( pathADistance - pathBDistance );
}


bool CCPathFinderNetwork::findPath(CCCollideable *objectToPath, Path &pathResult, const PathNode *fromNode, const PathNode *toNode)
{
    if( fromNode != NULL && toNode != NULL )
    {
        NodesList<const PathNode, 50> previousNodes;
        previousNodes.add( fromNode );

        pathTargetNode = toNode;
        path.distance = 0.0f;
        pathingFrom = fromNode;
        if( followPath( objectToPath, path, 0, 0.0f, previousNodes, fromNode, toNode ) )
        {
            pathResult = path;
            return true;
        }
    }
    return false;
}


bool CCPathFinderNetwork::followPath(CCCollideable *objectToPath,
                                     Path &path, const int currentDirection,
                                     const float currentDistance,
                                     NodesList<const PathNode, 50> &previousNodes,
                                     const PathNode *fromNode, const PathNode *toNode)
{
	// Node found
	if( fromNode == toNode )
	{
		path.endDirection = currentDirection;
		path.distance = currentDistance;
		return true;
	}

	const int nextDirection = currentDirection+1;
	if( nextDirection >= 50 )
	{
		// Give up
		return false;
	}

	pathFromNode = fromNode;
    const CCPtrList<PathNode::PathConnection> &connections = fromNode->connections;
	int *values = new int[connections.length];
	for( int i=0; i<connections.length; ++i )
	{
		values[i] = i;
	}
	qsort( values, connections.length, sizeof( int ), compare );

	for( int i=0; i<connections.length; ++i )
	{
		const PathNode::PathConnection *nextConnection = connections.list[values[i]];

        const PathNode *targetNode = nextConnection->node;

		// Previously followed?
		if( previousNodes.find( targetNode ) != -1 )
		{
			continue;
		}


        CCCollideable *collidedWith = CCOctreeMovementCollisionCheck( objectToPath, fromNode->point, targetNode->point );
        if( collidedWith != NULL )
        {
            continue;
        }

		const float pathDistance = currentDistance + nextConnection->distance;
		if( previousNodes.add( targetNode ) == false )
		{
			delete[] values;
			return false;
		}

		if( followPath( objectToPath, path, nextDirection, pathDistance, previousNodes, targetNode, toNode ) )
		{
			path.directions[currentDirection] = values[i];
			delete[] values;
			return true;
		}
	}

	delete[] values;
	return false;
}
