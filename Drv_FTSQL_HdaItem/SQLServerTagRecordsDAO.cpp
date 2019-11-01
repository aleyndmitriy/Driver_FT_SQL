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


std::map<std::string, DrvFTSQLHdaItem::TagItemRecord> DrvFTSQLHdaItem::SQLServerTagRecordsDAO::GetTags(const std::string& sessionId, const std::vector<std::string>& tagsName)
{
	std::map<std::string, TagItemRecord> tags;
	std::map<std::string, std::unique_ptr<IDatabaseEngine> >::iterator itr = m_dataConnectionsList.find(sessionId);
	if (itr == m_dataConnectionsList.end()) {
		return tags;
	}
	std::unique_ptr<SQLTable> table = GetTableInfo(sessionId, m_tagTableName);
	std::string sql = std::string("SELECT ");
	for (SQLTable::const_iterator itr = table->cbegin(); itr != table->cend(); ++itr) {
		sql += std::string(" ") + table->GetFullName() + std::string(".") + itr->first + std::string(", ");
	}
	sql.erase(sql.size() - 2, 2);
	sql += std::string(" FROM ") + table->GetFullName();
	if (!tagsName.empty()) {
		sql += std::string(" WHERE ");
	}
	for (std::vector<std::string>::const_iterator itr = tagsName.cbegin(); itr != tagsName.cend(); ++itr) {
		sql = sql + m_tagTableName + std::string(".") + std::string(TAG_TABLE_COLUMN_TAG_NAME) + std::string(" = '") + *itr + std::string("' OR ");
	}
	if (!tagsName.empty()) {
		sql.erase(sql.size() - 4, 4);
	}
	Log::GetInstance()->WriteInfo(_T("All Tag SQL Query : % s ."), (LPCTSTR)sql.c_str());
	std::vector<Record> records;
	records = itr->second->ExecuteStatement(sql, std::vector<std::string>());
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
	if (connectionID.empty()) {
		return false;
	}
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

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementValueList(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags)
{
	
	/*SELECT FloatTable.Val, DATEADD(millisecond,FloatTable.Millitm,FloatTable.DateAndTime) AS 'DateTime' FROM FloatTable INNER JOIN TagTable ON FloatTable.TagIndex = TagTable.TagIndex  
	WHERE TagTable.TagName = '[PLC_CP]PT3_1013.Val' AND DATEADD(millisecond,FloatTable.Millitm,FloatTable.DateAndTime) > DATEADD(day,-2, '2019-05-29') AND
DATEADD(millisecond,FloatTable.Millitm,FloatTable.DateAndTime) < (SELECT MIN(DATEADD(millisecond,FloatTable.Millitm,FloatTable.DateAndTime)) 
FROM FloatTable WHERE DATEADD(millisecond,FloatTable.Millitm,FloatTable.DateAndTime) > '2019-06-16') ORDER BY FloatTable.DateAndTime DESC OFFSET 50 ROWS FETCH NEXT 25 ROWS ONLY*/
	std::map<std::string, TagItemRecord >::const_iterator tagItr = tags.find(param.GetAddress());
	if (tagItr == tags.cend()) {
		return std::string();
	}

	std::string tableName = GetTableNameFromDataType(tagItr->second.GetTagDataType());
	if (tableName.empty()) {
		return tableName;
	}
	std::string date = std::string(" DATEADD(millisecond,") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(",") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(")");
	std::string startDate = std::to_string(startTime.wYear) + std::string("-") + std::to_string(startTime.wMonth) + std::string("-") +
		std::to_string(startTime.wDay) + std::string(" ") + std::to_string(startTime.wHour) + std::string(":") +
		std::to_string(startTime.wMinute) + std::string(":") + std::to_string(startTime.wSecond);
	std::string endDate = std::to_string(endTime.wYear) + std::string("-") + std::to_string(endTime.wMonth) + std::string("-") +
		std::to_string(endTime.wDay) + std::string(" ") + std::to_string(endTime.wHour) + std::string(":") +
		std::to_string(endTime.wMinute) + std::string(":") + std::to_string(endTime.wSecond);

	std::string query = std::string("SELECT ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string(", ") + date + std::string(" AS ") + std::string(TAG_TABLE_COLUMN_DATE_TIME_MILLISEC) +
		std::string(" FROM ") + tableName + std::string(" INNER JOIN ") + std::string(TAG_TABLE_NAME) + std::string(" ON ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_TAG_INDEX) + std::string(" = ") + std::string(TAG_TABLE_NAME) + std::string(".") +
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
	query = query + std::string(" AND ") + startTimeCondition + std::string(" AND ") + endTimeCondition;
	if (param.HasSql()) {
		query = query + std::string(" ") + param.GetSqc();
	}
	std::string limitCondition = std::string(" ORDER BY ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME);
	if (param.GetLimit().IsLimit()) {
		if (param.GetLimit().m_nLimitSide) {
			limitCondition = limitCondition + std::string(" DESC ");
		}
		else {
			limitCondition = limitCondition + std::string(" ASC ");
		}
		limitCondition = limitCondition + std::string("OFFSET ") + std::to_string(param.GetLimit().m_nLimitOffset) + std::string(" ROWS FETCH NEXT ") +
			std::to_string(param.GetLimit().m_nLimitCount) + std::string(" ROWS ONLY");
	}
	query = query + limitCondition;
	return query;
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementConditionValueList(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags)
{
	return CreateStatementValueList(std::move(param), startTime, endTime, tags);
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementLastValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, short tagId, short dataType)
{
	
	/*SELECT TOP(1) FloatTable.Val, DATEADD(millisecond,FloatTable.Millitm,FloatTable.DateAndTime) AS 'DateTime' FROM FloatTable INNER JOIN TagTable ON FloatTable.TagIndex = TagTable.TagIndex
WHERE TagTable.TagName = '[PLC_CP]ZSL_1011.Sts_Closed' AND DATEADD(millisecond,FloatTable.Millitm,FloatTable.DateAndTime) > DATEADD(day,-2, '2019-05-29') AND
DATEADD(millisecond,FloatTable.Millitm,FloatTable.DateAndTime) <=  '2019-06-16' ORDER BY FloatTable.DateAndTime DESC*/
	std::string tableName = GetTableNameFromDataType(dataType);
	if (tableName.empty()) {
		return tableName;
	}
	std::string date = std::string(" DATEADD(millisecond,") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(",") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(")");
	std::string startDate = std::to_string(startTime.wYear) + std::string("-") + std::to_string(startTime.wMonth) + std::string("-") +
		std::to_string(startTime.wDay) + std::string(" ") + std::to_string(startTime.wHour) + std::string(":") +
		std::to_string(startTime.wMinute) + std::string(":") + std::to_string(startTime.wSecond);
	std::string endDate = std::to_string(endTime.wYear) + std::string("-") + std::to_string(endTime.wMonth) + std::string("-") +
		std::to_string(endTime.wDay) + std::string(" ") + std::to_string(endTime.wHour) + std::string(":") +
		std::to_string(endTime.wMinute) + std::string(":") + std::to_string(endTime.wSecond);

	std::string query = std::string("SELECT TOP(1) ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string(", ") + date + std::string(" AS ") + std::string(TAG_TABLE_COLUMN_DATE_TIME_MILLISEC) +
		std::string(" FROM ") + tableName + std::string(" INNER JOIN ") + std::string(TAG_TABLE_NAME) + std::string(" ON ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_TAG_INDEX) + std::string(" = ") + std::string(TAG_TABLE_NAME) + std::string(".") +
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
	query = query + std::string(" AND ") + startTimeCondition + std::string(" AND ") + endTimeCondition;
	if (param.HasSql()) {
		query = query + std::string(" ") + param.GetSqc();
	}
	std::string limitCondition = std::string(" ORDER BY ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" DESC");
	
	query = query + limitCondition;
	return query;
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementFirstValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, short tagId, short dataType)
{
	/*SELECT TOP(1) FloatTable.Val, DATEADD(millisecond,FloatTable.Millitm,FloatTable.DateAndTime) AS 'DateTime' FROM FloatTable INNER JOIN TagTable ON FloatTable.TagIndex = TagTable.TagIndex
	WHERE TagTable.TagName = '[PLC_CP]ZSL_1011.Sts_Closed' AND DATEADD(millisecond,FloatTable.Millitm,FloatTable.DateAndTime) > DATEADD(day,-2, '2019-05-29') AND
	DATEADD(millisecond,FloatTable.Millitm,FloatTable.DateAndTime) <=  '2019-06-16' ORDER BY FloatTable.DateAndTime ASC*/
	std::string tableName = GetTableNameFromDataType(dataType);
	if (tableName.empty()) {
		return tableName;
	}
	std::string date = std::string(" DATEADD(millisecond,") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(",") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(")");
	std::string startDate = std::to_string(startTime.wYear) + std::string("-") + std::to_string(startTime.wMonth) + std::string("-") +
		std::to_string(startTime.wDay) + std::string(" ") + std::to_string(startTime.wHour) + std::string(":") +
		std::to_string(startTime.wMinute) + std::string(":") + std::to_string(startTime.wSecond);
	std::string endDate = std::to_string(endTime.wYear) + std::string("-") + std::to_string(endTime.wMonth) + std::string("-") +
		std::to_string(endTime.wDay) + std::string(" ") + std::to_string(endTime.wHour) + std::string(":") +
		std::to_string(endTime.wMinute) + std::string(":") + std::to_string(endTime.wSecond);

	std::string query = std::string("SELECT TOP(1) ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string(", ") + date + std::string(" AS ") + std::string(TAG_TABLE_COLUMN_DATE_TIME_MILLISEC) +
		std::string(" FROM ") + tableName + std::string(" INNER JOIN ") + std::string(TAG_TABLE_NAME) + std::string(" ON ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_TAG_INDEX) + std::string(" = ") + std::string(TAG_TABLE_NAME) + std::string(".") +
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
	query = query + std::string(" AND ") + startTimeCondition + std::string(" AND ") + endTimeCondition;
	if (param.HasSql()) {
		query = query + std::string(" ") + param.GetSqc();
	}
	std::string limitCondition = std::string(" ORDER BY ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" ASC");

	query = query + limitCondition;
	return query;
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementMinValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, short tagId, short dataType)
{
	/*SELECT TOP(1) FloatTable.Val, DATEADD(millisecond,FloatTable.Millitm,FloatTable.DateAndTime) AS 'DateTime' FROM FloatTable INNER JOIN TagTable ON FloatTable.TagIndex = TagTable.TagIndex
	WHERE TagTable.TagName = '[PLC_CP]ZSL_1011.Sts_Closed' AND DATEADD(millisecond,FloatTable.Millitm,FloatTable.DateAndTime) > DATEADD(day,-2, '2019-05-29') AND
	DATEADD(millisecond,FloatTable.Millitm,FloatTable.DateAndTime) <=  '2019-06-16' ORDER BY FloatTable.Val DESC, FloatTable.DateAndTime DESC*/
	std::string tableName = GetTableNameFromDataType(dataType);
	if (tableName.empty()) {
		return tableName;
	}
	std::string date = std::string(" DATEADD(millisecond,") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(",") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(")");
	std::string startDate = std::to_string(startTime.wYear) + std::string("-") + std::to_string(startTime.wMonth) + std::string("-") +
		std::to_string(startTime.wDay) + std::string(" ") + std::to_string(startTime.wHour) + std::string(":") +
		std::to_string(startTime.wMinute) + std::string(":") + std::to_string(startTime.wSecond);
	std::string endDate = std::to_string(endTime.wYear) + std::string("-") + std::to_string(endTime.wMonth) + std::string("-") +
		std::to_string(endTime.wDay) + std::string(" ") + std::to_string(endTime.wHour) + std::string(":") +
		std::to_string(endTime.wMinute) + std::string(":") + std::to_string(endTime.wSecond);

	std::string query = std::string("SELECT TOP(1) ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string(", ") + date + std::string(" AS ") + std::string(TAG_TABLE_COLUMN_DATE_TIME_MILLISEC) +
		std::string(" FROM ") + tableName + std::string(" INNER JOIN ") + std::string(TAG_TABLE_NAME) + std::string(" ON ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_TAG_INDEX) + std::string(" = ") + std::string(TAG_TABLE_NAME) + std::string(".") +
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
	query = query + std::string(" AND ") + startTimeCondition + std::string(" AND ") + endTimeCondition;
	if (param.HasSql()) {
		query = query + std::string(" ") + param.GetSqc();
	}
	std::string limitCondition = std::string(" ORDER BY ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string(" DESC, ") +
		tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" DESC");

	query = query + limitCondition;
	return query;
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementMaxValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, short tagId, short dataType)
{
	/*SELECT TOP(1) FloatTable.Val, DATEADD(millisecond,FloatTable.Millitm,FloatTable.DateAndTime) AS 'DateTime' FROM FloatTable INNER JOIN TagTable ON FloatTable.TagIndex = TagTable.TagIndex
	WHERE TagTable.TagName = '[PLC_CP]ZSL_1011.Sts_Closed' AND DATEADD(millisecond,FloatTable.Millitm,FloatTable.DateAndTime) > DATEADD(day,-2, '2019-05-29') AND
	DATEADD(millisecond,FloatTable.Millitm,FloatTable.DateAndTime) <=  '2019-06-16' ORDER BY FloatTable.Val ASC, FloatTable.DateAndTime DESC*/
	std::string tableName = GetTableNameFromDataType(dataType);
	if (tableName.empty()) {
		return tableName;
	}
	std::string date = std::string(" DATEADD(millisecond,") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(",") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(")");
	std::string startDate = std::to_string(startTime.wYear) + std::string("-") + std::to_string(startTime.wMonth) + std::string("-") +
		std::to_string(startTime.wDay) + std::string(" ") + std::to_string(startTime.wHour) + std::string(":") +
		std::to_string(startTime.wMinute) + std::string(":") + std::to_string(startTime.wSecond);
	std::string endDate = std::to_string(endTime.wYear) + std::string("-") + std::to_string(endTime.wMonth) + std::string("-") +
		std::to_string(endTime.wDay) + std::string(" ") + std::to_string(endTime.wHour) + std::string(":") +
		std::to_string(endTime.wMinute) + std::string(":") + std::to_string(endTime.wSecond);

	std::string query = std::string("SELECT TOP(1) ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string(", ") + date + std::string(" AS ") + std::string(TAG_TABLE_COLUMN_DATE_TIME_MILLISEC) +
		std::string(" FROM ") + tableName + std::string(" INNER JOIN ") + std::string(TAG_TABLE_NAME) + std::string(" ON ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_TAG_INDEX) + std::string(" = ") + std::string(TAG_TABLE_NAME) + std::string(".") +
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
	query = query + std::string(" AND ") + startTimeCondition + std::string(" AND ") + endTimeCondition;
	if (param.HasSql()) {
		query = query + std::string(" ") + param.GetSqc();
	}
	std::string limitCondition = std::string(" ORDER BY ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string(" ASC, ") +
		tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" DESC");

	query = query + limitCondition;
	return query;
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementSumValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, short tagId, short dataType)
{
	/*SELECT MAX(FloatTable.Val) AS 'FloatVal', '1970-01-01 00:00:00.000' AS 'DateTime' FROM FloatTable INNER JOIN TagTable ON FloatTable.TagIndex = TagTable.TagIndex
	WHERE TagTable.TagName = '[PLC_CP]ZSL_1011.Sts_Closed' AND DATEADD(millisecond, FloatTable.Millitm, FloatTable.DateAndTime) > DATEADD(day, -2, '2019-05-29') AND
	DATEADD(millisecond, FloatTable.Millitm, FloatTable.DateAndTime) <= '2019-06-16'*/
	std::string tableName = GetTableNameFromDataType(dataType);
	if (tableName.empty()) {
		return tableName;
	}
	std::string date = std::string("'1970-01-01 00:00:00.000'");
	std::string startDate = std::to_string(startTime.wYear) + std::string("-") + std::to_string(startTime.wMonth) + std::string("-") +
		std::to_string(startTime.wDay) + std::string(" ") + std::to_string(startTime.wHour) + std::string(":") +
		std::to_string(startTime.wMinute) + std::string(":") + std::to_string(startTime.wSecond);
	std::string endDate = std::to_string(endTime.wYear) + std::string("-") + std::to_string(endTime.wMonth) + std::string("-") +
		std::to_string(endTime.wDay) + std::string(" ") + std::to_string(endTime.wHour) + std::string(":") +
		std::to_string(endTime.wMinute) + std::string(":") + std::to_string(endTime.wSecond);

	std::string query = std::string("SELECT MAX(") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string(") AS 'Value', ") + date + std::string(" AS ") + std::string(TAG_TABLE_COLUMN_DATE_TIME_MILLISEC) +
		std::string(" FROM ") + tableName + std::string(" INNER JOIN ") + std::string(TAG_TABLE_NAME) + std::string(" ON ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_TAG_INDEX) + std::string(" = ") + std::string(TAG_TABLE_NAME) + std::string(".") +
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
	query = query + std::string(" AND ") + startTimeCondition + std::string(" AND ") + endTimeCondition;
	if (param.HasSql()) {
		query = query + std::string(" ") + param.GetSqc();
	}
	
	return query;
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementAvgValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, short tagId, short dataType)
{
	/*SELECT AVG(FloatTable.Val) AS 'FloatVal', '1970-01-01 00:00:00.000' AS 'DateTime' FROM FloatTable INNER JOIN TagTable ON FloatTable.TagIndex = TagTable.TagIndex
	WHERE TagTable.TagName = '[PLC_CP]ZSL_1011.Sts_Closed' AND DATEADD(millisecond, FloatTable.Millitm, FloatTable.DateAndTime) > DATEADD(day, -2, '2019-05-29') AND
	DATEADD(millisecond, FloatTable.Millitm, FloatTable.DateAndTime) <= '2019-06-16'*/
	std::string tableName = GetTableNameFromDataType(dataType);
	if (tableName.empty()) {
		return tableName;
	}
	std::string date = std::string("'1970-01-01 00:00:00.000'");
	std::string startDate = std::to_string(startTime.wYear) + std::string("-") + std::to_string(startTime.wMonth) + std::string("-") +
		std::to_string(startTime.wDay) + std::string(" ") + std::to_string(startTime.wHour) + std::string(":") +
		std::to_string(startTime.wMinute) + std::string(":") + std::to_string(startTime.wSecond);
	std::string endDate = std::to_string(endTime.wYear) + std::string("-") + std::to_string(endTime.wMonth) + std::string("-") +
		std::to_string(endTime.wDay) + std::string(" ") + std::to_string(endTime.wHour) + std::string(":") +
		std::to_string(endTime.wMinute) + std::string(":") + std::to_string(endTime.wSecond);

	std::string query = std::string("SELECT AVG(") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string(") AS 'Value', ") + date + std::string(" AS ") + std::string(TAG_TABLE_COLUMN_DATE_TIME_MILLISEC) +
		std::string(" FROM ") + tableName + std::string(" INNER JOIN ") + std::string(TAG_TABLE_NAME) + std::string(" ON ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_TAG_INDEX) + std::string(" = ") + std::string(TAG_TABLE_NAME) + std::string(".") +
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
	query = query + std::string(" AND ") + startTimeCondition + std::string(" AND ") + endTimeCondition;
	if (param.HasSql()) {
		query = query + std::string(" ") + param.GetSqc();
	}

	return query;
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementTimeStampFirstValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, short tagId, short dataType)
{
	return CreateStatementFirstValue(std::move(param),startTime,endTime, tagId, dataType);
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementTimeStampLastValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, short tagId, short dataType)
{
	return CreateStatementLastValue(std::move(param), startTime, endTime, tagId, dataType);
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementTimeStampMaxValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, short tagId, short dataType)
{
	return CreateStatementMaxValue(std::move(param), startTime, endTime, tagId, dataType);
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementTimeStampMinValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, short tagId, short dataType)
{
	return CreateStatementMinValue(std::move(param), startTime, endTime, tagId, dataType);
}

std::vector<DrvFTSQLHdaItem::Record> DrvFTSQLHdaItem::SQLServerTagRecordsDAO::GetRecords(const std::string& statement, const std::string& connectionID)
{
	std::vector<Record> records;
	std::vector<std::string> params = { };
	std::map<std::string, std::unique_ptr<IDatabaseEngine> >::iterator itr = m_dataConnectionsList.find(connectionID);
	if (itr == m_dataConnectionsList.end()) {
		Log::GetInstance()->WriteInfo(_T("Could not connect to database."));
		return records;
	}
	Log::GetInstance()->WriteInfo(_T("Execute statement: %s ...."), (LPCTSTR)statement.c_str());
	records = itr->second->ExecuteStatement(statement, params);
	if (records.empty()) {
		Log::GetInstance()->WriteInfo(_T("There are no any tags with this conditions"));
		return records;
	}
	Log::GetInstance()->WriteInfo(_T("Number of recordss: %d"), records.size());
	return records;
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::GetTableNameFromDataType(short dataType)
{
	switch (dataType) {
	case TagItemRecord::DATA_TYPE_NUMERIC:
		return std::string(TAG_FLOAT_VALUE_TABLE_NAME);
		break;
	case TagItemRecord::DATA_TYPE_DIGITAL:
	case TagItemRecord::DATA_TYPE_STRING:
		return std::string(TAG_STRING_VALUE_TABLE_NAME);
		break;
	default:
		return std::string(TAG_FLOAT_VALUE_TABLE_NAME);
		break;
	}
}
