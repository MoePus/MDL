/*
ST				A PACKAGE FORMAT
PK				  2016@MoePus
*/
#pragma once
#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
#include "MDL_Core.h"
using namespace std;

namespace STPK
{
	class stpkHandler
	{
	public:
		static stpkHandler* getSingleton()
		{
			static stpkHandler singleton;
			return &singleton;
		}
		stpkHandler();
		~stpkHandler();

		struct STPK_ret
		{
			DWORD SIZE;
			char* mem;
		};

		STPK_ret STPK_read(DWORD HASH)
		{
			for (int i = 0; i < STPK_filenum; i++)
			{
				if (IDXattr[i].HASH == HASH)
				{
					DWORD rb;
					SetFilePointer(STPK_HBIN, IDXattr[i].OFFSET, NULL, FILE_BEGIN);
					char* src = (char*)malloc(IDXattr[i].SIZE);
					auto dst = (char*)malloc(IDXattr[i].SIZE - 4);
					ReadFile(STPK_HBIN, src, IDXattr[i].SIZE, &rb, NULL);
					DWORD filesize = STPK_dec(src, dst, IDXattr[i].SIZE, ~0x99999999);
					free(src);
					return STPK_ret{ filesize, dst };
				}
			}
			return{ NULL,NULL };
		}
		bool init(string idxName);
	private:
		struct mIDX
		{
			DWORD HASH;
			DWORD OFFSET;
			DWORD SIZE;
		};
		bool inited;
		HANDLE STPK_HIDX;
		HANDLE STPK_HBIN;
		int STPK_filenum;
		mIDX* IDXattr;

		DWORD STPK_dec(char* src, char* dest, DWORD srcSize, DWORD key)
		{
			for (DWORD* i = (DWORD*)src; i < (DWORD*)src + srcSize / sizeof(DWORD); i++)
			{
				*i ^= key;

			}

			DWORD ORI_SIZE = NULL;
			memcpy(&ORI_SIZE, src, 4);
			DWORD roll = (srcSize - 4) / 0x40;
			for (unsigned int i = 0; i < roll; i++)
			{
				char* tmpDest = dest + i * 0x40;
				for (char* j = src + 4 + i * 0x40; j < src + 4 + 0x40 + i * 0x40; j += 2)
				{
					BYTE v1 = *(BYTE*)(j + 1);
					BYTE v2 = *(BYTE*)j;
					BYTE a1 = (v1 & 0xf0) + ((v2 & 0xf0) >> 4);
					BYTE a2 = ((v1 & 0xf) << 4) + (v2 & 0xf);

					*(BYTE*)tmpDest = a1;
					*(BYTE*)(tmpDest + 0x20) = a2;
					/*__asm
					{
						MOV EDX, j
						MOV AX, WORD ptr[EDX]		//0x00001234
							SHL EAX, 0x10			//0x12340000
							SHR EAX, 0xC			//0x00012340
							SHL AX, 4				//0x00013400
							SHL EAX, 4				//0x00134000
							SHR AX, 8				//0x00130040
							MOV EDX, j			
							INC EDX
							MOV AH, BYTE ptr[EDX]	//0x00131240
							SHL AX, 4				//0x00132400
							SHR EAX, 8				//0x00001324
							LEA EDX, tmpDest		
							MOV EDX, DWORD ptr[EDX]
							MOV Byte ptr[EDX], AH
							MOV Byte ptr[EDX + 0x20], AL
					}*/
					tmpDest++;
				}
			}


			return ORI_SIZE;
		}
	};

	stpkHandler::stpkHandler()
	{
		inited = false;
	}

	stpkHandler::~stpkHandler()
	{
	}

	bool stpkHandler::init(string idxName)
	{
		if (!inited)
		{
			string tpackName = idxName.substr(0, idxName.rfind('.'));
			string binName = tpackName + ".bin";

			STPK_HIDX = CreateFile(idxName.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
			STPK_HBIN = CreateFile(binName.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);


			DWORD IDXSIZE = GetFileSize(STPK_HIDX, NULL);
			STPK_filenum = IDXSIZE / 3 / sizeof(DWORD);

			DWORD rb;
			IDXattr = (mIDX*)malloc(sizeof(mIDX)*STPK_filenum);
			ReadFile(STPK_HIDX, IDXattr, IDXSIZE, &rb, NULL);
			CloseHandle(STPK_HIDX);
			inited = TRUE;

			return true;
		}
		return false;
	}

}