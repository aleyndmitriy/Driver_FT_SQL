#pragma once
#include<map>
#include<memory>
#include"DatabaseEngine.h"
#include<HdaFunctionTypes.h>
#include"ParamValueList.h"
#include"TagItemRecord.h"
#include"SQLTable.h"

namespace DrvFTSQLHdaItem
{
	class SQLServerTagRecordsDAO {
	public:
		SQLServerTagRecordsDAO(const ConnectionAttributes& attributes, const std::string& tagTable, const std::string& floatValueTable, const std::string& stringValueTable);
		~SQLServerTagRecordsDAO();
		SQLServerTagRecordsDAO() = delete;
		SQLServerTagRecordsDAO(const SQLServerTagRecordsDAO& src) = delete;
		SQLServerTagRecordsDAO(SQLServerTagRecordsDAO&& src) = delete;
		SQLServerTagRecordsDAO& operator=(const SQLServerTagRecordsDAO& src) = delete;
		SQLServerTagRecordsDAO& operator=(SQLServerTagRecordsDAO&& src) = delete;
		std::string CreateStatementValueList(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags);
		std::string CreateStatementConditionValueList(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags);
		std::string CreateStatementLastValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags);
		std::string CreateStatementFirstValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags);
		std::string CreateStatementMinValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags);
		std::string CreateStatementMaxValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags);
		std::string CreateStatementSumValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags);
		std::string CreateStatementAvgValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags);
		std::string CreateStatementTimeStampFirstValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags);
		std::string CreateStatementTimeStampLastValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags);
		std::string CreateStatementTimeStampMaxValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags);
		std::string CreateStatementTimeStampMinValue(ParamValueList&& param, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::map<std::string, TagItemRecord>& tags);
		std::map<std::string, TagItemRecord> GetTags(const std::string& sessionId, const std::vector<std::string>& tagsName);
		std::string OpenConnection();
		bool OpenConnectionWithUUID(const std::string& connectionID);
		void CloseConnectionWithUUID(const std::string& connectionID);
		std::vector<Record> GetRecords(const std::string& statement, const std::string& connectionID);
	private:
		ConnectionAttributes m_attributes;
		std::string m_tagTableName;
		std::string m_floatValueTableName;
		std::string m_stringValueTableName;
		std::map<std::string, std::unique_ptr<IDatabaseEngine> > m_dataConnectionsList;
		std::unique_ptr<SQLTable> GetTableInfo(const std::string& connectionID, std::string tableName);
		std::string GetTableNameFromDataType(short dataType);
		std::string ParseCondition(const std::string& condition, const std::string& startTime, const std::string& endTime, const std::map<std::string, TagItemRecord>& tags);
		std::vector<std::pair<std::string, std::string>> GetConditionsFromParam( std::string&& sql);
		void CreateDateTimeString(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::string& tableName, std::string& date, std::string& startDate, std::string& startDatePrev, std::string& endDate);
		void CreatePrevPointSql(std::string& prev, const std::string& tableName, short tagId, const std::string& startDate, const std::vector <std::pair<std::string, std::string> >& conditions, const std::map<std::string, TagItemRecord>& tags);
		void CreatePostPointSql(std::string& prev, const std::string& tableName, short tagId, const std::string& endDate, const std::vector <std::pair<std::string, std::string> >& conditions, const std::map<std::string, TagItemRecord>& tags);
	};
}