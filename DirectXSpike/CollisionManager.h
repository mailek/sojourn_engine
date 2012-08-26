#pragma once
/********************************************************************
	created:	2012/04/23
	filename: 	CollisionManager.h
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "ICollidable.h"
#include "Singleton.h"
#include "DoubleLinkedList.h"

#define MAX_COLLISION_PAIRS		500

typedef struct _CollisionPair
{
	ICollidable* items[2];
} CollisionPair;

class CCollisionManager : public SingleThreadSingleton<CCollisionManager, CCollisionManager>
{
	friend class SingleThreadSingleton<CCollisionManager, CCollisionManager>;
private:
	CCollisionManager(void);
	~CCollisionManager(void);

public:
	void RegisterDynamicCollidable( ICollidable* obj );
	void RegisterStaticCollidable( ICollidable* obj );
	void UnregisterCollidable( ICollidable* obj );
	void GetCollisionPairs( CollisionPair* pairs_out, int* size_out );
	void Update();

private:
	CDoubleLinkedList<ICollidable> m_dynamicCollidables;
	CDoubleLinkedList<ICollidable> m_staticCollidables;

private:
	bool DetermineCollision( ICollidable* c1, ICollidable* c2 );
};
