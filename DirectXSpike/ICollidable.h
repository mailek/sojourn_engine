#pragma once

#include "mathutil.h"

interface ICollidable{	
	virtual void GetCollideSphere( Sphere_PosRad& out ) =0;
};