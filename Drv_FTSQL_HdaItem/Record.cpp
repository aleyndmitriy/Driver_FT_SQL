#include"pch.h"
#include"Record.h"

std::pair<typename DrvFTSQLHdaItem::Record::const_iterator, bool> DrvFTSQLHdaItem::Record::insert(std::string colName, short dataType, std::string value)
{
	typename DrvFTSQLHdaItem::Record::FieldData val = std::make_pair(dataType, value);
	std::pair<std::string, typename DrvFTSQLHdaItem::Record::FieldData> rec = std::make_pair(colName, val);
	return recordData.insert(rec);
}


const typename DrvFTSQLHdaItem::Record::FieldData& DrvFTSQLHdaItem::Record::at(const std::string& key) const
{
	return recordData.at(key);
}

typename DrvFTSQLHdaItem::Record::const_iterator DrvFTSQLHdaItem::Record::cbegin() const
{
	return recordData.cbegin();
}

typename DrvFTSQLHdaItem::Record::const_iterator DrvFTSQLHdaItem::Record::cend() const
{
	return recordData.cend();
}

typename DrvFTSQLHdaItem::Record::const_iterator DrvFTSQLHdaItem::Record::findColumnValue(const std::string& key) const
{
	return recordData.find(key);
}

size_t DrvFTSQLHdaItem::Record::GetColumnNumber() const
{
	return recordData.size();
}

DrvFTSQLHdaItem::Record::~Record()
{
	recordData.clear();
}

bool DrvFTSQLHdaItem::operator==(const DrvFTSQLHdaItem::Record& lhs, const DrvFTSQLHdaItem::Record& rhs)
{
	return lhs.recordData == rhs.recordData;
}

bool DrvFTSQLHdaItem::operator!=(const DrvFTSQLHdaItem::Record& lhs, const DrvFTSQLHdaItem::Record& rhs)
{
	return lhs.recordData != rhs.recordData;
}

bool DrvFTSQLHdaItem::operator<(const DrvFTSQLHdaItem::Record& lhs, const DrvFTSQLHdaItem::Record& rhs)
{
	return lhs.recordData < rhs.recordData;
}

bool DrvFTSQLHdaItem::operator<=(const DrvFTSQLHdaItem::Record& lhs, const DrvFTSQLHdaItem::Record& rhs)
{
	return lhs.recordData <= rhs.recordData;
}

bool DrvFTSQLHdaItem::operator>(const DrvFTSQLHdaItem::Record& lhs, const DrvFTSQLHdaItem::Record& rhs)
{
	return lhs.recordData > rhs.recordData;
}

bool DrvFTSQLHdaItem::operator>=(const DrvFTSQLHdaItem::Record& lhs, const DrvFTSQLHdaItem::Record& rhs)
{
	return lhs.recordData >= rhs.recordData;
}