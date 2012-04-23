/********************************************************************
	created:	2012/04/23
	filename: 	CollisionManager.cpp
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "StdAfx.h"
#include "CollisionManager.h"

CCollisionManager::CCollisionManager(void)
{
}

CCollisionManager::~CCollisionManager(void)
{
}


void CCollisionManager::RegisterStaticCollidable( ICollidable* obj )
{
	m_staticCollidables.AddItemToEnd( obj );
}

void CCollisionManager::RegisterDynamicCollidable( ICollidable* obj )
{
	m_dynamicCollidables.AddItemToEnd( obj );
}

void CCollisionManager::UnregisterCollidable( ICollidable* obj )
{
	m_staticCollidables.RemoveItem( obj );
	m_dynamicCollidables.RemoveItem( obj );
}

void CCollisionManager::GetCollisionPairs( CollisionPair* pairs_out, int* size_out )
{
	assert(pairs_out);
	CDoubleLinkedList<ICollidable>::DoubleLinkedListItem *c1, *c2;
	int cnt = 0;

	/* Brute force method - Test each dynamic object against every other dynamic object */
	c1 = m_dynamicCollidables.first;
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
				assert( cnt < MAX_COLLISION_PAIRS );
			}

			c2 = c2->next;
		}

		c1 = c1->next;
	}
	
	/* Test every dynamic object against every static object */
	c1 = m_dynamicCollidables.first;
	while(c1 != NULL)
	{
		c2 = m_staticCollidables.first;
		while(c2 != NULL)
		{
			bool collided = DetermineCollision(c1->item, c2->item);
			if( collided == true )
			{
				pairs_out->items[0] = c1->item;
				pairs_out->items[1] = c2->item;

				pairs_out++;
				cnt++;
				assert( cnt < MAX_COLLISION_PAIRS );
			}

			c2 = c2->next;
		}

		c1 = c1->next;
	}

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

void CCollisionManager::Update()
{
	/* TODO: Update all the dynamic object's transform and collision in local storage */

	/* Find all the intersecting objects */
	CollisionPair pairs[MAX_COLLISION_PAIRS];
	int num;
	
	GetCollisionPairs( pairs, &num );

	/* Notify each object of the collision */
	for( int i=0; i < num; i++ )
	{
		pairs[i].items[0]->HandleCollision(pairs[i].items[1]);
		pairs[i].items[1]->HandleCollision(pairs[i].items[0]);
	}
	
}