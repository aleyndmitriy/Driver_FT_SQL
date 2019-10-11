#include "pch.h"
#include"BrowserHdaItem.h"
#include <OdsErr.h>

void* DrvFTSQLHdaItem::BrowserHdaItem::GetInterface(int nIfcId)
{
	return this;
}

int DrvFTSQLHdaItem::BrowserHdaItem::Init(TCHAR* szCfgString)
{
	return ODS::ERR::OK;
}

int DrvFTSQLHdaItem::BrowserHdaItem::Shut()
{
	return ODS::ERR::OK;
}

int DrvFTSQLHdaItem::BrowserHdaItem::GetBrowseItemList(const ODS::ItemAddress* pAddress, ODS::BrowseItem** ppList, ULONG* pulCount)
{
	return ODS::ERR::OK;
}

int DrvFTSQLHdaItem::BrowserHdaItem::DestroyBrowseItemList(ODS::BrowseItem* pList, ULONG ulCount)
{
	return ODS::ERR::OK;
}