#include"pch.h"
#include"SQLServerTagRecordsDAO.h"
#include"SQLServerDatabaseEngine.h"
#include"Log.h"
#include"Constants.h"
#include"Utils.h"

DrvFTSQLHdaItem::SQLServerTagRecordsDAO::SQLServerTagRecordsDAO(const ConnectionAttributes& attributes, const std::string& tagTable, const std::string& floatValueTable, const std::string& stringValueTable):
	m_attributes(attributes), m_tagTableName(tagTable),m_floatValueTableName(floatValueTable),m_stringValueTableName(stringValueTable),m_dataConnectionsList()
{

}

DrvFTSQLHdaItem::SQLServerTagRecordsDAO::~SQLServerTagRecordsDAO()
{
	for (std::map<std::string, std::unique_ptr<IDatabaseEngine> >::iterator itr = m_dataConnectionsList.begin(); itr != m_dataConnectionsList.end(); itr++) {
		itr->second->CloseConnection();
		itr->second.reset();
	}
}

std::unique_ptr<DrvFTSQLHdaItem::SQLTable> DrvFTSQLHdaItem::SQLServerTagRecordsDAO::GetTableInfo(const std::string& connectionID, std::string tableName)
{
	std::string querry = std::string("SELECT COLUMN_NAME, DATA_TYPE, TABLE_SCHEMA FROM Information_schema.Columns WHERE TABLE_NAME = '") + tableName + std::string("';");
	std::vector<std::string> vec = { };
	Log::GetInstance()->WriteInfo(_T("Table info querry : % s ."), (LPCTSTR)querry.c_str());
	std::map<std::string, std::unique_ptr<IDatabaseEngine> >::iterator itr = m_dataConnectionsList.find(connectionID);
	if (itr == m_dataConnectionsList.end()) {
		return nullptr;
	}
	std::vector<Record> records = itr->second->ExecuteStatement(querry, vec);
	if (records.empty()) {
		Log::GetInstance()->WriteInfo(_T("Table has no any columns"));
		return nullptr;
	}
	Log::GetInstance()->WriteInfo(_T("columns number : %d ."), records.size());
	std::unique_ptr<SQLTable> ptrData = std::make_unique<SQLTable>(tableName);
	for (std::vector<Record>::const_iterator itr = records.cbegin(); itr != records.cend(); ++itr) {
		Record::const_iterator recordItrColName = itr->findColumnValue("COLUMN_NAME");
		Record::const_iterator recordItrColType = itr->findColumnValue("DATA_TYPE");
		Record::const_iterator recordItrColSchema = itr->findColumnValue("TABLE_SCHEMA");
		if (recordItrColName != itr->cend() && recordItrColType != itr->cend() && recordItrColSchema != itr->cend()) {
			ptrData->InsertColumn(recordItrColName->second.second, recordItrColType->second.second);
			ptrData->SetSchemaName(recordItrColSchema->second.second);
		}
	}
	return ptrData;
}


std::map<std::string, DrvFTSQLHdaItem::TagItemRecord> DrvFTSQLHdaItem::SQLServerTagRecordsDAO::GetTags()
{
	std::map<std::string, TagItemRecord> tags;
	std::string key = OpenConnection();
	if (key.empty()) {
		return tags;
	}
	std::unique_ptr<SQLTable> table = GetTableInfo(key, m_tagTableName);
	std::string sql = std::string("SELECT ");
	for (SQLTable::const_iterator itr = table->cbegin(); itr != table->cend(); ++itr) {
		sql += std::string(" ") + table->GetFullName() + std::string(".") + itr->first + std::string(", ");
	}
	sql.erase(sql.size() - 2, 2);
	sql += std::string(" FROM ") + table->GetFullName();
	Log::GetInstance()->WriteInfo(_T("All Tag SQL Query : % s ."), (LPCTSTR)sql.c_str());
	std::vector<Record> records;
	std::vector<std::string> params = { };
	std::map<std::string, std::unique_ptr<IDatabaseEngine> >::iterator itr = m_dataConnectionsList.find(key);
	if (itr == m_dataConnectionsList.end()) {
		return tags;
	}
	records = itr->second->ExecuteStatement(sql, params);
	for (std::vector<Record>::const_iterator itr = records.cbegin(); itr != records.cend(); ++itr) {
		Record::const_iterator recordItrTagName = itr->findColumnValue(TAG_TABLE_COLUMN_TAG_NAME);
		Record::const_iterator recordItrTagIndex = itr->findColumnValue(TAG_TABLE_COLUMN_TAG_INDEX);
		Record::const_iterator recordItrTagType = itr->findColumnValue(TAG_TABLE_COLUMN_TAG_TYPE);
		Record::const_iterator recordItrTagDataType = itr->findColumnValue(TAG_TABLE_COLUMN_TAG_DATA_TYPE);
		if (recordItrTagName != itr->cend() && recordItrTagIndex != itr->cend() && recordItrTagType != itr->cend() && recordItrTagDataType != itr->cend()) {
			short tagIndex = std::stoi(recordItrTagIndex->second.second);
			short tagType = std::stoi(recordItrTagType->second.second);
			short tagDataType = std::stoi(recordItrTagDataType->second.second);
			std::pair<std::string, TagItemRecord> pair = std::make_pair<std::string, TagItemRecord>(std::string(recordItrTagName->second.second), TagItemRecord(recordItrTagName->second.second,tagIndex,tagType,tagDataType));
			tags.insert(pair);
		}
	}
	CloseConnectionWithUUID(key);
	return tags;
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::OpenConnection()
{
	GUID guid;
	if (CoCreateGuid(&guid) != S_OK) {
		return std::string();
	}
	WCHAR strGuide[MAX_UUID_LENGTH];
	int res = StringFromGUID2(guid, strGuide, MAX_UUID_LENGTH);
	std::string key = std::string(Wstr2Str(std::wstring(strGuide)));
	if (OpenConnectionWithUUID(key)) {
		return key;
	}
	else {
		return std::string();
	}
}

bool DrvFTSQLHdaItem::SQLServerTagRecordsDAO::OpenConnectionWithUUID(const std::string& connectionID)
{
	m_dataConnectionsList.insert(std::make_pair<std::string, std::unique_ptr<IDatabaseEngine> >(std::string(connectionID), std::make_unique<SQLServerDatabaseEngine>()));
	if (!m_dataConnectionsList[connectionID]->CreateDirectConnectionToDatabase(m_attributes)) {
		Log::GetInstance()->WriteInfo(_T("Can't connect to database"));
		m_dataConnectionsList.erase(connectionID);
		return false;
	}
	else {
		return true;
	}
}

void DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CloseConnectionWithUUID(const std::string& connectionID)
{
	std::map<std::string, std::unique_ptr<IDatabaseEngine> >::iterator itr = m_dataConnectionsList.find(connectionID);
	if (itr != m_dataConnectionsList.end()) {
		itr->second->CloseConnection();
		itr->second.reset();
		m_dataConnectionsList.erase(itr);
	}
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementValueList(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime)
{
	/*SELECT FloatTable.Val, DATEADD(millisecond,FloatTable.Millitm,FloatTable.DateAndTime) AS 'DateTime' FROM FloatTable INNER JOIN TagTable ON FloatTable.TagIndex = TagTable.TagIndex  
	WHERE TagTable.TagName = '[PLC_CP]PT3_1013.Val' AND DATEADD(millisecond,FloatTable.Millitm,FloatTable.DateAndTime) > DATEADD(day,-2, '2019-05-29') AND
DATEADD(millisecond,FloatTable.Millitm,FloatTable.DateAndTime) < (SELECT MIN(DATEADD(millisecond,FloatTable.Millitm,FloatTable.DateAndTime)) 
FROM FloatTable WHERE DATEADD(millisecond,FloatTable.Millitm,FloatTable.DateAndTime) > '2019-06-16') ORDER BY FloatTable.DateAndTime DESC OFFSET 50 ROWS FETCH NEXT 25 ROWS ONLY*/
	std::string date = std::string("DATEADD(millisecond,") + std::string("%s.") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(",%s.") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(")");
	std::string startDate = std::to_string(startTime.wYear) + std::string("-") + std::to_string(startTime.wMonth) + std::string("-") +
		std::to_string(startTime.wDay) + std::string(" ") + std::to_string(startTime.wHour) + std::string(":") +
		std::to_string(startTime.wMinute) + std::string(":") + std::to_string(startTime.wSecond);
	std::string endDate = std::to_string(endTime.wYear) + std::string("-") + std::to_string(endTime.wMonth) + std::string("-") +
		std::to_string(endTime.wDay) + std::string(" ") + std::to_string(endTime.wHour) + std::string(":") +
		std::to_string(endTime.wMinute) + std::string(":") + std::to_string(endTime.wSecond);

	std::string query = std::string("SELECT ") + std::string("%s") + std::string(".") + std::string(TAG_TABLE_COLUMN_VALUE) + date + std::string(" AS ") + std::string(TAG_TABLE_COLUMN_DATE_TIME_MILLISEC) +
		std::string("FROM %s INNER JOIN ") + std::string(TAG_TABLE_NAME) + std::string("ON %s.") + std::string(TAG_TABLE_COLUMN_TAG_INDEX) + std::string(" = ") + std::string(TAG_TABLE_NAME) + std::string(".") + 
		std::string(TAG_TABLE_COLUMN_TAG_INDEX) + std::string(" WHERE ") + std::string(TAG_TABLE_NAME) + std::string(".") + std::string(TAG_TABLE_COLUMN_TAG_NAME) + std::string(" = ") +
		std::string("'") + param.GetAddress() + std::string("' ");
	std::string startTimeCondition = date + std::string(" > ");
	if (param.HasPrevPoint()) {
		startTimeCondition = startTimeCondition + std::string("DATEADD(day, ") + std::string("-") + m_attributes.daysBack + std::string(", '") + startDate + std::string("') ");
	}
	else {
		startTimeCondition = startTimeCondition + std::string(" '") + startDate + std::string("' ");
	}
	std::string endTimeCondition;
	if (param.HasPostPoint()) {
		endTimeCondition = date + std::string(" <= ") + std::string(" '") + endDate + std::string("' ");
	}
	else {
		endTimeCondition = date + std::string(" < ") + std::string(" '") + endDate + std::string("' ");
	}
	if (param.GetLimit().IsLimit()) {
		std::string limitCondition = std::string(" ASC ");
		if (param.GetLimit().m_nLimitSide) {
			limitCondition = std::string(" DESC ");
		}
	}
	return query;
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementConditionValueList(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime)
{
	return std::string();
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementLastValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime)
{
	return std::string();
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementFirstValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime)
{
	return std::string();
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementMinValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime)
{
	return std::string();
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementMaxValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime)
{
	return std::string();
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementSumValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime)
{
	return std::string();
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementAvgValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime)
{
	return std::string();
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementTimeStampFirstValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime)
{
	return std::string();
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementTimeStampLastValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime)
{
	return std::string();
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementTimeStampMaxValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime)
{
	return std::string();
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementTimeStampMinValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime)
{
	return std::string();
}

