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
		std::string CreateStatementValueList(ParamValueList&& param, std::string startTime, std::string endTime, std::string tagName, int tagId);
		std::map<std::string, TagItemRecord> GetTags();
		std::string OpenConnection();
		bool OpenConnectionWithUUID(const std::string& connectionID);
		void CloseConnectionWithUUID(const std::string& connectionID);
	private:
		ConnectionAttributes m_attributes;
		std::string m_tagTableName;
		std::string m_floatValueTableName;
		std::string m_stringValueTableName;
		std::map<std::string, std::unique_ptr<IDatabaseEngine> > m_dataConnectionsList;
		std::unique_ptr<SQLTable> GetTableInfo(const std::string& connectionID, std::string tableName);
	};
}