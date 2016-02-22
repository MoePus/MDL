/*
TE			THE TEXTURE2D MODULE OF MDL
2D				   2016@MoePus
*/
#pragma once
#include <d3d11.h>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include "MDL_Core.h"
using namespace std;

namespace MDL
{
	struct texture2D
	{
		ID3D11ShaderResourceView*	colorMap;
	};

	class texture2DHandler
	{
	public:
		static texture2DHandler* getSingleton()
		{
			static texture2DHandler* singleton = new texture2DHandler;
			return singleton;
		}
		texture2DHandler();
		~texture2DHandler();

		void appendTexture(DWORD textureHandle, texture2D texture);
		DWORD loadTexture(string fileName);
		void unloadTexture(DWORD textureHandle);

		texture2D getTexture(DWORD id);
		ID3D11ShaderResourceView* getTextureColorMap(DWORD id);
	private:
		std::unordered_map<DWORD, texture2D> data;	//TextureMap
	};

	texture2DHandler::texture2DHandler()
	{
	}

	texture2DHandler::~texture2DHandler()
	{
	}

	void texture2DHandler::appendTexture(DWORD textureHandle, texture2D texture)
	{
		data[textureHandle] = texture;
	}

	DWORD texture2DHandler::loadTexture(string fileName)
	{
		DWORD textureHandle = SpecialFNVHash(fileName.c_str(), fileName.c_str() + fileName.length(), NULL);
		HRESULT d3dResult;
		ID3D11ShaderResourceView* colorMap_;

		auto mcore = core::getSingleton();


		auto tbM = mcore->getFile(textureHandle);
		D3DX11_IMAGE_LOAD_INFO textureInfoDesc;
		textureInfoDesc.MipLevels = 1;
		d3dResult = D3DX11CreateShaderResourceViewFromMemory(mcore->getDevice(), tbM.mem, tbM.SIZE, &textureInfoDesc, 0, &colorMap_, 0);

		free(tbM.mem);

		if (FAILED(d3dResult))
		{
			MDLERROR("Failed to load the texture image!");
			return false;
		}


		data[textureHandle].colorMap = colorMap_;


		return textureHandle;
	}

	void texture2DHandler::unloadTexture(DWORD textureHandle)
	{
		if (data[textureHandle].colorMap)
			data[textureHandle].colorMap->Release();
		data.erase(textureHandle);
	}

	texture2D  texture2DHandler::getTexture(DWORD id)
	{
		if (data.find(id) == data.end())
		{
			MDLERROR("Error when get a texture");
		}
		return data[id];
	}

	inline ID3D11ShaderResourceView * texture2DHandler::getTextureColorMap(DWORD id)
	{
		if (data.find(id) != data.end())
		{
			return data[id].colorMap;
		}
		else
		{
			return NULL;
		}
	}

}