/*
SP			THE SPRITE MODULE OF MDL
RI				  2016@MoePus
*/
#pragma once
#include <d3d11.h>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <xnamath.h>
#include <random>
#include "MDL_Core.h"
#include "MDL_Texture2D.h"
#include "MDL_Sprite_FX.h"

namespace MDL
{
	enum MDL_SPRITE_OVERLAPP_MODE
	{
		Ori_Alpha,
		All_Alpha,
		Add_Color,
		Sub_Color
	};
	struct spriteAttr
	{
		XMFLOAT2 scale = { 1.0f,1.0f };
		XMFLOAT2 position;
		float rotation = 0.0f;
		MDL_SPRITE_OVERLAPP_MODE OM = Ori_Alpha;
	};
	struct Rectf
	{
		float left;
		float right;
		float top;
		float bottom;
	};

	class sprite
	{
	private:

		DWORD textureHandle;
		ID3D11Buffer* vertexBuffer;
		ID3D11Buffer* mvpCB;
		XMMATRIX* vpMatrix;

		XMFLOAT2 size;

	public:
		sprite::sprite()
		{
			vpMatrix = (XMMATRIX*)_aligned_malloc(sizeof(XMMATRIX), 16);
		};
		void set(DWORD _textureHandle, ID3D11Buffer* _vertexBuffer,
			ID3D11Buffer* _mvpCB, XMMATRIX* _vpMatrix, XMFLOAT2 _size) 
		{
			textureHandle = _textureHandle;
			vertexBuffer = _vertexBuffer;
			mvpCB = _mvpCB;
			size = _size;

			memcpy(vpMatrix, _vpMatrix, sizeof(XMMATRIX));
		}
		sprite::~sprite()
		{
			if(vpMatrix!=nullptr)
				_aligned_free(vpMatrix);
		}
		friend class spriteHandler;
		friend class spriteRender;
		static XMMATRIX GetWorldMatrix(spriteAttr attr)
		{
			XMMATRIX _translation = XMMatrixTranslation(attr.position.x, attr.position.y, 0.0f);
			XMMATRIX _rotationZ = XMMatrixRotationZ(attr.rotation);
			XMMATRIX _scale = XMMatrixScaling(attr.scale.x, attr.scale.y, 1.0f);

			return _rotationZ * _scale* _translation;
		}
	};

	class spriteHandler
	{
	public:
		friend class spriteRender;
		static spriteHandler* getSingleton()
		{
			static spriteHandler singleton;
			return &singleton;
		}
		QWORD loadSprite(DWORD textureHandle, Rectf* cutter = nullptr);
		void unloadSprite(QWORD spriteHandle);
		XMFLOAT2 getSpriteSize(QWORD spriteHandle);
	private:
		std::unordered_map<QWORD, sprite> data;	//spritemap
		std::mt19937 mt;
	};

	XMFLOAT2 spriteHandler::getSpriteSize(QWORD spriteHandle)
	{
		return data[spriteHandle].size;
	}

	class spriteRender
	{
	public:
		static spriteRender* getSingleton()
		{
			static spriteRender* singleton = new spriteRender;
			return singleton;
		}
		spriteRender::spriteRender()
		{
			init();
		}

		spriteRender::~spriteRender()
		{
			if (defaultVS)
				defaultVS->Release();
			if (defaultPS)
				defaultPS->Release();
			if (defaultInputLayout)
				defaultInputLayout->Release();
			if (colorMapSampler)
				colorMapSampler->Release();
		}

		DWORD add2RenderList(QWORD spriteHandle, spriteAttr attrs = spriteAttr());
		spriteAttr& getspriteAttr(DWORD renderHandle);
		inline void clearRenderList();
		void doRender();
		void autoRender();
	private:
		ID3D11VertexShader*	defaultVS;
		ID3D11PixelShader*	defaultPS;
		ID3D11InputLayout*	defaultInputLayout;

		ID3D11SamplerState* colorMapSampler;

		vector<pair<QWORD, spriteAttr>> data;

		void init();
	};

	void spriteRender::init()
	{
		ID3DBlob* vsBuffer = 0;
		string shaderName = "defaultSpriteFx";
		if (!CompileShader(defaultSpriteFx, strlen(defaultSpriteFx), shaderName, "VS_Main", "vs_4_0", &vsBuffer))
		{
			MDLERROR("Error compiling the vertex shader!");
		}

		HRESULT d3dResult;
		auto mcore = core::getSingleton();
		auto d3dDevice = mcore->getDevice();


		d3dResult = d3dDevice->CreateVertexShader(vsBuffer->GetBufferPointer(),vsBuffer->GetBufferSize(), 0, &defaultVS);

		if (FAILED(d3dResult))
		{
			if (vsBuffer)
				vsBuffer->Release();
			MDLERROR("Error creating the vertex shader!");
		}

		D3D11_INPUT_ELEMENT_DESC solidColorLayout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		unsigned int totalLayoutElements = ARRAYSIZE(solidColorLayout);

		d3dResult = d3dDevice->CreateInputLayout(solidColorLayout, totalLayoutElements,vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &defaultInputLayout);

		vsBuffer->Release();

		if (FAILED(d3dResult))
		{
			MDLERROR("Error creating the input layout!");
		}

		ID3DBlob* psBuffer = 0;


		if (!CompileShader(defaultSpriteFx, strlen(defaultSpriteFx), shaderName, "PS_Main", "ps_4_0", &psBuffer))
		{
			MDLERROR("Error compiling pixel shader!");
		}

		d3dResult = d3dDevice->CreatePixelShader(psBuffer->GetBufferPointer(), psBuffer->GetBufferSize(), 0, &defaultPS);

		psBuffer->Release();

		if (FAILED(d3dResult))
		{
			MDLERROR("Error creating pixel shader!");
		}



		/*				Init Default Sampler for Sprite				 */
		D3D11_SAMPLER_DESC colorMapDesc;

		ZeroMemory(&colorMapDesc, sizeof(colorMapDesc));
		colorMapDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		colorMapDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		colorMapDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;

		d3dResult = d3dDevice->CreateSamplerState(&colorMapDesc, &colorMapSampler);

		if (FAILED(d3dResult))
		{
			MDLERROR("Failed to create color map sampler state!");
		}


		return;
	}

	DWORD spriteRender::add2RenderList(QWORD spriteHandle, spriteAttr attrs)//NOT thread safe
	{
		DWORD handle = static_cast<DWORD>(data.size());
		data.push_back(make_pair(spriteHandle, attrs));
		return handle;
	}

	spriteAttr & spriteRender::getspriteAttr(DWORD renderHandle)
	{
		return data[renderHandle].second;
	}

	inline void spriteRender::clearRenderList()
	{
		data.clear();
	}

	void spriteRender::doRender()
	{
		unsigned int stride = sizeof(VertexPos);
		unsigned int offset = 0;
		auto size = data.size();
		auto mcore = core::getSingleton();
		auto d3dContext = mcore->getContext();

		d3dContext->IASetInputLayout(defaultInputLayout);
		d3dContext->VSSetShader(defaultVS, 0, 0);
		d3dContext->PSSetShader(defaultPS, 0, 0);
		d3dContext->PSSetSamplers(0, 1, &colorMapSampler);
		d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		for (auto i = 0; i < size; i++)
		{
			
			sprite* st = &spriteHandler::getSingleton()->data[data[i].first];

			d3dContext->IASetVertexBuffers(0, 1, &st->vertexBuffer, &stride, &offset);

			auto colorMap = texture2DHandler::getSingleton()->getTextureColorMap(st->textureHandle);
			d3dContext->PSSetShaderResources(0, 1, &colorMap);


			XMMATRIX world = sprite::GetWorldMatrix(data[i].second);

			XMMATRIX mvp = XMMatrixMultiply(world, *(st->vpMatrix));
			mvp = XMMatrixTranspose(mvp);

			d3dContext->UpdateSubresource(st->mvpCB, 0, 0, &mvp, 0, 0);
			d3dContext->VSSetConstantBuffers(0, 1, &st->mvpCB);
			d3dContext->Draw(6, 0);

		}
	}

	inline void spriteRender::autoRender()
	{
		doRender();
		clearRenderList();
	}



	QWORD MDL::spriteHandler::loadSprite(DWORD textureHandle, Rectf * cutter)
	{


		ID3D11Buffer* vertexBuffer_;
		ID3D11Buffer* mvpCB_;
		XMMATRIX vpMatrix_;
		ID3D11Resource* colorTex;
		auto mcore = core::getSingleton();
		auto mt2dhan = texture2DHandler::getSingleton();

		mt2dhan->getTextureColorMap(textureHandle)->GetResource(&colorTex);

		D3D11_TEXTURE2D_DESC colorTexDesc;
		((ID3D11Texture2D*)colorTex)->GetDesc(&colorTexDesc);
		colorTex->Release();
		if (cutter == nullptr)
		{

			float halfWidth = (float)colorTexDesc.Width / 2.0f;
			float halfHeight = (float)colorTexDesc.Height / 2.0f;


			VertexPos vertices[] =
			{
				{ XMFLOAT3(halfWidth, halfHeight, 1.0f), XMFLOAT2(1.0f, 0.0f) },
				{ XMFLOAT3(halfWidth, -halfHeight, 1.0f), XMFLOAT2(1.0f, 1.0f) },
				{ XMFLOAT3(-halfWidth, -halfHeight, 1.0f), XMFLOAT2(0.0f, 1.0f) },
				{ XMFLOAT3(-halfWidth, -halfHeight, 1.0f), XMFLOAT2(0.0f, 1.0f) },
				{ XMFLOAT3(-halfWidth, halfHeight, 1.0f), XMFLOAT2(0.0f, 0.0f) },
				{ XMFLOAT3(halfWidth, halfHeight, 1.0f), XMFLOAT2(1.0f, 0.0f) },
			};
			D3D11_BUFFER_DESC vertexDesc;
			ZeroMemory(&vertexDesc, sizeof(vertexDesc));
			vertexDesc.Usage = D3D11_USAGE_DEFAULT;
			vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vertexDesc.ByteWidth = sizeof(VertexPos) * 6;

			D3D11_SUBRESOURCE_DATA resourceData;
			ZeroMemory(&resourceData, sizeof(resourceData));
			resourceData.pSysMem = vertices;


			if (FAILED(mcore->getDevice()->CreateBuffer(&vertexDesc, &resourceData, &vertexBuffer_)))
			{
				MDLERROR("Failed to create vertex buffer!");
				return false;
			}
		}
		else
		{
			float halfWidth = (float)colorTexDesc.Width *(cutter->right - cutter->left) / (float)colorTexDesc.Width / 2.0f;
			float halfHeight = (float)colorTexDesc.Height *(cutter->bottom - cutter->top) / (float)colorTexDesc.Height / 2.0f;
			VertexPos vertices[] =
			{
				{ XMFLOAT3(halfWidth, halfHeight, 1.0f), XMFLOAT2(cutter->right / (float)colorTexDesc.Width, cutter->top / (float)colorTexDesc.Height) },
				{ XMFLOAT3(halfWidth, -halfHeight, 1.0f), XMFLOAT2(cutter->right / (float)colorTexDesc.Width, cutter->bottom / (float)colorTexDesc.Height) },
				{ XMFLOAT3(-halfWidth, -halfHeight, 1.0f), XMFLOAT2(cutter->left / (float)colorTexDesc.Width, cutter->bottom / (float)colorTexDesc.Height) },
				{ XMFLOAT3(-halfWidth, -halfHeight, 1.0f), XMFLOAT2(cutter->left / (float)colorTexDesc.Width, cutter->bottom / (float)colorTexDesc.Height) },
				{ XMFLOAT3(-halfWidth, halfHeight, 1.0f), XMFLOAT2(cutter->left / (float)colorTexDesc.Width,cutter->top / (float)colorTexDesc.Height) },
				{ XMFLOAT3(halfWidth, halfHeight, 1.0f), XMFLOAT2(cutter->right / (float)colorTexDesc.Width, cutter->top / (float)colorTexDesc.Height) },
			};
			D3D11_BUFFER_DESC vertexDesc;
			ZeroMemory(&vertexDesc, sizeof(vertexDesc));
			vertexDesc.Usage = D3D11_USAGE_DEFAULT;
			vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vertexDesc.ByteWidth = sizeof(VertexPos) * 6;

			D3D11_SUBRESOURCE_DATA resourceData;
			ZeroMemory(&resourceData, sizeof(resourceData));
			resourceData.pSysMem = vertices;
			if (FAILED(mcore->getDevice()->CreateBuffer(&vertexDesc, &resourceData, &vertexBuffer_)))
			{
				MDLERROR("Failed to create vertex buffer!");
				return false;
			}
		}
		
		D3D11_BUFFER_DESC constDesc;
		ZeroMemory(&constDesc, sizeof(constDesc));
		constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constDesc.ByteWidth = sizeof(XMMATRIX);
		constDesc.Usage = D3D11_USAGE_DEFAULT;


		if (FAILED(mcore->getDevice()->CreateBuffer(&constDesc, 0, &mvpCB_)))
		{
			return false;
		}


		XMMATRIX view = XMMatrixIdentity();
		XMMATRIX projection = XMMatrixOrthographicOffCenterLH(0.0f, (float)mcore->getWidth(), 0.0f, (float)mcore->getHeight(), 0.1f, 100.0f);
		vpMatrix_ = XMMatrixMultiply(view, projection);


		QWORD spriteHandle = (static_cast<QWORD>(textureHandle) << 32) + mt();

		auto st = &data[spriteHandle];
		st->set(textureHandle, vertexBuffer_, mvpCB_, &vpMatrix_, { (float)colorTexDesc.Width, (float)colorTexDesc.Height });
		return spriteHandle;
	}

	void spriteHandler::unloadSprite(QWORD spriteHandle)
	{
		if (data[spriteHandle].vertexBuffer)
			data[spriteHandle].vertexBuffer->Release();


		if (data[spriteHandle].mvpCB)
			data[spriteHandle].mvpCB->Release();

		data.erase(spriteHandle);
	}


}