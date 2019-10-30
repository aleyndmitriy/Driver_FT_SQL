#pragma once
#include <HdaFunctionResult.h>
#include<vector>

namespace DrvFTSQLHdaItem
{
	class HdaFunctionResultValueList : public ODS::HdaFunctionResultValueList
	{
	public:
		HdaFunctionResultValueList();
		~HdaFunctionResultValueList();
		HdaFunctionResultValueList(const HdaFunctionResultValueList& rOther) = default;
		HdaFunctionResultValueList& operator = (const HdaFunctionResultValueList& rOther) = default;
		virtual void* GetContext() const override;
		virtual void SetContext(void* pContext) override;
		int SetTvqList(ODS::Tvq** pList, int nCount);
		int GetTvqList(ODS::Tvq*** pppList, int* pnCount) const;
		void AddTvqList(ODS::Tvq** pList, int nCount);
		void AddTvq(ODS::Tvq* pTvq);
		void AddTvqToArray(const ODS::Tvq& tvq);
		int GetTvqListCount() const;
		const ODS::Tvq* GetTvq(int nlIndex) const;
		static int DestroyTvqList(ODS::Tvq** pList);
	private:
		std::vector<ODS::Tvq> tvqArray;
	};
}