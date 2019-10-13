#include"pch.h"
#include <sstream>
#include "DsConfigurator.h"
#include "OdsErr.h"
#include "OdsString.h"
#include"ConnectionAttributes.h"
#include"XMLSettingsDataSource.h"
#include "DlgConnectionSqlSvr.h"

DrvFTSQLHdaItem::CDsConfigurator::CDsConfigurator(std::function<ODS::UI::IAbstractUIFacrory * (void)> uiFactoryGetter) :m_uiFactoryGetter(uiFactoryGetter), m_hParentWindow(nullptr)
{

}

void* DrvFTSQLHdaItem::CDsConfigurator::GetInterface(int nIfcId)
{
	return NULL;
}

int DrvFTSQLHdaItem::CDsConfigurator::Configure(const TCHAR* szCfgInString, TCHAR** pszCfgOutString)
{
	int iRes = ODS::ERR::FILE;
	std::shared_ptr<ConnectionAttributes> attributes = std::make_shared<ConnectionAttributes>();
	XMLSettingsDataSource settingSource;
	if (szCfgInString != NULL)
	{
		size_t len = _tcslen(szCfgInString);
		if (len > 0) {
			settingSource.LoadAttributesString(szCfgInString, len, *attributes);
		}
	}
	HINSTANCE hOld = AfxGetResourceHandle();
	HMODULE hModule = GetModuleHandle(TEXT("Drv_FTSQL_HdaItem.dll"));
	AfxSetResourceHandle(hModule);
	CWnd* parent = CWnd::FromHandle(m_hParentWindow);
	DlgConnectionSqlSvr dlg(m_uiFactoryGetter, attributes, parent);
	int response = dlg.DoModal();
	if (response == IDOK) {
		std::ostringstream outStream;
		settingSource.SaveAttributesString(*attributes, outStream);
		std::string outString = outStream.str();
		char* outStr = new char[outString.length() + 1];
		_tcscpy_s(outStr, outString.length() + 1, outString.c_str());
		*pszCfgOutString = outStr;
		iRes = ODS::ERR::OK;
	}
	else {
		if (szCfgInString != NULL)
		{
			size_t len = _tcslen(szCfgInString);
			char* outStr = new char[len + 1];
			_tcscpy_s(outStr, len + 1, szCfgInString);
			*pszCfgOutString = outStr;
			iRes = ODS::ERR::OK;
		}
	}
	return iRes;
}

int DrvFTSQLHdaItem::CDsConfigurator::DestroyString(TCHAR* szCfgString)
{
	delete[] szCfgString;

	return ODS::ERR::OK;
}

void DrvFTSQLHdaItem::CDsConfigurator::SetOwnerWnd(void* pOwnerWnd)
{
	m_hParentWindow = reinterpret_cast<HWND>(pOwnerWnd);
}