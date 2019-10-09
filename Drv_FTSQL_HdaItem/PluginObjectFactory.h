#pragma once
#include<IPluginObjectFactory.h>
#include<OdsErr.h>
#include<string>
#include<map>
#include<memory>
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
		bool InsertRegisterInfo(std::string name, std::unique_ptr<ODS::RegisterInfo>&& reg);
	private:
		PluginObjectFactory() = default;
		std::map<std::string, std::unique_ptr<ODS::RegisterInfo> > regInfoDSList;
	};
}