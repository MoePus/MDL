/*
CO			THE COMPOSITION MODULE OF MDL
MP					2016@MoePus
*/
#pragma once
namespace MDL
{
	class composition
	{
	public:
		static composition* getSingleton()
		{
			static composition* singleton = new composition;
			return singleton;
		}

	};

}