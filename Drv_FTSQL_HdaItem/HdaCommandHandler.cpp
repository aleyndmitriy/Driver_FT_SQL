#include"pch.h"
#include"HdaCommandHandler.h"
#include<OdsErr.h>
#include<OdsCoreLib/HdaCommandHelper.h>
#include<OdsCoreLib/HdaFunctionHelper.h>
#include<HdaFunction.h>
#include <HdaFunctionResult.h>
#include<HdaFunctionTypes.h>
#include"Constants.h"
#include"Log.h"

DrvFTSQLHdaItem::HdaCommandHandler::HdaCommandHandler()
{

}

DrvFTSQLHdaItem::HdaCommandHandler::~HdaCommandHandler()
{

}

int DrvFTSQLHdaItem::HdaCommandHandler::Init(const ConnectionAttributes& pCfg)
{
	m_attributes = pCfg;
	m_database = std::make_unique<SQLServerTagRecordsDAO>(m_attributes, TAG_TABLE_NAME, TAG_FLOAT_VALUE_TABLE_NAME, TAG_STRING_VALUE_TABLE_NAME);
	return ODS::ERR::OK;
}

int DrvFTSQLHdaItem::HdaCommandHandler::Shut()
{
	m_database.reset();
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
	for (std::vector<ODS::HdaFunctionResult*>::const_iterator iterRes = resultList.cbegin(); iterRes != resultList.cend(); iterRes++)
	{
		pResult->AddFunctionResult(*iterRes);
	}
	return ODS::ERR::OK;
}

int DrvFTSQLHdaItem::HdaCommandHandler::ExecuteCommand(const ODS::HdaCommand* pCommand, std::vector<ODS::HdaFunctionResult*>* pResultList)
{
	std::map<int,std::vector<ODS::HdaFunction*> > requestMap;
	int res = AnalyzeCommand(pCommand, requestMap);
	if (res == ODS::ERR::OK) {
		std::map<int, std::vector<ODS::HdaFunction*> >::const_iterator itr = requestMap.find(ODS::HdaFunctionType::OPEN_SESSION);
		if (itr != requestMap.cend() && itr->second.size() == 1) {
			return HandleOpenSession(itr->second[0], pResultList);
		}
		itr = requestMap.find(ODS::HdaFunctionType::CLOSE_SESSION);
		if (itr != requestMap.cend() && itr->second.size() == 1) {
			return HandleCloseSession(itr->second[0], pResultList);
		}

		return ODS::ERR::OK;
	}
	else {
		return res;
	}
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
	if (ODS::HdaFunctionType::OPEN_SESSION == funcList[0]->GetType() || ODS::HdaFunctionType::CLOSE_SESSION == funcList[0]->GetType()) {
		int nFuncType = funcList[0]->GetType();
		std::pair<int, std::vector<ODS::HdaFunction*> > pair = std::make_pair<int, std::vector<ODS::HdaFunction*> >(std::move(nFuncType), std::vector<ODS::HdaFunction*>());
		std::pair<std::map<int, std::vector<ODS::HdaFunction*> >::iterator, bool > insertedPair = requestMap.insert(pair);
		insertedPair.first->second.push_back(funcList[0]);
		return ODS::ERR::OK;
	}
		
	for (std::vector<ODS::HdaFunction*>::const_iterator itr = funcList.cbegin(); itr != funcList.cend(); ++itr) {
		int nFuncType = (*itr)->GetType();
		std::pair<int, std::vector<ODS::HdaFunction*> > pair = std::make_pair<int, std::vector<ODS::HdaFunction*> >(std::move(nFuncType), std::vector<ODS::HdaFunction*>());
		std::pair<std::map<int, std::vector<ODS::HdaFunction*> >::iterator, bool > insertedPair = requestMap.insert(pair);
		insertedPair.first->second.push_back(*itr);
	}
	return ODS::ERR::OK;
}

int DrvFTSQLHdaItem::HdaCommandHandler::HandleOpenSession(ODS::HdaFunction* pFunc, std::vector<ODS::HdaFunctionResult*>* pResultList)
{
	ODS::HdaCommandResult* pRes = new ODS::HdaCommandResult;
	ODS::HdaFunctionResultSession* pSession = new ODS::HdaFunctionResultSession;
	pSession->SetContext(pFunc->GetContext());
	ODS::Core::Uuid sessionId;
	std::string uuid = std::string(sessionId.ToString().GetString());
	if (m_database->OpenConnectionWithUUID(uuid)) {
		pSession->SetSessionId(sessionId);
		pSession->SetRc(ODS::ERR::OK);
		Log::GetInstance()->WriteInfoDebug(_T("OpenSession ok,  session id %s"), (LPCTSTR)sessionId.ToString());
	}
	else {
		pSession->SetSessionId(ODS::Core::Uuid::EmptyUuid());
		pSession->SetRc(ODS::ERR::DB_CONNECTION_FAILED);
		Log::GetInstance()->WriteErrorDebug(_T("Connect failed, %s"), (LPCTSTR)sessionId.ToString());
	}
	pResultList->push_back(pSession);

	return ODS::ERR::OK;
}

int DrvFTSQLHdaItem::HdaCommandHandler::HandleCloseSession(ODS::HdaFunction* pFunc, std::vector<ODS::HdaFunctionResult*>* pResultList)
{
	ODS::HdaFunctionHelper fh(pFunc);
	ODS::Core::Uuid sessionId;
	fh.GetParamSessionId(&sessionId);
	std::string uuid = std::string(sessionId.ToString().GetString());
	m_database->CloseConnectionWithUUID(uuid);
	Log::GetInstance()->WriteInfoDebug(_T("CloseSession ok,  session id %s"), (LPCTSTR)sessionId.ToString());
	ODS::HdaCommandResult* pRes = new ODS::HdaCommandResult;
	ODS::HdaFunctionResultSession* pSession = new ODS::HdaFunctionResultSession;
	pSession->SetContext(pFunc->GetContext());
	pSession->SetRc(ODS::ERR::OK);
	pResultList->push_back(pSession);
	return ODS::ERR::OK;
}