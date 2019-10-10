#include "PluginObjectFactory.h"
#include <OdsErr.h>
#include "DSource.h"

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

	if (!strcmp(szObjKey, PluginObjKey::HDA_ITEM_CITECT_VJH))
	{
		std::string key(szObjKey);
		*ppPluginObj = new CDSource(key);
		if (*ppPluginObj)
			return ODS::ERR::OK;
		else
			return ODS::ERR::MEMORY_ALLOCATION_ERR;
	}

	return ODS::ERR::BAD_PARAM;
}

int DrvFTSQLHdaItem::PluginObjectFactory::DestroyObject(ODS::IPluginObj* pPluginObj)
{
	delete pPluginObj;

	return ODS::ERR::OK;
}

std::string DrvFTSQLHdaItem::PluginObjectFactory::CreateRegisterInfo()
{

}

ODS::RegisterInfo* DrvFTSQLHdaItem::PluginObjectFactory::GetRegisterInfo(const std::string& objKey)
{

}