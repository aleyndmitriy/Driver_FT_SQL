#include"pch.h"
#include"HdaCommandHandler.h"
#include<OdsErr.h>
#include<OdsCoreLib/HdaCommandHelper.h>
#include<HdaFunction.h>
#include <HdaFunctionResult.h>
#include<HdaFunctionTypes.h>

DrvFTSQLHdaItem::HdaCommandHandler::HdaCommandHandler()
{

}

DrvFTSQLHdaItem::HdaCommandHandler::~HdaCommandHandler()
{

}

int DrvFTSQLHdaItem::HdaCommandHandler::Init(const ConnectionAttributes& pCfg)
{
	m_attributes = pCfg;
	return ODS::ERR::OK;
}

int DrvFTSQLHdaItem::HdaCommandHandler::Shut()
{
	return ODS::ERR::OK;
}

DrvFTSQLHdaItem::ConnectionAttributes DrvFTSQLHdaItem::HdaCommandHandler::GetConfig() const
{
	return m_attributes;
}

int DrvFTSQLHdaItem::HdaCommandHandler::HandleCommand(ODS::HdaCommand* pCommand, ODS::HdaCommandResult* pResult)
{
	std::vector<ODS::HdaFunctionResult*> resultList;
	ExecuteCommand(pCommand, &resultList);
	std::vector<ODS::HdaFunctionResult*>::iterator iterRes = resultList.begin(), iterResEnd = resultList.end();
	for (; iterRes != iterResEnd; iterRes++)
	{
		pResult->AddFunctionResult(*iterRes);
	}
	return ODS::ERR::OK;
}

int DrvFTSQLHdaItem::HdaCommandHandler::ExecuteCommand(const ODS::HdaCommand* pCommand, std::vector<ODS::HdaFunctionResult*>* pResultList)
{
	std::map<int,std::vector<ODS::HdaFunction*> > requestMap;
	AnalyzeCommand(pCommand, requestMap);

	return ODS::ERR::OK;
}

int DrvFTSQLHdaItem::HdaCommandHandler::AnalyzeCommand(const ODS::HdaCommand* pCommand, std::map<int, std::vector<ODS::HdaFunction*> >& requestMap)
{
	requestMap.clear();
	ODS::HdaCommand* pCmd = const_cast<ODS::HdaCommand*>(pCommand);
	ODS::HdaCommandHelper ch(pCmd);
	std::vector<ODS::HdaFunction*> funcList;
	ch.GetFunctionList(&funcList);
	if (funcList.size() <= 0) {
		return ODS::ERR::NOT_SUPPORTED;
	}
	for (std::vector<ODS::HdaFunction*>::const_iterator itr = funcList.cbegin(); itr != funcList.cend(); ++itr) {
		int nFuncType = (*itr)->GetType();
		std::pair<int, std::vector<ODS::HdaFunction*> > pair = std::make_pair<int, std::vector<ODS::HdaFunction*> >(std::move(nFuncType), std::vector<ODS::HdaFunction*>());
		std::pair<std::map<int, std::vector<ODS::HdaFunction*> >::iterator, bool > insertedPair = requestMap.insert(pair);
		insertedPair.first->second.push_back(*itr);
	}
	return ODS::ERR::OK;
}

int DrvFTSQLHdaItem::HdaCommandHandler::HandleOpenSession(ODS::HdaFunction* pFunc, ODS::HdaCommandResult** ppResult)
{
	ODS::HdaCommandResult* pRes = new ODS::HdaCommandResult;
	ODS::HdaFunctionResultSession* pSession = new ODS::HdaFunctionResultSession;
	pSession->SetContext(pFunc->GetContext());
	ODS::Core::Uuid sessionId;
	return ODS::ERR::OK;
}

int DrvFTSQLHdaItem::HdaCommandHandler::HandleCloseSession(ODS::HdaFunction* pFunc, ODS::HdaCommandResult** ppResult)
{
	return ODS::ERR::OK;
}