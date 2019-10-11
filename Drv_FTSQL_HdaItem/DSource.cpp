#include "pch.h"
#include "DSource.h"
#include "PluginObjectFactory.h"

DrvFTSQLHdaItem::CDSource::CDSource() : m_pConfigurator([this]()->ODS::UI::IAbstractUIFacrory* {
	if (m_pHost)
		return (ODS::UI::IAbstractUIFacrory*) (m_pHost->GetInterface(ODS::IPluginHost::IID_UI_FACTORY));
	return 	nullptr;
	}), m_pHost(nullptr)
{

}

	DrvFTSQLHdaItem::CDSource::~CDSource()
	{
		m_pHost = nullptr;
	}

	void* DrvFTSQLHdaItem::CDSource::GetInterface(int nIfcID)
	{
		return this;
	}

	ODS::IDsConfigurator* DrvFTSQLHdaItem::CDSource::GetConfigurator()
	{
		return &m_pConfigurator;
	}

	ODS::IServerHda* DrvFTSQLHdaItem::CDSource::GetServer()
	{
		return NULL;
	}

	ODS::IBrowserItem* DrvFTSQLHdaItem::CDSource::GetBrowser()
	{
		return NULL;
	}

	int DrvFTSQLHdaItem::CDSource::Attach(const ODS::IPluginHost* pHost)
	{
		if (!pHost)
			return ODS::ERR::BAD_PARAM;

		m_pHost = const_cast<ODS::IPluginHost*>(pHost);
		return ODS::ERR::OK;
	}

	int DrvFTSQLHdaItem::CDSource::Detach()
	{
		m_pHost = 0;
		return ODS::ERR::OK;
	}

	ODS::IPropertySet* DrvFTSQLHdaItem::CDSource::GetPropertySet()
	{
		ODS::RegisterInfo* pInfo = PluginObjectFactory::GetInstance().GetRegisterInfo();
		return pInfo->m_pPropertySet;
	}