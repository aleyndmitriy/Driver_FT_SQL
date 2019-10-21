#include "pch.h"
#include"Constants.h"
#include"BrowserHdaItem.h"
#include <OdsErr.h>
#include <AddressComponent.h>
#include <Address.h>
#include<AddressHelper.h>
#include"XMLSettingsDataSource.h"


DrvFTSQLHdaItem::BrowserHdaItem::BrowserHdaItem():m_database(nullptr),m_attributes(),m_TagList()
{

}

void* DrvFTSQLHdaItem::BrowserHdaItem::GetInterface(int nIfcId)
{
	return this;
}

int DrvFTSQLHdaItem::BrowserHdaItem::Init(TCHAR* szCfgString)
{
	XMLSettingsDataSource settingSource;
	if (szCfgString != NULL)
	{
		size_t len = _tcslen(szCfgString);
		if (len > 0) {
			settingSource.LoadAttributesString(szCfgString, len, m_attributes);
		}
	}
	m_database = std::make_unique<SQLServerTagRecordsDAO>(m_attributes, TAG_TABLE_NAME, TAG_FLOAT_VALUE_TABLE_NAME, TAG_STRING_VALUE_TABLE_NAME);

	return ODS::ERR::OK;
}

int DrvFTSQLHdaItem::BrowserHdaItem::Shut()
{
	m_database.reset();
	return ODS::ERR::OK;
}

ODS::OdsString DrvFTSQLHdaItem::BrowserHdaItem::GetAddressOld(const ODS::ItemAddress& rAddress)
{
	ODS::AddressHelper ah(&rAddress);

	int nIndex = 0;
	std::vector<std::pair<ODS::OdsString, ODS::OdsString>> listAddrCmp;
	ah.GetAddressAsList(&listAddrCmp, &nIndex);

	for (size_t i = 0; i < listAddrCmp.size(); i++)
	{
		if (listAddrCmp[i].first == _T("address_old"))
			return listAddrCmp[i].second;
	}

	return _T("");
}


int DrvFTSQLHdaItem::BrowserHdaItem::GetBrowseItemList(const ODS::ItemAddress* pAddress, ODS::BrowseItem** ppList, ULONG* pulCount)
{
	int iRes = ODS::ERR::OK;

	if (ppList && pulCount)
	{
		std::vector<STagItem> tagList;

		int nCount = 0;
		ODS::AddressComponent* pAddrComponent = NULL;
		std::vector<ODS::OdsString> entry;

		if (pAddress)
		{
			TCHAR* szName = NULL;
			TCHAR* szValue = NULL;
			int nIndex = 0;

			ODS::ItemAddress addr;

			ODS::ItemAddress* pAddr = (ODS::ItemAddress*)pAddress;

			ODS::OdsString addressOld = GetAddressOld(*pAddress);
			if (!addressOld.IsEmpty())
			{
				addr.SetPlainAddress((LPCTSTR)addressOld);
				pAddr = &addr;
			}
			
			pAddr->GetAddress(&pAddrComponent, &nCount, &nIndex);

			for (int ind = 0; ind < nCount; ind++)
			{
				pAddrComponent[ind].GetName(&szName);
				pAddrComponent[ind].GetValue(&szValue);

				if (szValue && szValue[0])
					entry.push_back(szValue);

				pAddrComponent[ind].DestroyString(szName);
				pAddrComponent[ind].DestroyString(szValue);
			}
		}

		iRes = GetTagList(entry, &tagList);
		if (iRes != ODS::ERR::OK)
			return iRes;

		*pulCount = tagList.size();

		if (*pulCount)
		{
			*ppList = new ODS::BrowseItem[*pulCount];
			if (*ppList)
			{
				for (UINT ind = 0; ind < tagList.size(); ind++)
				{
					ODS::BrowseItem* pItem = *ppList + ind;

					ODS::ItemAddress addr;
					UINT uiComponentCount = tagList[ind].m_vAddress.size();
					ODS::AddressComponent* pAddrComponent = new ODS::AddressComponent[uiComponentCount];

					if (pAddrComponent)
					{
						for (UINT i = 0; i < uiComponentCount; i++)
						{
							pAddrComponent[i].SetName((LPCTSTR)tagList[ind].m_vAddress[i]);

							pAddrComponent[i].SetValue((LPCTSTR)tagList[ind].m_vAddress[i]);
						}

						addr.SetAddress(pAddrComponent, uiComponentCount, 0);

						delete[] pAddrComponent;
					}

					pItem->SetName((LPCTSTR)tagList[ind].m_vAddress[tagList[ind].m_vAddress.size() - 1]);
					pItem->SetAddress(addr);
					pItem->SetDescription((LPCTSTR)tagList[ind].m_szDescription);

					pItem->SetHierarchyType(tagList[ind].m_nType);
				}
			}
		}
	}
	else
	{
		iRes = ODS::ERR::BAD_PARAM;
	}

	return iRes;
	return ODS::ERR::OK;
}

int DrvFTSQLHdaItem::BrowserHdaItem::DestroyBrowseItemList(ODS::BrowseItem* pList, ULONG ulCount)
{
	if (pList)
	{
		delete[] pList;
	}
	return ODS::ERR::OK;
}

int DrvFTSQLHdaItem::BrowserHdaItem::GetTagList(std::vector<ODS::OdsString>& rEntry, std::vector<STagItem>* pTagList)
{
	pTagList->clear();
	std::map<std::string, TagItemRecord> tags = m_database->GetTags();
	for (std::map<std::string, TagItemRecord>::const_iterator itr = tags.cbegin(); itr != tags.cend(); ++itr) {
		STagItem item;
		item.m_vAddress.push_back(ODS::OdsString(itr->second.GetTegName().c_str())); 
		item.m_szDescription = ODS::OdsString(itr->second.GetTegName().c_str());
		pTagList->push_back(item);
	}

	return ODS::ERR::OK;
}

