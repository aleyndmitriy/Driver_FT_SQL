#pragma once
#include"ConnectionAttributes.h"
#include<HdaCommand.h>
#include<vector>
#include"SQLServerTagRecordsDAO.h"
#include<Tvq.h>

namespace DrvFTSQLHdaItem
{
	class HdaCommandHandler
	{
	public:
		HdaCommandHandler();
		~HdaCommandHandler();
		int Init(const ConnectionAttributes& pCfg);
		int Shut();
		ConnectionAttributes GetConfig() const;
		int HandleCommand(ODS::HdaCommand* pCommand, ODS::HdaCommandResult* pResult);
	private:
		int ExecuteCommand(ODS::HdaCommand* pCommand, ODS::HdaFunction* funcList, int listSize, std::vector<ODS::HdaFunctionResult*>* pResultList);
		int AnalyzeCommand(ODS::HdaCommand* pCommand, ODS::HdaFunction* funcList, int listSize, std::map<int, std::vector<ODS::HdaFunction*> >& requestMap);
		int HandleOpenSession(ODS::HdaFunction* pFunc, std::vector<ODS::HdaFunctionResult*>* pResultList);
		int HandleCloseSession(ODS::HdaFunction* pFunc, std::vector<ODS::HdaFunctionResult*>* pResultList);
		ConnectionAttributes m_attributes;
		std::unique_ptr<SQLServerTagRecordsDAO> m_database;
		std::vector<std::string> BuildCmdValueList(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList, const std::map<std::string, TagItemRecord>& tags);
		std::vector<std::string> BuildCmdValueListConditions(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList, const std::map<std::string, TagItemRecord>& tags);
		std::vector<std::string> BuildCmdFirstValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList, const std::map<std::string, TagItemRecord>& tags);
		std::vector<std::string> BuildCmdLastValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList, const std::map<std::string, TagItemRecord>& tags);
		std::vector<std::string> BuildCmdMinValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList, const std::map<std::string, TagItemRecord>& tags);
		std::vector<std::string> BuildCmdMaxValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList, const std::map<std::string, TagItemRecord>& tags);
		std::vector<std::string> BuildCmdSumValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList, const std::map<std::string, TagItemRecord>& tags);
		std::vector<std::string> BuildCmdAvgValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList, const std::map<std::string, TagItemRecord>& tags);
		std::vector<std::string> BuildCmdTimeStampFirstValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList, const std::map<std::string, TagItemRecord>& tags);
		std::vector<std::string> BuildCmdTimeStampLastValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList, const std::map<std::string, TagItemRecord>& tags);
		std::vector<std::string> BuildCmdTimeStampMaxValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList, const std::map<std::string, TagItemRecord>& tags);
		std::vector<std::string> BuildCmdTimeStampMinValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList, const std::map<std::string, TagItemRecord>& tags);
		ParamValueList GetParameterValueList(const ODS::HdaFunction* pHdaFunc);
		ODS::Tvq* CreateTvqFromRecord(const Record& record) const;
		void CreateQueriesList(const std::map<int, std::vector<ODS::HdaFunction*> >& requestFunctions, std::map<int, std::vector<std::string> >& queriesList, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::string& sessionId);
		void ExecuteQueriesList(const std::map<int, std::vector<ODS::HdaFunction*> >& requestFunctions, const std::map<int, std::vector<std::string> >& queriesList, std::vector<ODS::HdaFunctionResult*>* pResultList, const std::string& sessionId);
	};
}