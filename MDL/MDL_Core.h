/*
CO				THE MINI DRAW LIBRARY
RE					2016@MoePus
*/
#pragma once
#include <windows.h>
#include <dxgi.h>
#include <iostream>
#include <string>
#include <mutex>
#include <d3dcommon.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include "MDL_STPK.h"

#pragma comment ( lib, "D3D11.lib")
#pragma comment ( lib, "DXGI.lib")
#pragma comment ( lib, "d3dcompiler.lib")
#pragma comment ( lib, "D3Dx11.lib")

namespace MDL
{
	DWORD SpecialFNVHash(const char *begin, const  char *end, DWORD initHash);

	typedef unsigned long long	QWORD;
	typedef unsigned long	DWORD;
	typedef unsigned short	WORD;
	typedef unsigned char	BYTE;

	struct VertexPos
	{
		XMFLOAT3 pos;
		XMFLOAT2 tex0;
	};

	void MDLERROR(std::string _msg_)
	{
		OutputDebugString(_msg_.c_str());
		std::terminate();
	}
	class core
	{

	public:
		
		static core* getSingleton()
		{
			static core* singleton = new core;
			return singleton;
		}
		core();
		~core();
		HWND CreateMDLWindow(HINSTANCE hInstance, char* windowname, char* classname, int _width, int _height, WNDPROC WndProc = coreDefaultWndProc);
		BOOL ShowMDLWindow(int nCmdShow);

		HRESULT InitDevice(int fpsNum, int fpsDen = 1);
		bool InitStpk(string idxName);
		ID3D11Device* getDevice()
		{
			return d3dDevice;
		}
		ID3D11DeviceContext* getContext()
		{
			return ImmediateContext;
		}
		void clearComposition();
		void RenderPresentSync()
		{
			SwapChain->Present(1, 0);
		}
		void RenderPresent()
		{
			SwapChain->Present(0, 0);
		}

		typedef STPK::stpkHandler::STPK_ret MDL_FILE;
		MDL_FILE getFile(string fileName);
		MDL_FILE getFile(DWORD fileNameHash);

		int getWidth() { return width; }
		int getHeight() { return height; }

	private:
		HINSTANCE	hInst;
		HWND		hWnd;
		int			width;
		int			height;

		const D3D_DRIVER_TYPE   driverType = D3D_DRIVER_TYPE_HARDWARE;
		ID3D11Device*           d3dDevice;
		ID3D11DeviceContext*    ImmediateContext;
		IDXGISwapChain*         SwapChain;
		ID3D11RenderTargetView* RenderTargetView;

		void CleanupDevice()
		{
			if (ImmediateContext) ImmediateContext->ClearState();
			if (RenderTargetView) RenderTargetView->Release();
			if (SwapChain) SwapChain->Release();
			if (ImmediateContext) ImmediateContext->Release();
			if (d3dDevice) d3dDevice->Release();
		}

		static LRESULT CALLBACK coreDefaultWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			PAINTSTRUCT ps;
			HDC hdc;

			switch (message)
			{
			case WM_PAINT:
				hdc = BeginPaint(hWnd, &ps);
				EndPaint(hWnd, &ps);
				break;

			case WM_DESTROY:
				PostQuitMessage(0);
				break;

			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}

			return 0;
		}
	};

	core::core()
	{

		auto stpk = STPK::stpkHandler::getSingleton();
	}

	core::~core()
	{
		CleanupDevice();
	}

	HWND core::CreateMDLWindow(HINSTANCE hInstance, char* windowname, char* classname, int _width, int _height, WNDPROC WndProc)
	{
		width = _width;
		height = _height;

		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = NULL;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = classname;
		wcex.hIconSm = NULL;
		if (!RegisterClassEx(&wcex))
			return FALSE;

		hInst = hInstance;
		RECT rc = { 0, 0, width, height };
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
		hWnd = CreateWindow(wcex.lpszClassName, windowname, WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
			NULL);
		if (!hWnd)
			return FALSE;

		return hWnd;
	}
	BOOL core::ShowMDLWindow(int nCmdShow)
	{
		return ShowWindow(hWnd, nCmdShow);
	}
	HRESULT core::InitDevice(int fpsNum,int fpsDen)
	{
		HRESULT hr;

		UINT createDeviceFlags = 0;
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = fpsNum;
		sd.BufferDesc.RefreshRate.Denominator = fpsDen;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		
		hr = D3D11CreateDeviceAndSwapChain(NULL, driverType, NULL, createDeviceFlags, &featureLevel, 1,
				D3D11_SDK_VERSION, &sd, &SwapChain, &d3dDevice, NULL, &ImmediateContext);

		if (FAILED(hr))
			return hr;

		ID3D11Texture2D* pBackBuffer = NULL;
		hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
		if (FAILED(hr))
			return hr;

		hr = d3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &RenderTargetView);

		pBackBuffer->Release();
		if (FAILED(hr))
			return hr;

		ImmediateContext->OMSetRenderTargets(1, &RenderTargetView, NULL);

		// Setup the viewport
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)width;
		vp.Height = (FLOAT)height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		ImmediateContext->RSSetViewports(1, &vp);


		/*				Init Default BlendState 			 */
		ID3D11BlendState* alphaBlendState_;
		D3D11_BLEND_DESC blendDesc;
		ZeroMemory(&blendDesc, sizeof(blendDesc));

		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0f;

		const static float oBECL[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		d3dDevice->CreateBlendState(&blendDesc, &alphaBlendState_);
		ImmediateContext->OMSetBlendState(alphaBlendState_, oBECL, 0xFFFFFFFF);
		alphaBlendState_->Release();

		return S_OK;
	}

	bool core::InitStpk(string idxName)
	{
		char IDXPATH[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, IDXPATH);
		string idxFullPath = IDXPATH + string("\\") + idxName;
		auto mstpk = STPK::stpkHandler::getSingleton();
		
		return mstpk->init(idxFullPath);
	}

	void core::clearComposition()
	{
		float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; //red,green,blue,alpha
		ImmediateContext->ClearRenderTargetView(RenderTargetView, ClearColor);
	}

	inline core::MDL_FILE core::getFile(string fileName)
	{
		DWORD hash = SpecialFNVHash(fileName.c_str(), fileName.c_str() + fileName.length(), NULL);
		return getFile(hash);
	}

	inline core::MDL_FILE core::getFile(DWORD fileNameHash)
	{
		auto mstpk = STPK::stpkHandler::getSingleton();
		return mstpk->STPK_read(fileNameHash);
	}


	/*       ShaderCompileDefinitions        */
	constexpr DWORD D3DCOMPILE_DEBUG = (1 << 0);
	constexpr DWORD D3DCOMPILE_SKIP_VALIDATION = (1 << 1);
	constexpr DWORD D3DCOMPILE_SKIP_OPTIMIZATION = (1 << 2);
	constexpr DWORD D3DCOMPILE_PACK_MATRIX_ROW_MAJOR = (1 << 3);
	constexpr DWORD D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR = (1 << 4);
	constexpr DWORD D3DCOMPILE_PARTIAL_PRECISION = (1 << 5);
	constexpr DWORD D3DCOMPILE_FORCE_VS_SOFTWARE_NO_OPT = (1 << 6);
	constexpr DWORD D3DCOMPILE_FORCE_PS_SOFTWARE_NO_OPT = (1 << 7);
	constexpr DWORD D3DCOMPILE_NO_PRESHADER = (1 << 8);
	constexpr DWORD D3DCOMPILE_AVOID_FLOW_CONTROL = (1 << 9);
	constexpr DWORD D3DCOMPILE_PREFER_FLOW_CONTROL = (1 << 10);
	constexpr DWORD D3DCOMPILE_ENABLE_STRICTNESS = (1 << 11);
	constexpr DWORD D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY = (1 << 12);
	constexpr DWORD D3DCOMPILE_IEEE_STRICTNESS = (1 << 13);
	constexpr DWORD D3DCOMPILE_OPTIMIZATION_LEVEL0 = (1 << 14);
	constexpr DWORD D3DCOMPILE_OPTIMIZATION_LEVEL1 = 0;
	constexpr DWORD D3DCOMPILE_OPTIMIZATION_LEVEL2 = ((1 << 14) | (1 << 15));
	constexpr DWORD D3DCOMPILE_OPTIMIZATION_LEVEL3 = (1 << 15);
	constexpr DWORD D3DCOMPILE_RESERVED16 = (1 << 16);
	constexpr DWORD D3DCOMPILE_RESERVED17 = (1 << 17);
	constexpr DWORD D3DCOMPILE_WARNINGS_ARE_ERRORS = (1 << 18);

	BOOL CompileShader(char* mem, SIZE_T len, string fxPath, char* entry, char* shaderModel, ID3DBlob** buffer)
	{
		DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

		#if defined( DEBUG ) || defined( _DEBUG )
		shaderFlags |= D3DCOMPILE_DEBUG;
		#endif

		ID3DBlob* errorBuffer = 0;
		HRESULT result;

		result = D3DX11CompileFromMemory(mem, len, fxPath.c_str(), 0, 0, entry, shaderModel,
			shaderFlags, 0, 0, buffer, &errorBuffer, 0);

		if (FAILED(result))
		{
			if (errorBuffer != 0)
			{
				OutputDebugStringA((char*)errorBuffer->GetBufferPointer());
				errorBuffer->Release();
			}

			return false;
		}

		if (errorBuffer != 0)
			errorBuffer->Release();

		return true;
	}

	DWORD SpecialFNVHash(const char *begin, const  char *end, DWORD initHash)//!Riatre!
	{
		if (!initHash)
		{
			initHash = 0x811C9DC5u;
		}
		DWORD hash;
		byte ch;

		int inMBCS = 0;

		for (hash = initHash; begin != end; hash = (hash^ch) * 0x1000193)
		{
			ch = *begin++;
			if (ch == '/') ch = '\\';
			if (!inMBCS && ch >= 128)
				inMBCS = 2;
			if (!inMBCS)
			{
				ch = tolower(ch);  // bad ass style but WORKS PERFECTLY!
			}
			else inMBCS--;
		}
		return hash;
	}

}