#pragma once
/********************************************************************
	created:	2012/04/23
	filename: 	ICollidable.h
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "mathutil.h"

typedef enum _ECollidableType
{
	OBSTACLE,
	CHARACTER
} ECollidableType;

interface ICollidable{	
	virtual void GetCollideSphere( Sphere_PosRad& out ) =0;
	virtual void HandleCollision( ICollidable* other ) =0;

	int collidableType;
};