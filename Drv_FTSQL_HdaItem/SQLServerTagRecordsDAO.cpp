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
	if (!table) {
		Log::GetInstance()->WriteInfo(_T("Can't find table with name  %s ."), (LPCTSTR)m_tagTableName.c_str());
		return tags;
	}
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
	std::string limitCondition = std::string(" ORDER BY ") + std::string(TAG_TABLE_COLUMN_DATE_TIME);
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
	std::string query = CreateStatementList(std::move(param), startTime, endTime, tags);
	
	query = query + limitCondition;
	return query;
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementConditionValueList(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags)
{
	std::map<std::string, TagItemRecord >::const_iterator tagItr = tags.find(param.GetAddress());
	if (tagItr == tags.cend()) {
		return std::string();
	}
	std::string tableName = GetTableNameFromDataType(tagItr->second.GetTagDataType());
	if (tableName.empty()) {
		return tableName;
	}
	std::string date;
	std::string startDate;
	std::string startDatePrev;
	std::string endDate;
	std::vector <std::pair<std::string, std::string> > conditions;
	if (param.HasSql()) {
		conditions = GetConditionsFromParam(param.GetSqc());
	}
	CreateDateTimeString(startTime, endTime, tableName, date, startDate, startDatePrev, endDate);
	date = tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME);
	std::string query = std::string("SELECT ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_VALUE) + 
		std::string("' , ") + date + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string("' , ") +
		tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_MILLITM) +
		std::string("' FROM ") + tableName + std::string(" WHERE ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_TAG_INDEX) + std::string(" = ") +
		std::to_string(tagItr->second.GetTagId()) + std::string(" AND ") + date + std::string(" > '") + startDate + std::string("' AND ") + date + std::string(" < '") + endDate + std::string("'");

	std::string prev;
	if (param.HasPrevPoint()) {
		CreatePrevPointConditionalSql(prev, tableName, tagItr->second.GetTagId(), startDate, conditions, tags);
	}

	std::string post;
	if (param.HasPostPoint()) {
		CreatePostPointConditionalSql(post, tableName, tagItr->second.GetTagId(), endDate, conditions, tags);
	}

	if (!conditions.empty()) {
		std::string queries;
		for (std::vector <std::pair<std::string, std::string> >::const_iterator itr = conditions.cbegin(); itr != conditions.cend(); ++itr) {
			queries = queries + ParseCondition(itr->first, std::string("'") + startDate + std::string("'"), std::string("'") + endDate + std::string("'"), tags) + itr->second;
		}
		query = prev + std::string("SELECT Tags.") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_VALUE) + 
			std::string("', Tags.") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_DATE_TIME) +
			std::string("', Tags.") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_MILLITM) +
			std::string("', TagsConditions.") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_CONDITION_DATE_TIME) +
			std::string("', TagsConditions.") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_CONDITION_MILLISEC) +
			std::string("' FROM (") + query + std::string(") AS Tags LEFT OUTER JOIN (") + queries + (") AS TagsConditions ON Tags.") +
			std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string("= TagsConditions.") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" AND Tags.") +
			std::string(TAG_TABLE_COLUMN_MILLITM) + std::string("= TagsConditions.") + std::string(TAG_TABLE_COLUMN_MILLITM);
	}
			
	if (!prev.empty()) {
		query = prev + query;
	}
	if (!post.empty()) {
		query += post;
	}
	std::string limitCondition = std::string(" ORDER BY ") + std::string(TAG_TABLE_COLUMN_DATE_TIME);
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

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementLastValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags)
{
	std::string query = CreateStatementList(std::move(param), startTime, endTime, tags);
	query = std::string("SELECT TOP(1) UNION_TABLE.") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string(", UNION_TABLE.") + std::string(TAG_TABLE_COLUMN_DATE_TIME) +
		std::string(", UNION_TABLE.") + std::string(TAG_TABLE_COLUMN_MILLITM) +
		std::string(" FROM (") + query + std::string(") AS UNION_TABLE ORDER BY UNION_TABLE.") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" DESC");
	return query;
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementFirstValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags)
{
	std::string query = CreateStatementList(std::move(param), startTime, endTime, tags);
	query = std::string("SELECT TOP(1) UNION_TABLE.") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string(", UNION_TABLE.") + std::string(TAG_TABLE_COLUMN_DATE_TIME) +
		std::string(", UNION_TABLE.") + std::string(TAG_TABLE_COLUMN_MILLITM) +
		std::string(" FROM (") + query + std::string(") AS UNION_TABLE ORDER BY UNION_TABLE.") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" ASC");
	return query;
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementMinValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags)
{
	std::string query = CreateStatementList(std::move(param), startTime, endTime, tags);
	query = std::string("SELECT TOP(1) UNION_TABLE.") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string(", UNION_TABLE.") + std::string(TAG_TABLE_COLUMN_DATE_TIME) +
		std::string(", UNION_TABLE.") + std::string(TAG_TABLE_COLUMN_MILLITM) +
		std::string(" FROM (") + query + std::string(") AS UNION_TABLE ORDER BY UNION_TABLE.") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string(" ASC, ")+ std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" DESC");
	return query;
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementMaxValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags)
{
	std::string query = CreateStatementList(std::move(param), startTime, endTime, tags);
	query = std::string("SELECT TOP(1) UNION_TABLE.") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string(", UNION_TABLE.") + std::string(TAG_TABLE_COLUMN_DATE_TIME) +
		std::string(", UNION_TABLE.") + std::string(TAG_TABLE_COLUMN_MILLITM) +
		std::string(" FROM (") + query + std::string(") AS UNION_TABLE ORDER BY UNION_TABLE.") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string(" DESC, ") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" DESC");
	return query;
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementSumValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags)
{
	std::string query = CreateStatementList(std::move(param), startTime, endTime, tags);
	query = std::string("SELECT SUM(UNION_TABLE.") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string(") AS '") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string("' , NULL AS ") + std::string(TAG_TABLE_COLUMN_DATE_TIME) +
		std::string(" FROM (") + query + std::string(") AS UNION_TABLE");
	return query;
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementAvgValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags)
{
	std::string query = CreateStatementList(std::move(param), startTime, endTime, tags);
	query = std::string("SELECT AVG(UNION_TABLE.") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string(") AS '") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string("', NULL AS ") + std::string(TAG_TABLE_COLUMN_DATE_TIME) +
		std::string(" FROM (") + query + std::string(") AS UNION_TABLE");
	return query;
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementTimeStampFirstValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags)
{
	return CreateStatementFirstValue(std::move(param),startTime,endTime, tags);
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementTimeStampLastValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags)
{
	return CreateStatementLastValue(std::move(param), startTime, endTime, tags);
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementTimeStampMaxValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags)
{
	return CreateStatementMaxValue(std::move(param), startTime, endTime, tags);
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementTimeStampMinValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags)
{
	return CreateStatementMinValue(std::move(param), startTime, endTime, tags);
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

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::ParseCondition(const std::string& condition, const std::string& startTime, const std::string& endTime, const std::map<std::string, TagItemRecord>& tags)
{
	std::string res;
	size_t posBegin = condition.find("[i#", 0);
	if (posBegin == std::string::npos) {
		return res;
	}
	posBegin = condition.find(".", posBegin);
	posBegin += 1;
	size_t posEnd = condition.find_last_of(']', condition.size() - 1);
	if (posEnd == std::string::npos) {
		return res;
	}
	std::string tagName = condition.substr(posBegin, posEnd - posBegin);
	std::map<std::string, TagItemRecord >::const_iterator tagItr = tags.find(tagName);
	if (tagItr == tags.cend()) {
		return res;
	}
	std::string tableName = GetTableNameFromDataType(tagItr->second.GetTagDataType());
	if (tableName.empty()) {
		return res;
	}
	/*std::string date = std::string("DATEADD(millisecond,") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(",") + tableName + std::string(".") +
		std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(")");*/
	std::string date = tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME);
	res = std::string("SELECT ") + date + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string("', ") + 
		tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_MILLITM) + 
		std::string("' FROM ") + tableName + std::string(" WHERE ") + date + std::string(" > ") + startTime;
	if (!endTime.empty()) {
		res = res + std::string(" AND ") + date + std::string(" < ") + endTime;
	}
	 res = res + std::string(" AND ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_TAG_INDEX) + std::string(" = ") + std::to_string(tagItr->second.GetTagId()) +
		std::string(" AND ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_VALUE) + condition.substr(posEnd + 1, std::string::npos);
	return res;
}

std::vector<std::pair<std::string, std::string> > DrvFTSQLHdaItem::SQLServerTagRecordsDAO::GetConditionsFromParam(const std::string& sql)
{
	std::vector<std::pair<std::string, std::string> > res;
	size_t posBegin = sql.find("[i#", 0);
	if (posBegin == std::string::npos) {
		Log::GetInstance()->WriteInfo(_T("Could not parse the conditions"));
		return res;
	}
	size_t posPrevBegin = 0;
	size_t posOrBegin = sql.find(" OR ", 0);
	if (posOrBegin == std::string::npos) {
		posOrBegin = sql.find(" or ", 0);
	}
	size_t posAndBegin = sql.find(" AND ", 0);
	if (posAndBegin == std::string::npos) {
		posAndBegin = sql.find(" and ", 0);
	}
	while (posAndBegin != std::string::npos || posOrBegin != std::string::npos) {
		if (posAndBegin > posOrBegin) {
			res.push_back(std::make_pair<std::string, std::string>(sql.substr(posPrevBegin, posOrBegin - posPrevBegin), std::string(" UNION ")));
			posPrevBegin = posOrBegin + 4;
		}
		else {
			res.push_back(std::make_pair<std::string, std::string>(sql.substr(posPrevBegin, posAndBegin - posPrevBegin), std::string(" INTERSECT ")));
			posPrevBegin = posAndBegin + 5;
		}
		posOrBegin = sql.find(" OR ", posPrevBegin);
		if (posOrBegin == std::string::npos) {
			posOrBegin = sql.find(" or ", posPrevBegin);
		}
		posAndBegin = sql.find("AND", posPrevBegin);
		if (posAndBegin == std::string::npos) {
			posAndBegin = sql.find(" and ", posPrevBegin);
		}
	}
	res.push_back(std::make_pair<std::string, std::string>(sql.substr(posPrevBegin, std::string::npos), std::string()));
	return res;
}

void DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateDateTimeString(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::string& tableName, std::string& date, std::string& startDate, std::string& startDatePrev, std::string& endDate)
{
	date = std::string("DATEADD(millisecond,") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(",") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(")");

	startDate = std::to_string(startTime.wYear) + std::string("-") + std::to_string(startTime.wMonth) + std::string("-") +
		std::to_string(startTime.wDay) + std::string(" ") + std::to_string(startTime.wHour) + std::string(":") +
		std::to_string(startTime.wMinute) + std::string(":") + std::to_string(startTime.wSecond);

	startDatePrev = std::string("DATEADD(day, ") + std::string("-") + m_attributes.daysBack + std::string(", '") + startDate + std::string("') ");

	endDate = std::to_string(endTime.wYear) + std::string("-") + std::to_string(endTime.wMonth) + std::string("-") +
		std::to_string(endTime.wDay) + std::string(" ") + std::to_string(endTime.wHour) + std::string(":") +
		std::to_string(endTime.wMinute) + std::string(":") + std::to_string(endTime.wSecond);
}

void DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreatePrevPointSql(std::string& prev, const std::string& tableName, short tagId, const std::string& startDate, const std::string& sql, const std::vector <std::pair<std::string, std::string> >& conditions, const std::map<std::string, TagItemRecord>& tags)
{
	//std::string date = std::string("DATEADD(millisecond,") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(",") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(")");
	std::string date = tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME);

	std::string startDatePrev = std::string("DATEADD(day, ") + std::string("-") + m_attributes.daysBack + std::string(", '") + startDate + std::string("') ");
	prev = std::string("SELECT TOP(1) ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string("  AS '") +
		std::string(TAG_TABLE_COLUMN_VALUE) + std::string("' , ") + date + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string("' , ") +
		tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_MILLITM) +
		std::string("' FROM ") + tableName + std::string(" WHERE ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_TAG_INDEX) + std::string(" = ") +
		std::to_string(tagId) + std::string(" AND ") +
		date + std::string(" >= DATEADD(day, ") + std::string("-") + m_attributes.daysBack + std::string(", '") + startDate + std::string("') AND ") +
		date + std::string(" < '") + startDate + std::string("'"); 
	if (conditions.empty() && sql.size() > 1) {
		prev = prev + std::string(" ") + sql ;
	}
	prev = prev + std::string(" ORDER BY ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" DESC");
	
	if (!conditions.empty()) {
		std::string prevQueries;
		for (std::vector <std::pair<std::string, std::string> >::const_iterator itr = conditions.cbegin(); itr != conditions.cend(); ++itr) {
			prevQueries = prevQueries + ParseCondition(itr->first, startDatePrev, std::string("'") + startDate + std::string("'"), tags) + itr->second;
		}
		prev = std::string("SELECT PrevTag.") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string("  AS '") + std::string(TAG_TABLE_COLUMN_VALUE) + 
			std::string("' , PrevTag.") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + 
			std::string("' , PrevTag.") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_MILLITM) +
			std::string("' FROM (") + prev + std::string(") AS PrevTag INNER JOIN (") + prevQueries + (") AS TagPrevConditions ON PrevTag.") +
			std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string("= TagPrevConditions.") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" AND PrevTag.") +
			std::string(TAG_TABLE_COLUMN_MILLITM) + std::string("= TagPrevConditions.") + std::string(TAG_TABLE_COLUMN_MILLITM);
	}
	else {
		prev = std::string("SELECT PrevTag.") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string("  AS '") + std::string(TAG_TABLE_COLUMN_VALUE) +
			std::string("' , PrevTag.") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_DATE_TIME) +
			std::string("' , PrevTag.") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_MILLITM) +
			std::string("' FROM (") + prev + std::string(") AS PrevTag");
	}
	prev += std::string(" UNION ALL ");
}

void DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreatePrevPointConditionalSql(std::string& prev, const std::string& tableName, short tagId, const std::string& startDate, const std::vector <std::pair<std::string, std::string> >& conditions, const std::map<std::string, TagItemRecord>& tags)
{
	if (conditions.empty()) {
		return;
	}
	//std::string date = std::string("DATEADD(millisecond,") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(",") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(")");
	std::string date = tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME);
	std::string startDatePrev = std::string("DATEADD(day, ") + std::string("-") + m_attributes.daysBack + std::string(", '") + startDate + std::string("') ");
	prev = std::string("SELECT TOP(1) ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string("  AS '") +
		std::string(TAG_TABLE_COLUMN_VALUE) + std::string("' , ") + date + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string("' , ") +
		tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_MILLITM) +
		std::string("' FROM ") + tableName + std::string(" WHERE ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_TAG_INDEX) + std::string(" = ") +
		std::to_string(tagId) + std::string(" AND ") +
		date + std::string(" >= DATEADD(day, ") + std::string("-") + m_attributes.daysBack + std::string(", '") + startDate + std::string("') AND ") +
		date + std::string(" < '") + startDate + std::string("' ORDER BY ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" DESC");
	
	if (!conditions.empty()) {
		std::string prevQueries;
		for (std::vector <std::pair<std::string, std::string> >::const_iterator itr = conditions.cbegin(); itr != conditions.cend(); ++itr) {
			prevQueries = prevQueries + ParseCondition(itr->first, startDatePrev, std::string("'") + startDate + std::string("'"), tags) + itr->second;
		}
		prev = std::string("SELECT PrevTag.") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string("  AS '") + std::string(TAG_TABLE_COLUMN_VALUE) +
			std::string("' , PrevTag.") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + 
			std::string("' , PrevTag.") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_MILLITM) +
			std::string("' , TagPrevConditions.") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_CONDITION_DATE_TIME) +
			std::string("' , TagPrevConditions.") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_CONDITION_MILLISEC) +
			std::string("' FROM (") + prev + std::string(") AS PrevTag LEFT OUTER JOIN (") + prevQueries + (") AS TagPrevConditions ON PrevTag.") +
			std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string("= TagPrevConditions.") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" AND PrevTag.") +
			std::string(TAG_TABLE_COLUMN_MILLITM) + std::string("= TagPrevConditions.") + std::string(TAG_TABLE_COLUMN_MILLITM);
	}
	prev += std::string(" UNION ALL ");
}


void DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreatePostPointSql(std::string& post, const std::string& tableName, short tagId, const std::string& endDate, const std::string& sql, const std::vector <std::pair<std::string, std::string> >& conditions, const std::map<std::string, TagItemRecord>& tags)
{
	//std::string date = std::string("DATEADD(millisecond,") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(",") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(")");
	std::string date = tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME);
	post = std::string("SELECT TOP(1) ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string("  AS '") + std::string(TAG_TABLE_COLUMN_VALUE) +
		std::string("' , ") + date + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string("' , ") +
		tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_MILLITM) +
		std::string("' FROM ") + tableName + std::string(" WHERE ") +
		tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_TAG_INDEX) + std::string(" = ") +
		std::to_string(tagId) + std::string(" AND ") + date + std::string(" >= '") + endDate + std::string("'");
	if (conditions.empty() && sql.size() > 1) {
		post = post + std::string(" ")+ sql;
	}
	post = post + std::string(" ORDER BY ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" ASC");
	if (!conditions.empty()) {
		std::string postQueries;
		for (std::vector <std::pair<std::string, std::string> >::const_iterator itr = conditions.cbegin(); itr != conditions.cend(); ++itr) {
			postQueries = postQueries + ParseCondition(itr->first, std::string("'") + endDate + std::string("'"), std::string(), tags) + itr->second;
		}
		post = std::string("SELECT PostTag.") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string("  AS '") + std::string(TAG_TABLE_COLUMN_VALUE) + 
			std::string("' , PostTag.") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_DATE_TIME) +
			std::string("' , PostTag.") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_MILLITM) +
			std::string("' FROM (") + post + std::string(") AS PostTag INNER JOIN (") + postQueries + (") AS TagPostConditions ON PostTag.") +
			std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string("= TagPostConditions.") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" AND PostTag.") +
			std::string(TAG_TABLE_COLUMN_MILLITM) + std::string("= TagPostConditions.") + std::string(TAG_TABLE_COLUMN_MILLITM);
	}
	else {
		post = std::string("SELECT PostTag.") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string("  AS '") + std::string(TAG_TABLE_COLUMN_VALUE) +
			std::string("' , PostTag.") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_DATE_TIME) +
			std::string("' , PostTag.") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_MILLITM) +
			std::string("' FROM (") + post + std::string(") AS PostTag");
	}
	post = std::string(" UNION ALL ") + post;
}

void DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreatePostPointConditionalSql(std::string& post, const std::string& tableName, short tagId, const std::string& endDate, const std::vector <std::pair<std::string, std::string> >& conditions, const std::map<std::string, TagItemRecord>& tags)
{
	if (conditions.empty()) {
		return;
	}
	//std::string date = std::string("DATEADD(millisecond,") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(",") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(")");
	std::string date = tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME);
	post = std::string("SELECT TOP(1) ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string("  AS '") + std::string(TAG_TABLE_COLUMN_VALUE) +
		std::string("' , ") + date + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string("' , ") +
		tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_MILLITM) +
		std::string("' FROM ") + tableName + std::string(" WHERE ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_TAG_INDEX) + std::string(" = ") +
		std::to_string(tagId) + std::string(" AND ") + date + std::string(" >= '") + endDate +
		std::string("' ORDER BY ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" ASC");
	
	
	if (!conditions.empty()) {
		std::string postQueries;
		for (std::vector <std::pair<std::string, std::string> >::const_iterator itr = conditions.cbegin(); itr != conditions.cend(); ++itr) {
			postQueries = postQueries + ParseCondition(itr->first, std::string("'") + endDate + std::string("'"), std::string(), tags) + itr->second;
		}

		post = std::string("SELECT PostTag.") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string("  AS '") + std::string(TAG_TABLE_COLUMN_VALUE) +
			std::string("' , PostTag.") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_DATE_TIME) +
			std::string("' , PostTag.") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_MILLITM) +
			std::string("', TagPostConditions.") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_CONDITION_DATE_TIME) +
			std::string("', TagPostConditions.") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_CONDITION_MILLISEC) +
			std::string("' FROM (") + post + std::string(") AS PostTag LEFT OUTER JOIN (") + postQueries + (") AS TagPostConditions ON PostTag.") +
			std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string("= TagPostConditions.") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" AND PostTag.") +
			std::string(TAG_TABLE_COLUMN_MILLITM) + std::string("= TagPostConditions.") + std::string(TAG_TABLE_COLUMN_MILLITM);
	}
	post = std::string(" UNION ALL ") + post;
}

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementList(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags)
{
	std::map<std::string, TagItemRecord >::const_iterator tagItr = tags.find(param.GetAddress());
	if (tagItr == tags.cend()) {
		return std::string();
	}
	std::string tableName = GetTableNameFromDataType(tagItr->second.GetTagDataType());
	if (tableName.empty()) {
		return tableName;
	}
	std::string date;
	std::string startDate;
	std::string startDatePrev;
	std::string endDate;
	std::vector <std::pair<std::string, std::string> > conditions;
	std::string sql = param.GetSqc();
	if (param.HasSql()) {
		conditions = GetConditionsFromParam(sql);
	}
	CreateDateTimeString(startTime, endTime, tableName, date, startDate, startDatePrev, endDate);
	date = tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_DATE_TIME);
	std::string query = std::string("SELECT ") + tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string("  AS '") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string("', ") +
		date + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string("' , ") +
		tableName + std::string(".") + std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_MILLITM) +
		std::string("' FROM ") + tableName + std::string(" WHERE ") + tableName +
		std::string(".") + std::string(TAG_TABLE_COLUMN_TAG_INDEX) + std::string(" = ") + std::to_string(tagItr->second.GetTagId()) + std::string(" AND ") +
		date + std::string(" > '") + startDate + std::string("' AND ") + date + std::string(" < '") + endDate + std::string("'");
	if (conditions.empty() && sql.size() > 0) {
		query = query + std::string(" ") + sql;
	}
	std::string prev;
	if (param.HasPrevPoint()) {
		CreatePrevPointSql(prev, tableName, tagItr->second.GetTagId(), startDate, sql, conditions, tags);
	}

	std::string post; 
	//if (param.HasPostPoint()) {
		CreatePostPointSql(post, tableName, tagItr->second.GetTagId(), endDate, sql, conditions, tags);
	//}

	if (!conditions.empty()) {
		std::string queries;
		for (std::vector <std::pair<std::string, std::string> >::const_iterator itr = conditions.cbegin(); itr != conditions.cend(); ++itr) {
			queries = queries + ParseCondition(itr->first, std::string("'") + startDate + std::string("'"), std::string("'") + endDate + std::string("'"), tags) + itr->second;
		}
		query = std::string("SELECT Tags.") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string("  AS '") + std::string(TAG_TABLE_COLUMN_VALUE) + std::string("', Tags.") +
			std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string("', Tags.") +
			std::string(TAG_TABLE_COLUMN_MILLITM) + std::string(" AS '") + std::string(TAG_TABLE_COLUMN_MILLITM) +
			std::string("' FROM (") + query + std::string(") AS Tags INNER JOIN (") + queries + (") AS TagsConditions ON Tags.") +
			std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string("= TagsConditions.") + std::string(TAG_TABLE_COLUMN_DATE_TIME) + std::string(" AND Tags.") +
			std::string(TAG_TABLE_COLUMN_MILLITM) + std::string("= TagsConditions.") + std::string(TAG_TABLE_COLUMN_MILLITM);
	}

	if (!prev.empty()) {
		query = prev + query;
	}
	if (!post.empty()) {
		query += post;
	}
	return query;
}