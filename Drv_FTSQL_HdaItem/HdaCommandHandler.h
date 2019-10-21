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
		std::string BuildCmdValueList(const ODS::HdaCommand* pCommand, const std::vector<ODS::HdaFunction*>& rFuncList);
		std::string BuildCmdValueListConditions(const ODS::HdaCommand* pCommand, const std::vector<ODS::HdaFunction*>& rFuncList);
		std::string BuildCmdFirstValue(const ODS::HdaCommand* pCommand, const std::vector<ODS::HdaFunction*>& rFuncList);
		std::string BuildCmdLastValue(const ODS::HdaCommand* pCommand, const std::vector<ODS::HdaFunction*>& rFuncList);
		std::string BuildCmdMinValue(const ODS::HdaCommand* pCommand, const std::vector<ODS::HdaFunction*>& rFuncList);
		std::string BuildCmdMaxValue(const ODS::HdaCommand* pCommand, const std::vector<ODS::HdaFunction*>& rFuncList);
		std::string BuildCmdSumValue(const ODS::HdaCommand* pCommand, const std::vector<ODS::HdaFunction*>& rFuncList);
		std::string BuildCmdAvgValue(const ODS::HdaCommand* pCommand, const std::vector<ODS::HdaFunction*>& rFuncList);
		std::string BuildCmdTimeStampFirstValue(const ODS::HdaCommand* pCommand, const std::vector<ODS::HdaFunction*>& rFuncList);
		std::string BuildCmdTimeStampLastValue(const ODS::HdaCommand* pCommand, const std::vector<ODS::HdaFunction*>& rFuncList);
		std::string BuildCmdTimeStampMaxValue(const ODS::HdaCommand* pCommand, const std::vector<ODS::HdaFunction*>& rFuncList);
		std::string BuildCmdTimeStampMinValue(const ODS::HdaCommand* pCommand, const std::vector<ODS::HdaFunction*>& rFuncList);
		ParamValueList GetParameterValueList(const ODS::HdaFunction* pHdaFunc);
	};
}