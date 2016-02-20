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
		static texture2DHandler& getSingleton()
		{
			static texture2DHandler singleton;
			return singleton;
		}
		texture2DHandler();
		~texture2DHandler();

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

	DWORD texture2DHandler::loadTexture(string fileName)
	{
		DWORD texture_handle = SpecialFNVHash(fileName.c_str(), fileName.c_str() + fileName.length(), NULL);
		HRESULT d3dResult;
		ID3D11ShaderResourceView* colorMap_;

		auto mcore = core::getSingleton();


		auto tbM = mcore.getFile(texture_handle);
		d3dResult = D3DX11CreateShaderResourceViewFromMemory(mcore.getDevice(), tbM.mem.get(), tbM.SIZE, 0, 0, &colorMap_, 0);


		if (FAILED(d3dResult))
		{
			MDLERROR("Failed to load the texture image!");
			return false;
		}


		data[texture_handle].colorMap = colorMap_;


		return texture_handle;
	}

	void texture2DHandler::unloadTexture(DWORD textureHandle)
	{
		if (data[textureHandle].colorMap)
			data[textureHandle].colorMap->Release();
		data.erase(textureHandle);
	}

	texture2D  texture2DHandler::getTexture(DWORD id)
	{
		if (data.find(id) != data.end())
		{
			return data[id];
		}
		else
		{
			return texture2D{ 0,0 };
		}
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