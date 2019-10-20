#include"pch.h"
#include"SQLTable.h"


DrvFTSQLHdaItem::SQLTable::SQLTable(const std::string& tableName, const std::string& schemaName) :_tableName(tableName), _schemaName(schemaName), columns(std::map<std::string, std::string>{})
{

}

DrvFTSQLHdaItem::SQLTable::SQLTable(const std::string& tableName) : SQLTable(tableName, std::string())
{

}

DrvFTSQLHdaItem::SQLTable::~SQLTable()
{
	_tableName.clear();
	_schemaName.clear();
	columns.clear();
}

std::string DrvFTSQLHdaItem::SQLTable::GetTableName() const
{
	return _tableName;
}

std::string DrvFTSQLHdaItem::SQLTable::GetSchemaName() const
{
	return _schemaName;
}

std::string DrvFTSQLHdaItem::SQLTable::GetFullName() const
{
	if (_schemaName.empty()) {
		return _tableName;
	}
	else {
		return std::string(_schemaName + std::string(".") + _tableName);
	}
}

void DrvFTSQLHdaItem::SQLTable::SetSchemaName(std::string schemaName)
{
	_schemaName = schemaName;
}

bool DrvFTSQLHdaItem::SQLTable::InsertColumn(const std::string& columnName, const std::string& columnType)
{
	std::pair<std::string, std::string> val = std::make_pair<std::string, std::string>(std::string(columnName), std::string(columnType));
	std::pair<std::map<std::string, std::string>::iterator, bool> res = columns.insert(val);
	return res.second;
}

DrvFTSQLHdaItem::SQLTable::const_iterator DrvFTSQLHdaItem::SQLTable::cbegin() const
{
	return columns.cbegin();
}

DrvFTSQLHdaItem::SQLTable::const_iterator DrvFTSQLHdaItem::SQLTable::cend() const
{
	return columns.cend();
}

std::string& DrvFTSQLHdaItem::SQLTable::at(std::string columnName)
{
	return columns.at(columnName);
}

bool DrvFTSQLHdaItem::operator==(const DrvFTSQLHdaItem::SQLTable& lhs, const DrvFTSQLHdaItem::SQLTable& rhs)
{
	return lhs.GetFullName() == rhs.GetFullName();
}

bool DrvFTSQLHdaItem::operator!=(const DrvFTSQLHdaItem::SQLTable& lhs, const DrvFTSQLHdaItem::SQLTable& rhs)
{
	return lhs.GetFullName() != rhs.GetFullName();
}

bool DrvFTSQLHdaItem::operator<(const DrvFTSQLHdaItem::SQLTable& lhs, const DrvFTSQLHdaItem::SQLTable& rhs)
{
	return lhs.GetFullName() < rhs.GetFullName();
}

bool DrvFTSQLHdaItem::operator<=(const DrvFTSQLHdaItem::SQLTable& lhs, const DrvFTSQLHdaItem::SQLTable& rhs)
{
	return lhs.GetFullName() <= rhs.GetFullName();
}

bool DrvFTSQLHdaItem::operator>(const DrvFTSQLHdaItem::SQLTable& lhs, const DrvFTSQLHdaItem::SQLTable& rhs)
{
	return lhs.GetFullName() > rhs.GetFullName();
}

bool DrvFTSQLHdaItem::operator>=(const DrvFTSQLHdaItem::SQLTable& lhs, const DrvFTSQLHdaItem::SQLTable& rhs)
{
	return lhs.GetFullName() >= rhs.GetFullName();
}