#pragma once
#include<map>
#include<memory>
#include"DatabaseEngine.h"
#include<HdaFunctionTypes.h>
#include"ParamValueList.h"

namespace DrvFTSQLHdaItem
{
	class SQLServerTagRecordsDAO {
	public:
		std::string CreateStatementValueList(ParamValueList&& param, std::string startTime, std::string endTime, std::string tagName, int tagId);
	private:
		std::map <std::string, std::unique_ptr<IDatabaseEngine> > m_dataConnectionsList;
		ConnectionAttributes m_attributes;
	};
}