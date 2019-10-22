#pragma once
#include"ConnectionAttributes.h"
#include<HdaCommand.h>
#include<vector>
#include"SQLServerTagRecordsDAO.h"

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
		int ExecuteCommand(const ODS::HdaCommand* pCommand, std::vector<ODS::HdaFunctionResult*>* pResultList);
		int AnalyzeCommand(const ODS::HdaCommand* pCommand, std::map<int, std::vector<ODS::HdaFunction*> >& requestMap);
		int HandleOpenSession(ODS::HdaFunction* pFunc, std::vector<ODS::HdaFunctionResult*>* pResultList);
		int HandleCloseSession(ODS::HdaFunction* pFunc, std::vector<ODS::HdaFunctionResult*>* pResultList);
		ConnectionAttributes m_attributes;
		std::unique_ptr<SQLServerTagRecordsDAO> m_database;
		std::vector<std::string> BuildCmdValueList(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList);
		std::vector<std::string> BuildCmdValueListConditions(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList);
		std::vector<std::string> BuildCmdFirstValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList);
		std::vector<std::string> BuildCmdLastValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList);
		std::vector<std::string> BuildCmdMinValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList);
		std::vector<std::string> BuildCmdMaxValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList);
		std::vector<std::string> BuildCmdSumValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList);
		std::vector<std::string> BuildCmdAvgValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList);
		std::vector<std::string> BuildCmdTimeStampFirstValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList);
		std::vector<std::string> BuildCmdTimeStampLastValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList);
		std::vector<std::string> BuildCmdTimeStampMaxValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList);
		std::vector<std::string> BuildCmdTimeStampMinValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList);
		ParamValueList GetParameterValueList(const ODS::HdaFunction* pHdaFunc);
	};
}