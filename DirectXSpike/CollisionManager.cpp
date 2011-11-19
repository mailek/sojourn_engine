#include "StdAfx.h"
#include "CollisionManager.h"

CCollisionManager::CCollisionManager(void)
{
}

CCollisionManager::~CCollisionManager(void)
{
}


void CCollisionManager::RegisterCollidable( ICollidable* obj )
{
	m_collidables.AddItemToEnd( obj );
}

void CCollisionManager::UnregisterCollidable( ICollidable* obj )
{
	m_collidables.RemoveItem( obj );
}

void CCollisionManager::GetCollisionPairs( CollisionPair* pairs_out, int* size_out )
{
	/* Brute force method - Test each object against every other object */
	assert(pairs_out);
	int cnt = 0;

	CDoubleLinkedList<ICollidable>::DoubleLinkedListItem *c1, *c2;
	c1 = m_collidables.first;
	while(c1 != NULL)
	{
		c2 = c1->next;
		while(c2 != NULL)
		{
			bool collided = DetermineCollision(c1->item, c2->item);
			if( collided == true )
			{
				pairs_out->items[0] = c1->item;
				pairs_out->items[1] = c2->item;

				pairs_out++;
				cnt++;
			}

			c2 = c2->next;
		}

		c1 = c1->next;
	}

	assert( cnt < MAX_COLLISION_PAIRS );
	*size_out = cnt;
}

bool CCollisionManager::DetermineCollision( ICollidable* c1, ICollidable* c2 )
{
	/* Sphere to Sphere Collision Test */

	Sphere_PosRad s1, s2;

	c1->GetCollideSphere(s1);
	c2->GetCollideSphere(s2);

	return Collide_SphereToSphere(s1, s2);
}