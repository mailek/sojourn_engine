#pragma once
//////////////////////////////////////////////////////////////////////////
// ShaderManager.h - 2011 Matthew Alford
// 
// Description: Creates, loads, and unloads shader programs.  Provides
//	shader parameter persistance and ability to reload shaders.
//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////
// Includes
//////////////////////////////////////
#include <stack>
#include "Camera.h"

//////////////////////////////////////
// Type Definitions
//////////////////////////////////////

//////////////////////////////////////
// Constants
//////////////////////////////////////
enum ePassID {
		PASS_DEFAULT,
		PASS_RTPLAIN,
		//PASS_RTBLURH,
		//PASS_RTBLURV,
		PASS_PRIMITIVE,
		PASS_PRIM_COLORED,
		PASS_CNT,
		PASS_INVALID = -1
	};

enum SHADER_VAR_TYPE {
		SHADER_VAR_INVALID,
		SHADER_VAR_MATRIX,
		SHADER_VAR_FLOAT4,
		SHADER_VAR_FLOAT
	};


//////////////////////////////////////
// Class Definition
//////////////////////////////////////
class CShaderManager
{
public:
	typedef struct {
		int						type;
		union {
			float				_matrix[16];
			float				_float;
			float				_float4[4];
		};
	} ShaderVariant;

	typedef struct {
		ShaderVariant			worldTransform;
		D3DXMATRIX				viewProjection;
		D3DXMATRIX				view;
		D3DXMATRIX				projection;
		ShaderVariant			matDiffuse;
		ShaderVariant			matAmbient;
		ShaderVariant			drawColor;
		ShaderVariant			dirToLight;
		ShaderVariant			invViewportWidth;
		ShaderVariant			invViewportHeight;
	} ShaderParams;

	typedef struct {
		IDirect3DVertexShader9	*pVS;
		IDirect3DPixelShader9	*pPS;
		ID3DXConstantTable		*pVSConstTable;
		ID3DXConstantTable		*pPSConstTable;
		ShaderParams			params;
	} ShaderPass;

	typedef struct {
		ePassID					VSPass;
		ePassID					PSPass;
	} ShaderStackItem;

public:
	CShaderManager(void);
	virtual ~CShaderManager(void);

private:
	std::vector<ShaderPass>		m_vecPasses;
	D3DLIGHT9					m_DirectionalLight;
	bool						m_bDebugRotateLight;
	std::stack<ShaderStackItem> m_shaderStack;
	ePassID						m_currentVS;
	ePassID						m_currentPS;

	void ReloadPixelShader(ePassID passId);
	void ReloadVertexShader(ePassID passId);
	void CreateVertexShader(ShaderPass &pass, LPCSTR vsFileName);
	void CreatePixelShader(ShaderPass &pass, LPCSTR psFileName);
	void SetInvViewport(ePassID pass, float viewWidth, float viewHeight, bool pixelShader=false);
	bool SetShaderConstant( ID3DXConstantTable *pConsts, LPCSTR constName, ShaderVariant &variant );
	
	inline IDirect3DVertexShader9 *GetVertexShaderByPass(ePassID id) {return m_vecPasses[id].pVS;}
	inline IDirect3DPixelShader9 *GetPixelShaderByPass(ePassID id) {return m_vecPasses[id].pPS;}

protected:
	virtual void Setup();

	LPDIRECT3DDEVICE9			m_device;
	unsigned int				m_iViewportWidth;
	unsigned int				m_iViewportHeight;

public:
	void SetDevice(LPDIRECT3DDEVICE9 device);
	void SetWorldTransform(ePassID pass, D3DXMATRIX worldTransform);
	void SetViewProjection(ePassID pass, D3DXMATRIX viewTransform, D3DXMATRIX projectionTransform);
	void SetMaterial(ePassID pass, const D3DMATERIAL9 &mat );
	void SetDrawColor(ePassID pass, D3DXCOLOR color );

	void SetVertexShader(ePassID vtShaderId);
	void SetPixelShader(ePassID pxShaderId);
	void Update(float elapsed);
	void PushCurrentShader();
	void PopCurrentShader();
	
	inline void SetViewDimensions(unsigned int width, unsigned int height) { m_iViewportWidth=width; m_iViewportHeight=height;}
	inline D3DXVECTOR3 GetLightDirection(void) {return D3DXVECTOR3(m_DirectionalLight.Direction.x, m_DirectionalLight.Direction.y, m_DirectionalLight.Direction.z);}
	inline void SetDebugRotateLight(bool rotate) {m_bDebugRotateLight = rotate;}
};