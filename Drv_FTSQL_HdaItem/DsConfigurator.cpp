#include"pch.h"
#include "DsConfigurator.h"
#include "OdsErr.h"
#include "OdsString.h"

DrvFTSQLHdaItem::CDsConfigurator::CDsConfigurator(std::function<ODS::UI::IAbstractUIFacrory * (void)> uiFactoryGetter) :m_uiFactoryGetter(uiFactoryGetter)
{

}

void* DrvFTSQLHdaItem::CDsConfigurator::GetInterface(int nIfcId)
{
	return NULL;
}

int DrvFTSQLHdaItem::CDsConfigurator::Configure(const TCHAR* szCfgInString, TCHAR** pszCfgOutString)
{
	int iRes = ODS::ERR::OK;

	return iRes;
}

int DrvFTSQLHdaItem::CDsConfigurator::DestroyString(TCHAR* szCfgString)
{
	delete[] szCfgString;

	return ODS::ERR::OK;
}

void DrvFTSQLHdaItem::CDsConfigurator::SetOwnerWnd(void* pOwnerWnd)
{

}