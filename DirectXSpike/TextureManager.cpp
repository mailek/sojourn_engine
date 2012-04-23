/********************************************************************
	created:	2012/04/16
	filename: 	TextureManager.cpp
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "StdAfx.h"
#include "TextureManager.h"

int CTextureManager::m_nextTextureContextId = 2;

CTextureManager::CTextureManager(void) : m_device(NULL)
{
	::ZeroMemory(m_textures, sizeof(m_textures));
	::ZeroMemory(m_contexts, sizeof(m_contexts));
}

CTextureManager::~CTextureManager(void)
{
}

TextureContextIdType CTextureManager::CreateTextureContext(const char* name)
{
	TextureContextIdType id = m_nextTextureContextId;
	assert(id < MAX_TEX_CONTEXT_CNT);

	TextureContextType* context = NULL;
	for(int i = 0; i < MAX_TEX_CONTEXT_CNT; i++)
	{
		if(m_contexts[i].id == INVALID_TEX_CONTEXT)
		{
			context = &m_contexts[i];
			break;
		}
	}
	assert(context);

	context->id = id;
	strcpy_s(context->name, name);

	return m_nextTextureContextId++;
}

void CTextureManager::UnloadTextureContext(TextureContextIdType contextId)
{
	TextureContextType* context = NULL;
	for(int i = 0; i < MAX_TEX_CONTEXT_CNT; i++)
	{
		if(m_contexts[i].id == contextId)
		{
			context = &m_contexts[i];
			break;
		}
	}
	assert(context);

	for(int i = 0; i < MAX_TEXTURES; i++)
	{
		TextureInstanceType* texture = &m_textures[i];
		if(texture->parent_context == context->id)
		{
			COM_SAFERELEASE(texture->tex);
			texture->filename[0] = 0;
			texture->parent_context = INVALID_TEX_CONTEXT;
		}
	}

	context->id = INVALID_TEX_CONTEXT;
	context->name[0] = 0;
}


LPDIRECT3DTEXTURE9 CTextureManager::GetTexture(TextureContextIdType context, const char* filename)
{
	LPDIRECT3DTEXTURE9 ret = NULL;

	if(context != GLOBAL_TEX_CONTEXT)
	{
		// Verify the context is valid
		TextureContextType* parent = NULL;
		for(int i = 0; i < MAX_TEX_CONTEXT_CNT; i++)
		{
			if(m_contexts[i].id == context)
			{
				parent = &m_contexts[i];
				break;
			}
		}
		assert(parent);
	}

	// Try to find the texture
	for(int i = 0; i < MAX_TEXTURES; i++)
	{
		TextureInstanceType* texture = &m_textures[i];
		if(texture->parent_context == context && _stricmp(texture->filename, filename) == 0)
		{
			ret = texture->tex;
		}
	}

	// If it doesn't exist then add it to the context
	if(ret == NULL && context != GLOBAL_TEX_CONTEXT)
	{
		TextureInstanceType* newTex = InsertNewTextureInMap(context, filename);
		ret = newTex->tex;
	}

	return ret;
}

void CTextureManager::Init(LPDIRECT3DDEVICE9 device)
{
	m_device = device;

	// Load the global textures
	for(int i = 0; i < GLOBAL_TEX_CNT; i++)
	{
		const GlobalTextureDef* tex = &m_globalTextureDef[i];
		InsertNewTextureInMap(GLOBAL_TEX_CONTEXT, tex->filename);
	}
	
}

TextureInstanceType* CTextureManager::InsertNewTextureInMap(TextureContextIdType context, const char* filename)
{
	TextureInstanceType* ret = NULL;

	TextureInstanceType* search = NULL;
	for(int i = 0; i < MAX_TEXTURES; i++)
	{
		search = &m_textures[i];
		if(search->parent_context == INVALID_TEX_CONTEXT)
		{
			ret = search;
			break;
		}
	}
	assert(ret);

	char strFilename[MAX_TEX_FILENAME_LEN];
	strcpy_s(strFilename, RESOURCE_FOLDER);
	strcat_s(strFilename, filename);
	
	D3DXCreateTextureFromFile(m_device, strFilename, &ret->tex);
	
	// if texture wasn't found, try returning the default texture
	if(ret->tex == NULL && strcmp(filename, DEFAULT_TEXTURE)!=0)
	{
		TextureInstanceType* search = NULL;
		for(int i = 0; i < MAX_TEXTURES; i++)
		{
			search = &m_textures[i];
			if(search->parent_context == GLOBAL_TEX_CONTEXT && _stricmp(search->filename, DEFAULT_TEXTURE)==0)
			{
				ret = search;
				break;
			}
		}
		assert(ret);
	}
	else
	{
		strcpy_s(ret->filename, filename);
		ret->parent_context = context;
	}

	return ret;
}