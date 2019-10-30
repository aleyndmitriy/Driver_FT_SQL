#include"pch.h"
#include"HdaFunctionResultValueList.h"
#include<Tvq.h>

DrvFTSQLHdaItem::HdaFunctionResultValueList::HdaFunctionResultValueList()
{

}

DrvFTSQLHdaItem::HdaFunctionResultValueList::~HdaFunctionResultValueList()
{

}

void* DrvFTSQLHdaItem::HdaFunctionResultValueList::GetContext() const
{
	return ODS::HdaFunctionResultValueList::GetContext();
}

void DrvFTSQLHdaItem::HdaFunctionResultValueList::SetContext(void* pContext)
{
	ODS::HdaFunctionResultValueList::SetContext(pContext);
}

int DrvFTSQLHdaItem::HdaFunctionResultValueList::SetTvqList(ODS::Tvq** pList, int nCount)
{
	tvqArray.clear();
	for (int ind = 0; ind < nCount; ++ind) {
		tvqArray.push_back(**(pList + ind));
	}
	return nCount;
}

int DrvFTSQLHdaItem::HdaFunctionResultValueList::GetTvqList(ODS::Tvq*** pppList, int* pnCount) const
{
	if (tvqArray.empty()) {
		pppList = nullptr;
		*pnCount = 0;
		return 0;
	}
	*pppList = new ODS::Tvq*;
	**pppList = new ODS::Tvq[tvqArray.size()];
	for (int ind = 0; ind < tvqArray.size(); ind++) {
		*(**pppList + ind) = tvqArray.at(ind);
	}
	*pnCount = tvqArray.size();
	return *pnCount;
}

void DrvFTSQLHdaItem::HdaFunctionResultValueList::AddTvqList(ODS::Tvq** pList, int nCount)
{
	for (int ind = 0; ind < nCount; ++ind) {
		tvqArray.push_back(**(pList + ind));
	}
}

void DrvFTSQLHdaItem::HdaFunctionResultValueList::AddTvq(ODS::Tvq* pTvq)
{
	tvqArray.push_back(*pTvq);
}

int DrvFTSQLHdaItem::HdaFunctionResultValueList::GetTvqListCount() const
{
	return tvqArray.size();
}

const ODS::Tvq* DrvFTSQLHdaItem::HdaFunctionResultValueList::GetTvq(int nlIndex) const
{
	if (nlIndex < tvqArray.size() && nlIndex > -1) {
		return &tvqArray.at(nlIndex);
	}
	else {
		return nullptr;
	}
}

int DrvFTSQLHdaItem::HdaFunctionResultValueList::DestroyTvqList(ODS::Tvq** pList)
{
	delete [] pList;
	delete *pList;
	*pList = 0;
	return 0;
}

void DrvFTSQLHdaItem::HdaFunctionResultValueList::AddTvqToArray(const ODS::Tvq& tvq)
{
	tvqArray.push_back(tvq);
}