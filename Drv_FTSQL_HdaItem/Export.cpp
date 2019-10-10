#include "Export.h"
#include"PluginObjectFactory.h"

int InitPlugin(void* pParam, ODS::IPluginHost* pPluginHost)
{
	if (pParam == nullptr || pPluginHost == nullptr) {
		return ODS::ERR::INIT_ERR;
	}
	std::string objKey = DrvFTSQLHdaItem::PluginObjectFactory::GetInstance().CreateRegisterInfo();
	if (objKey.empty()) {
		return ODS::ERR::INIT_ERR;
	}
	else {
		pPluginHost->RegisterObject(pParam, DrvFTSQLHdaItem::PluginObjectFactory::GetInstance().GetRegisterInfo(objKey));
		return ODS::ERR::OK;
	}
}