#pragma once

#include "mathutil.h"

typedef enum
{
	OBSTACLE,
	CHARACTER
} ECollidableType;

interface ICollidable{	
	virtual void GetCollideSphere( Sphere_PosRad& out ) =0;
	virtual void HandleCollision( ICollidable* other ) =0;

	int collidableType;
};