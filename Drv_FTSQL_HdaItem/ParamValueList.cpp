#include"pch.h"
#include"ParamValueList.h"

DrvFTSQLHdaItem::Limit::Limit(int limitSide, int limitOffset, int limitCount) :m_nLimitSide(limitSide), m_nLimitOffset(limitOffset), m_nLimitCount(limitCount)
{

}

DrvFTSQLHdaItem::Limit::Limit() : Limit(0, 0, 0)
{

}

DrvFTSQLHdaItem::Limit::~Limit()
{

}

bool DrvFTSQLHdaItem::Limit::IsLimit() const
{
	return (m_nLimitOffset != 0 && m_nLimitCount != 0);
}

DrvFTSQLHdaItem::ParamValueList::ParamValueList(std::string&& address, std::string&& fullAddress, std::string&& sql, bool prevPoint, bool postPoint, int valueType, Limit&& limit):
	ParamValue(std::move(address),std::move(fullAddress),std::move(sql),prevPoint,postPoint),m_ValueType(valueType),m_Limit(std::move(limit))
{

}

DrvFTSQLHdaItem::ParamValueList::~ParamValueList() {

}

DrvFTSQLHdaItem::Limit DrvFTSQLHdaItem::ParamValueList::GetLimit() const
{
	return m_Limit;
}

int DrvFTSQLHdaItem::ParamValueList::GetValueType() const
{
	return m_ValueType;
}