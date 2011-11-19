#pragma once

#include "ICollidable.h"
#include "Singleton.h"
#include "DoubleLinkedList.h"

#define MAX_COLLISION_PAIRS		50

typedef struct
{
	ICollidable* items[2];
} CollisionPair;

class CCollisionManager : public SingleThreadSingleton<CCollisionManager>
{
	friend class SingleThreadSingleton<CCollisionManager>;
private:
	CCollisionManager(void);
	~CCollisionManager(void);

public:
	void RegisterCollidable( ICollidable* obj );
	void UnregisterCollidable( ICollidable* obj );
	void GetCollisionPairs( CollisionPair* pairs_out, int* size_out );

private:
	CDoubleLinkedList<ICollidable> m_collidables;

private:
	bool DetermineCollision( ICollidable* c1, ICollidable* c2 );
};
