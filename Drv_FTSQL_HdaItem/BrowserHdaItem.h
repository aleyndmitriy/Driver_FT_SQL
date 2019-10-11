#pragma once
#include<IBrowserItem.h>
#include<OdsString.h>

namespace DrvFTSQLHdaItem
{
	class BrowserHdaItem: public ODS::IBrowserItem
	{
	public:
		BrowserHdaItem() = default;
		void* GetInterface(int nIfcId) override;
		int Init(TCHAR* szCfgString) override;
		int Shut() override;
		int GetBrowseItemList(const ODS::ItemAddress* pAddress, ODS::BrowseItem** ppList, ULONG* pulCount) override;
		int DestroyBrowseItemList(ODS::BrowseItem* pList, ULONG ulCount) override;
	};
}