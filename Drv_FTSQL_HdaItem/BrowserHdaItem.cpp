#include "pch.h"
#include"BrowserHdaItem.h"
#include <OdsErr.h>
#include <AddressComponent.h>
#include <Address.h>
#include"XMLSettingsDataSource.h"



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
			settingSource.LoadAttributesString(szCfgString, len, attributes);
		}
	}
	
	return ODS::ERR::OK;
}

int DrvFTSQLHdaItem::BrowserHdaItem::Shut()
{
	return ODS::ERR::OK;
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
	return ODS::ERR::OK;
}

int DrvFTSQLHdaItem::BrowserHdaItem::GetNextTagLevel(SQLHANDLE hConn, int nItemID, int nLevel, std::vector<ODS::OdsString>& rEntry, std::vector<STagItem>* pTagList)
{
	return ODS::ERR::OK;
}

int DrvFTSQLHdaItem::BrowserHdaItem::GetItemTagList(SQLHANDLE hConn, int nItemID, std::vector<ODS::OdsString>& rEntry, std::vector<STagItem>* pTagList)
{
	return ODS::ERR::OK;
}