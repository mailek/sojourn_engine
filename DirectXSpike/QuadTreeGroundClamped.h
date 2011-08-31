#pragma once
//////////////////////////////////////////////////////////////////////////
// QuadTreeGroundClamped.h - 2011 Matthew Alford
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////
// Includes
//////////////////////////////////////
#include "MathUtil.h"

//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////

//////////////////////////////////////
// Types
//////////////////////////////////////
typedef long NODE_INDEX_TYPE;

//////////////////////////////////////
// Constants
//////////////////////////////////////

static const long		MAXQUADS		= 87381;

//////////////////////////////////////
// Class Definition
//////////////////////////////////////
template <typename T, typename U>
class QuadTree_GroundClamped
{
public:
	struct QuadTreeNode {
		D3DXMATRIX					worldTransform;
		Sphere_PosRad				sphereBounds; 
		ABB_MaxMin					abb;
		NODE_INDEX_TYPE				index;
		NODE_INDEX_TYPE				parentIndex;
		std::vector<T*>            *objects;
	};

	struct GCQTDefinition {
		int			quadTreeDepth;
		float		branchHeight;
		float		leafHeight;
	};

	QuadTree_GroundClamped(void);
	virtual ~QuadTree_GroundClamped(void);

	inline long GetQuadTreeSize(void);
	bool Setup( ABB_MaxMin bounds, U& heightMap, GCQTDefinition def );

	int FindLeafQuadByPoint(D3DXVECTOR3 point);
	D3DXMATRIX GetWorldTransformByQuadIndex( NODE_INDEX_TYPE quadIndex );
	bool QuadHasChildren(NODE_INDEX_TYPE index);
	int FrustumRecurseCollideQuadChildren( NODE_INDEX_TYPE index, const Frustum_Camera &camera, NODE_INDEX_TYPE *leafIndices );
	
protected:
	QuadTreeNode					m_quadTreeArray[MAXQUADS];
	GCQTDefinition					m_def;

	int FindLeafListQuadsByBoundSphere(const Sphere_PosRad &sphere, NODE_INDEX_TYPE *leafIndices);
	int GetAllOccupiedDescendents( NODE_INDEX_TYPE parent, NODE_INDEX_TYPE *leafIndices );

private:
	void BuildNextQuadTreeDepth(const QuadTreeNode *parent, long &quadCount, std::vector<QuadTreeNode*> &newParents, U& heightMap, bool leafDepth);
};

//////////////////////////////////////////////////////////////////////////
// Setup Functions
//////////////////////////////////////////////////////////////////////////
template <typename T, typename U>
QuadTree_GroundClamped<T,U>::QuadTree_GroundClamped(void)
{
	::ZeroMemory(&m_quadTreeArray, sizeof(m_quadTreeArray));
	::ZeroMemory(&m_def, sizeof(m_def));
}

template <typename T, typename U>
QuadTree_GroundClamped<T,U>::~QuadTree_GroundClamped(void)
{
	long quadSize = GetQuadTreeSize();
	for(long i = 0; i < quadSize; i++)
	{
		PTR_SAFEDELETE(m_quadTreeArray[i].objects);
	}
}

/* Create a new quad tree with the given ABB bounds.  Area within this 
   bounds will be quartered N times, where N = QUADTREEDEPTH */
template <typename T, typename U>
bool QuadTree_GroundClamped<T,U>::Setup( ABB_MaxMin bounds, U& heightMap, GCQTDefinition def )
{
	m_def = def;

	// build quad tree
	QuadTreeNode* root = &m_quadTreeArray[0];
	root->abb = bounds;
	root->worldTransform = ABB_CalcUnitTransform(root->abb);
	root->sphereBounds = ABB_CalcMinSphereContaining(root->abb);
		
	root->index = 0;
	root->parentIndex = 0; 
	
	std::vector<QuadTreeNode*> nodesToBuild, builtNodes;
	builtNodes.push_back(root);
	long quadCount(1);
		
	for(long i = 0; i < m_def.quadTreeDepth; i++)
	{
		for(std::vector<QuadTreeNode*>::iterator it = builtNodes.begin(), _it = builtNodes.end(); it != _it; it++)
			BuildNextQuadTreeDepth((*it), quadCount, nodesToBuild, heightMap, i == m_def.quadTreeDepth-1); // index to next item
		
		builtNodes = nodesToBuild;
		nodesToBuild.clear();
	}

	return true;
}

/* Creates a new quad tree node, and if at max tree depth, then make this node a leaf */
template <typename T, typename U>
void QuadTree_GroundClamped<T,U>::BuildNextQuadTreeDepth(const QuadTreeNode *parent, long &quadCount, std::vector<QuadTreeNode*> &newParents, U& heightMap, bool leafDepth)
{
	const float parentWidth = ABB_CalcWidth(parent->abb);
	const float parentDepth = ABB_CalcDepth(parent->abb);
	const float abbHeight = (leafDepth) ? m_def.leafHeight : m_def.branchHeight;
	const float abbYSink = abbHeight / 2.0f;
	
	ABB_MaxMin scaledParent = ABB_Scale( parent->abb, 0.5f, 1.0f, 0.5f );
	D3DXMATRIX rotation;
	D3DXMatrixIdentity( &rotation );
	D3DXVECTOR3 ulCenterOffset(-parentWidth/4.0f, 0, parentDepth/4.0f ); // center of upper-left quad
	
	// UL ->UR ->LR ->LL
	for(int i = 0; i < 4; i++)
	{
		QuadTreeNode* newSubNode = &m_quadTreeArray[quadCount];
		newSubNode->index = quadCount;
		newSubNode->parentIndex = parent->index;
		newSubNode->abb = scaledParent;
		
		D3DXVECTOR3 offset;
		D3DXVec3TransformCoord( &offset, &ulCenterOffset, &rotation );
		newSubNode->abb.max = scaledParent.max + offset;
		newSubNode->abb.min = scaledParent.min + offset;
		
		/* find quad's lowest point colliding with terrain */
		float yMin = heightMap.GetTerrainHeightAtXZ(newSubNode->abb.min.x, newSubNode->abb.min.z);
		yMin = min(yMin, heightMap.GetTerrainHeightAtXZ(newSubNode->abb.min.x + parentWidth/2.0f, newSubNode->abb.min.z));
		yMin = min(yMin, heightMap.GetTerrainHeightAtXZ(newSubNode->abb.min.x, newSubNode->abb.min.z + parentDepth/2.0f));
		yMin = min(yMin, heightMap.GetTerrainHeightAtXZ(newSubNode->abb.min.x + parentWidth/2.0f, newSubNode->abb.min.z + parentDepth/2.0f));

		/* size the Y axis */
		newSubNode->abb.min.y = yMin - abbYSink;
		newSubNode->abb.max.y = newSubNode->abb.min.y + abbHeight;

		newSubNode->worldTransform = ABB_CalcUnitTransform(newSubNode->abb);
		newSubNode->sphereBounds = ABB_CalcMinSphereContaining(newSubNode->abb);
		
		assert(MAXQUADS >= newSubNode->index);
		newParents.push_back(&m_quadTreeArray[newSubNode->index]);

		quadCount++;
		D3DXMATRIX deltaRot;
		D3DXMatrixRotationY( &deltaRot, HALF_PI );
		rotation *= deltaRot;
	}
}

//////////////////////////////////////////////////////////////////////////
// Update Functions
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Query Functions
//////////////////////////////////////////////////////////////////////////

/* Calculates whether given quad index has children */
template <typename T, typename U>
bool QuadTree_GroundClamped<T,U>::QuadHasChildren( NODE_INDEX_TYPE index )
{
	long i = (4*index)+1;

	return !(i > GetQuadTreeSize()-1 || i >= MAXQUADS);
}

/* Gets the world transform for the center of the requested quad */
template <typename T, typename U>
D3DXMATRIX QuadTree_GroundClamped<T,U>::GetWorldTransformByQuadIndex( NODE_INDEX_TYPE quadIndex )
{
	const NODE_INDEX_TYPE INVALID_INDEX(-1);
	assert( quadIndex < MAXQUADS && quadIndex != INVALID_INDEX );
	return m_quadTreeArray[quadIndex].worldTransform;
}

/* Finds the index of the leaf containing the requested point */
template <typename T, typename U>
int QuadTree_GroundClamped<T,U>::FindLeafQuadByPoint(D3DXVECTOR3 point)
{
	NODE_INDEX_TYPE index(0);
	bool indexChanged(false);
	do
	{
		indexChanged = false;
		for(int subIndex = 1; subIndex < 5; subIndex++)
		{
			NODE_INDEX_TYPE newIndex = 4*index+subIndex;
			if(Collide_PointToBox(point, m_quadTreeArray[newIndex].abb))
			{
				index = newIndex;
				indexChanged = true;
				break;
			}
		} 
	} while(QuadHasChildren(index) && indexChanged);

	return index;
}

/* Finds all the leaf nodes intersecting or contained with the requested sphere (3d pos, radius)
   Returns: num of leaf indices in array */
template <typename T, typename U>
int QuadTree_GroundClamped<T,U>::FindLeafListQuadsByBoundSphere(const Sphere_PosRad &sphere, NODE_INDEX_TYPE *leafIndices)
{
	int retLeafCnt(0);
	const int MAXSEARCHITEMS(20);
	const int INVALIDINDEX(-1);
	static int parentsBucket[2][MAXSEARCHITEMS];
	int newSearch(0), oldSearch(1);
	int newCount(0);
	
	memset( parentsBucket[oldSearch], INVALIDINDEX, sizeof(parentsBucket[oldSearch]) );
	parentsBucket[oldSearch][0] = 0; // seed recursion with root node

	do // breadth first recursion
	{
		memset( parentsBucket[newSearch], INVALIDINDEX, sizeof(parentsBucket[newSearch]) );

		newCount = 0;
		for( long i = 0; parentsBucket[oldSearch][i] != INVALIDINDEX; i++ )
		{
			for(int subIndex = 1; subIndex <= 4; subIndex++)
			{
				NODE_INDEX_TYPE index = 4*(parentsBucket[oldSearch][i])+subIndex;
				if(Collide_SphereToBox(sphere, m_quadTreeArray[index].abb))
				{
					if(QuadHasChildren(index))
					{
						parentsBucket[newSearch][newCount] = index;
						newCount++;
						assert( newCount < MAXSEARCHITEMS );
					}
					else
					{
						*leafIndices = index;
						leafIndices++;
						retLeafCnt++;
					}
				}
			}
		}

		// flip the search buffers
		newSearch = oldSearch;
		oldSearch = ++oldSearch%2;

	} while( newCount > 0 );

	return retLeafCnt;
}

/* Gets the total number of quad nodes used in the tree */
template <typename T, typename U>
inline long QuadTree_GroundClamped<T,U>::GetQuadTreeSize(void)
{
	static long nQuadTreeSize = 0;
	if(nQuadTreeSize == 0)
	{
		// calculate quad tree size
		float total(0.0f);
		for(int i=0; i<=m_def.quadTreeDepth; i++)
			total += pow(4.0f, i);

		nQuadTreeSize = (int)floor(total);
	}

	return nQuadTreeSize;
}

/* Recursively finds a list of all sub-children which contain objects */
template <typename T, typename U>
int QuadTree_GroundClamped<T,U>::GetAllOccupiedDescendents( NODE_INDEX_TYPE parent, NODE_INDEX_TYPE *leafIndices )
{
	int retCount(0);

	if(!QuadHasChildren(parent))
	{
		*leafIndices = parent;
		retCount++;
	} else
	{
		long foundCnt(0);
		for(long i = 4*parent+1, j = 4*parent+4; i <= j; i++)
		{
			foundCnt = GetAllOccupiedDescendents(i, leafIndices);
			retCount += foundCnt;
			leafIndices += foundCnt;	
		}
	}

	return retCount;
}

/* Recursively finds a list of all nodes which are in the camera frustum and contain objects */
template <typename T, typename U>
int QuadTree_GroundClamped<T,U>::FrustumRecurseCollideQuadChildren( NODE_INDEX_TYPE index, const Frustum_Camera &camera, NODE_INDEX_TYPE *leafIndices )
{
	int retCount(0);
	static int nRecursionLevel = 0;
			
	if(!QuadHasChildren(index) && m_quadTreeArray[index].objects != NULL )
	{
		*leafIndices = index;
		return(1);
	}

	nRecursionLevel++;
	
	// if children then collide the frustum to each child's bound box
	long foundCnt(0);
	for(NODE_INDEX_TYPE i = 4*index+1, j = 4*index+4; i <= j; i++, foundCnt=0)
	{
		QuadTreeNode *node = &m_quadTreeArray[i];

		IntersectType result = COLLIDE_INVALID;
		if(nRecursionLevel < DEPTHOFSPHERETEST)
		{
			// if at the top of the quad tree, then use sphere collision for faster results
			result = Collide_SphereToFrustum(node->sphereBounds, camera, OptimizeCollisionFor2D);
		} 
		else {
			// Test by ABB (more accurate for smaller quads)
			result = Collide_ABBToFrustum(m_quadTreeArray[i].abb, camera, OptimizeCollisionFor2D);
		}	

		if(result == COLLIDE_INTERSECT)
		{
			foundCnt = FrustumRecurseCollideQuadChildren(i, camera, leafIndices );// if child is partially outside then add to a list of further collision checks and go to next child
		}
		else if(result == COLLIDE_CONTAINED)
		{
			foundCnt = GetAllOccupiedDescendents(i, leafIndices);	
		}
		//else if(result == COLLIDE_OUTSIDE)
			//continue; // if child is totally outside then go to next child

		retCount += foundCnt;
		leafIndices += foundCnt;
		
	}
	
	nRecursionLevel--;

	return retCount;
}