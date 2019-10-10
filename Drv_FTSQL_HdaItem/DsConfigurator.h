#pragma once
#include <IDsConfigurator.h>
#include <Interface\IAbstractUIFacrory.h>
#include <functional>

namespace DrvFTSQLHdaItem
{
	class CDsConfigurator : public ODS::IDsConfigurator
	{
		std::function<ODS::UI::IAbstractUIFacrory * (void)>  m_uiFactoryGetter;
	public:

		CDsConfigurator(std::function<ODS::UI::IAbstractUIFacrory * (void)> uiFactoryGetter);
		~CDsConfigurator() = default;

		void* GetInterface(int nIfcId) override;

		int Configure(const TCHAR* szCfgInString, TCHAR** pszCfgOutString) override;

		int DestroyString(TCHAR* szCfgString) override;

		void SetOwnerWnd(void* pOwnerWnd) override;

	};
}