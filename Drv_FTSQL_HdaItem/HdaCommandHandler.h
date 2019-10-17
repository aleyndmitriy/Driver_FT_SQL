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
		int HandleOpenSession(ODS::HdaFunction* pFunc, ODS::HdaCommandResult** ppResult);
		int HandleCloseSession(ODS::HdaFunction* pFunc, ODS::HdaCommandResult** ppResult);
		ConnectionAttributes m_attributes;

	};
}