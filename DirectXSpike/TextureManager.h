#pragma once

#include "singleton.h"

typedef int TextureContextIdType;

#define MAX_TEX_CONTEXT_CNT		(5)
#define TEX_CONTEXT_NAME_LEN	(20)
#define INVALID_TEX_CONTEXT		(0)
#define MAX_TEX_FILENAME_LEN	(50)
#define MAX_TEXTURES			(20)
#define GLOBAL_TEX_CONTEXT		(1)
#define DEFAULT_TEXTURE			"defaultTexture.dds"

typedef enum 
{
	DEFAULT_TEXTURE_ID = 0,
	GLOBAL_TEX_CNT
};

typedef struct
{
	int						globalTextureId;
	char					filename[MAX_TEX_FILENAME_LEN];
} GlobalTextureDef;

const GlobalTextureDef m_globalTextureDef[] =
{{DEFAULT_TEXTURE_ID, "defaultTexture.dds"}
};

typedef struct 
{
	TextureContextIdType	id;
	char					name[TEX_CONTEXT_NAME_LEN];
} TextureContextType;

typedef struct
{
	TextureContextIdType	parent_context;
	LPDIRECT3DTEXTURE9		tex;
	char					filename[MAX_TEX_FILENAME_LEN];

} TextureInstanceType;

class CTextureManager : public SingleThreadSingleton<CTextureManager>
{
	friend class SingleThreadSingleton<CTextureManager>;
private:
	CTextureManager(void);
	~CTextureManager(void);

public:
	void Init(LPDIRECT3DDEVICE9 device);
	TextureContextIdType CreateTextureContext(const char* name);
	void UnloadTextureContext(TextureContextIdType contextId);
	LPDIRECT3DTEXTURE9 GetTexture(TextureContextIdType context, const char* filename);

private:
	TextureInstanceType* InsertNewTextureInMap(TextureContextIdType context, const char* filename);

private:
	static int				m_nextTextureContextId;
	TextureContextType		m_contexts[MAX_TEX_CONTEXT_CNT];
	TextureInstanceType		m_textures[MAX_TEXTURES];
	LPDIRECT3DDEVICE9		m_device;
};