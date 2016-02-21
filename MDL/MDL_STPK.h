/*
ST				A PACKAGE FORMAT
PK				  2016@MoePus
*/
#pragma once
#include "MDL_Core.h"
#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
using namespace std;

namespace STPK
{
	typedef unsigned char	BYTE;

	class stpkHandler
	{
	public:
		static stpkHandler* getSingleton()
		{
			static stpkHandler* singleton = new stpkHandler;
			return singleton;
		}
		stpkHandler();
		~stpkHandler();

		struct STPK_ret
		{
			DWORD SIZE;
			unique_ptr<BYTE> mem;
		};
	private:
		struct mIDX
		{
			DWORD HASH;
			DWORD OFFSET;
			DWORD SIZE;
		};

		STPK_ret STPK_read(DWORD HASH);
		friend class core;
		bool init(string idxName);

		bool inited;
	};

	stpkHandler::stpkHandler()
	{
		inited = false;
	}

	stpkHandler::~stpkHandler()
	{
	}

	inline stpkHandler::STPK_ret stpkHandler::STPK_read(DWORD HASH)
	{
		return STPK_ret();
	}

	bool stpkHandler::init(string idxName)
	{
		if (!inited)
		{



		}
		return false;
	}

}