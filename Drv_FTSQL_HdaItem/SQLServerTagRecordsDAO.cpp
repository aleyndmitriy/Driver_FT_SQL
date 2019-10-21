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

std::string DrvFTSQLHdaItem::SQLServerTagRecordsDAO::CreateStatementValueList(ParamValueList&& param, std::string startTime, std::string endTime, std::string tagName, int tagId)
{

	return std::string();
}