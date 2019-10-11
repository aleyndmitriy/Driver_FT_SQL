#include "pch.h"
#include "PluginObjectFactory.h"
#include <OdsErr.h>
#include "DSource.h"
#include"Constants.h"

DrvFTSQLHdaItem::PluginObjectFactory& DrvFTSQLHdaItem::PluginObjectFactory::GetInstance()
{
	static PluginObjectFactory f;
	return f;
}

void* DrvFTSQLHdaItem::PluginObjectFactory::GetInterface(int nIfcId)
{
	return this;
}

int DrvFTSQLHdaItem::PluginObjectFactory::CreateObject(const TCHAR* szObjKey, void* pObjCreationParam, ODS::IPluginObj** ppPluginObj)
{
	*ppPluginObj = 0;
	if (_tcscmp(szObjKey, HDA_ITEM_FTSQL_VJH)) {
		return ODS::ERR::BAD_PARAM;
	}
	*ppPluginObj = new CDSource();
	if (*ppPluginObj)
		return ODS::ERR::OK;
	else
		return ODS::ERR::MEMORY_ALLOCATION_ERR;
}

int DrvFTSQLHdaItem::PluginObjectFactory::DestroyObject(ODS::IPluginObj* pPluginObj)
{
	delete pPluginObj;

	return ODS::ERR::OK;
}

void DrvFTSQLHdaItem::PluginObjectFactory::CreateRegisterInfo()
{
	regInfoDSList = std::make_unique<CRegisterInfo>(VERSION_MINOR, VERSION_MAJOR, HDA_ITEM_FTSQL_VJH, nullptr, this);
}

ODS::RegisterInfo* DrvFTSQLHdaItem::PluginObjectFactory::GetRegisterInfo()
{
	if (regInfoDSList) {
		return &(regInfoDSList->m_RegInfo);
	}
	else {
		return nullptr;
	}
	
}