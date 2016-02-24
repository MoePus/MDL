#include "MDL_Core.h"
#include "windows.h"
#include "iostream"
#include <thread>
#include "MDL_Texture2D.h"
#include "MDL_Sprite.h"
#include "MDL_Comp.h"
#include <functional>

void renderLoop(DWORD64 sH);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	auto mcore = MDL::core::getSingleton();
	mcore->CreateMDLWindow(hInstance, "ExampleMDL", "exp", 1600, 900);
	mcore->InitDevice(60);
	mcore->InitStpk("exp.idx");
	auto mt2D = MDL::texture2DHandler::getSingleton();
	auto msprite = MDL::spriteHandler::getSingleton();
	auto msr = MDL::spriteRender::getSingleton();

	auto tH = mt2D->loadTexture("exp\\1.png");
	auto sH = msprite->loadSprite(tH);

	mcore->ShowMDLWindow(nCmdShow);
	MSG msg = { 0 };
	std::thread rl(renderLoop,sH);
	rl.detach();
	while (WM_QUIT != msg.message)
	{
		
		if (GetMessage(&msg, NULL, 0, 0)>0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

	}

	rl.~thread();

}

void renderLoop(DWORD64 sH)
{
	auto mcore = MDL::core::getSingleton();
	auto mt2D = MDL::texture2DHandler::getSingleton();
	auto msprite = MDL::spriteHandler::getSingleton();
	auto msr = MDL::spriteRender::getSingleton();


	std::function<void()> renderFunc = [&]()
	{
		auto rH = msr->add2RenderList(sH);

		auto& attr = msr->getspriteAttr(rH);
		auto sprSize = msprite->getSpriteSize(sH);
		float sc = min((float)mcore->getWidth() / sprSize.x, (float)mcore->getHeight() / sprSize.y);
		attr.position = { sprSize.x / 2 * sc,sprSize.y / 2 * sc };
		attr.scale = { sc,sc };

		msr->autoRender();
	};


	while (1)
	{
		mcore->clearComposition();

		auto Acomp = MDL::composition<std::function<void()>>(renderFunc, "一个合成组");
		Acomp.autoRender();
		auto tcompH = Acomp.sendColorMapToTexture2DHandle();
		auto scompH = msprite->loadSprite(tcompH);
		auto rcompH = msr->add2RenderList(scompH);
		auto& attr = msr->getspriteAttr(rcompH);
		attr.position = { 800.0f,450.0f };

		msr->autoRender();
		msprite->unloadSprite(scompH);
		mt2D->unloadTexture(tcompH);
		mcore->RenderPresentSync();
	}

}