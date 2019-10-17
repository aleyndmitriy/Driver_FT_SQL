#include"pch.h"
#include"ParamValue.h"

DrvFTSQLHdaItem::ParamValue::ParamValue(std::string&& address, std::string&& fullAddress, std::string&& sql, bool prevPoint, bool postPoint):
	m_Address(address),m_FullAddress(fullAddress),m_Sql(sql),m_bPrevPoint(prevPoint), m_bPostPoint(postPoint)
{

}

DrvFTSQLHdaItem::ParamValue::~ParamValue() {

}

std::string DrvFTSQLHdaItem::ParamValue::GetAddress() const
{
	return m_Address;
}

std::string DrvFTSQLHdaItem::ParamValue::GetFullAddress() const
{
	return m_FullAddress;
}

std::string DrvFTSQLHdaItem::ParamValue::GetSqc() const
{
	return m_Sql;
}

bool DrvFTSQLHdaItem::ParamValue::HasPrevPoint() const
{
	return m_bPrevPoint;
}

bool DrvFTSQLHdaItem::ParamValue::HasPostPoint() const
{
	return m_bPostPoint;
}

bool DrvFTSQLHdaItem::ParamValue::HasSql() const
{
	return !m_Sql.empty();
}
