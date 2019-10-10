#pragma once
#include<IPluginObjectFactory.h>
#include<OdsErr.h>
#include<string>
#include<map>
#include"RegisterInfo.h"
namespace DrvFTSQLHdaItem
{
	class PluginObjectFactory : public ODS::IPluginObjectFactory
	{
	public:
		PluginObjectFactory(const PluginObjectFactory& src) = delete;
		PluginObjectFactory& operator = (const PluginObjectFactory& rhs) = delete;
		PluginObjectFactory(PluginObjectFactory&& src) = delete;
		PluginObjectFactory& operator = (PluginObjectFactory&& rhs) = delete;
		~PluginObjectFactory() = default;
		static PluginObjectFactory& GetInstance();
		void* GetInterface(int nIfcId) override;
		int CreateObject(const TCHAR* szObjKey, void* pObjCreationParam, ODS::IPluginObj** ppPluginObj) override;
		int DestroyObject(ODS::IPluginObj* pPluginObj) override;
		std::string CreateRegisterInfo();
		ODS::RegisterInfo* GetRegisterInfo(const std::string& objKey);
	private:
		PluginObjectFactory() = default;
		std::map<std::string, CRegisterInfo> regInfoDSList;
	};
}