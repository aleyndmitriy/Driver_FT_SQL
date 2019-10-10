#pragma once
#include <ISourceHdaItem.h>
#include<IPluginHost.h>
#include "DsConfigurator.h"

namespace DrvFTSQLHdaItem
{
	class CDSource : public ODS::ISourceHdaItem
	{
	public:
		CDSource(const std::string& key);
		~CDSource();
		CDSource(const CDSource& src) = delete;
		CDSource& operator=(const CDSource& src) = delete;
		CDSource(CDSource&& src) = delete;
		CDSource& operator=(CDSource&& src) = delete;
		void* GetInterface(int nIfcId) override;
		ODS::IDsConfigurator* GetConfigurator() override;
		ODS::IServerHda* GetServer() override;
		ODS::IBrowserItem* GetBrowser() override;
		int Attach(const ODS::IPluginHost* pHost) override;
		int Detach() override;
		ODS::IPropertySet* GetPropertySet() override;
	private:
		std::string objKey;
		CDsConfigurator m_pConfigurator;
		ODS::IPluginHost* m_pHost;
	};
}