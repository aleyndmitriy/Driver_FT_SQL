#pragma once
#include "PropertySet.h"
#include <IPluginHost.h>
namespace DrvFTSQLHdaItem
{
	class CRegisterInfo
	{
	private:
		ODS::RegisterInfo m_RegInfo;

	public:
		CRegisterInfo();
		CRegisterInfo(const CRegisterInfo& src);
		~CRegisterInfo();
	};
}