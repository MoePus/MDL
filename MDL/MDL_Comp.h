/*
CO			THE COMPOSITION MODULE OF MDL
MP					2016@MoePus
*/
#pragma once
#include <d3d11.h>
#include <iostream>
#include <string>
#include "MDL_Core.h"
#include "MDL_Texture2D.h"

namespace MDL
{
	template<typename funtype>
	class composition
	{
	private:
		funtype renderFunc;
		DWORD compTextHandle;
		ID3D11Texture2D*			compTexture;
		ID3D11RenderTargetView*		compTargetView;
		ID3D11ShaderResourceView*	colorMap;


		ID3D11RenderTargetView*		oldTargetView;
		ID3D11DepthStencilView*		olddepthStencilView;

		bool init();
		bool bindeColorMap();
	public:
		composition::composition(funtype _renderFunc, std::string compName);
		//composition::composition() {};   //CanT
		composition::~composition();
		void setRenderAttrs(funtype _renderFunc, std::string compName);

		void clearComposition()
		{
			auto mcore = core::getSingleton();
			float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; //red,green,blue,alpha
			mcore->getContext()->ClearRenderTargetView(compTargetView, ClearColor);
		}
		void doRender();
		void autoRender();

		void saveCurrentRenderTargetView();
		void setRenderTargetViewBack();
		void setRenderTargetView();

		DWORD sendColorMapToTexture2DHandle();
	};

	template<typename funtype>
	bool composition<funtype>::init()
	{
		D3D11_TEXTURE2D_DESC textureDesc;
		auto mcore = core::getSingleton();
		ZeroMemory(&textureDesc, sizeof(textureDesc));


		textureDesc.Width = mcore->getWidth();
		textureDesc.Height = mcore->getHeight();
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		/*				Init render target texture for this comp			 */
		if (FAILED(mcore->getDevice()->CreateTexture2D(&textureDesc, NULL, &compTexture)))
		{
			return false;
		}


		/*				Init render target view for this comp				  */
		if (FAILED(mcore->getDevice()->CreateRenderTargetView(compTexture, NULL, &compTargetView)))
		{
			return false;
		}

		return bindeColorMap();
	}

	template<typename funtype>
	inline bool composition<funtype>::bindeColorMap()
	{
		auto mcore = core::getSingleton();

		D3D11_SHADER_RESOURCE_VIEW_DESC colorMapDesc;
		colorMapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		colorMapDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		colorMapDesc.Texture2D.MostDetailedMip = 0;
		colorMapDesc.Texture2D.MipLevels = 1;

		if (FAILED(mcore->getDevice()->CreateShaderResourceView(compTexture, &colorMapDesc, &colorMap)))
		{
			return false;
		}
		return true;
	}

	template<typename funtype>
	inline composition<funtype>::composition(funtype _renderFunc, std::string compName)
	{
		setRenderAttrs(_renderFunc, compName);
		init();
	}


	template<typename funtype>
	inline composition<funtype>::~composition()
	{
		if (colorMap)
		{
			colorMap->Release();
		}

		if (compTargetView)
		{
			compTargetView->Release();
		}

		if (compTexture)
		{
			compTexture->Release();
		}
	}

	template<typename funtype>
	inline void composition<funtype>::setRenderAttrs(funtype _renderFunc, std::string compName)
	{
		compTextHandle = SpecialFNVHash(compName.c_str(), compName.c_str() + compName.length(), NULL);
		renderFunc = _renderFunc;
	}

	template<typename funtype>
	inline void composition<funtype>::doRender()
	{
		if (!colorMap)
			bindeColorMap();
		renderFunc();
	}

	template<typename funtype>
	inline void composition<funtype>::autoRender()
	{
		clearComposition();
		saveCurrentRenderTargetView();
		setRenderTargetView();
		doRender();
		setRenderTargetViewBack();
	}

	template<typename funtype>
	inline void composition<funtype>::saveCurrentRenderTargetView()
	{
		auto mcore = core::getSingleton();
		mcore->getContext()->OMGetRenderTargets(1, &oldTargetView, &olddepthStencilView);
	}

	template<typename funtype>
	inline void composition<funtype>::setRenderTargetViewBack()
	{
		auto mcore = core::getSingleton();
		mcore->getContext()->OMSetRenderTargets(1, &oldTargetView, olddepthStencilView);
	}

	template<typename funtype>
	inline void composition<funtype>::setRenderTargetView()
	{
		auto mcore = core::getSingleton();
		mcore->getContext()->OMSetRenderTargets(1, &compTargetView, NULL);
	}

	template<typename funtype>
	DWORD composition<funtype>::sendColorMapToTexture2DHandle()
	{
		if (colorMap)
		{
			auto mT2D = texture2DHandler::getSingleton();
			mT2D->appendTexture(compTextHandle, texture2D{ colorMap });
			colorMap = nullptr;
		}
		return compTextHandle;
	}



}