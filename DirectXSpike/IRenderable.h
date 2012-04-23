#pragma once
/********************************************************************
	created:	2012/04/16
	filename: 	IRenderable.h
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "MathUtil.h"

class CShaderManager;
class CRenderEngine;

interface IRenderable{ // look into converting to templates
	virtual void Render( CRenderEngine &rndr ) =0;
	virtual D3DXMATRIX GetWorldTransform() =0;
	virtual bool IsTransparent() =0;
	virtual void SetLastRenderFrame(UINT frameNum) =0;
	virtual UINT GetLastRenderFrame() =0;
	virtual Sphere_PosRad GetBoundingSphere() =0;
};
typedef std::list<IRenderable*> SceneMgrRenderList;
