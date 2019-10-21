#include"pch.h"
#include"HdaCommandHandler.h"
#include<OdsErr.h>
#include<OdsCoreLib/HdaCommandHelper.h>
#include<OdsCoreLib/HdaFunctionHelper.h>
#include<HdaFunction.h>
#include <HdaFunctionResult.h>
#include<HdaFunctionTypes.h>
#include <HdaFunctionParam.h>
#include"Constants.h"
#include"Log.h"
#include"ParamUtils.h"
#include<OdsCoreLib/TimeUtils.h>

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
		ODS::HdaCommandHelper hdaCmdHelper((ODS::HdaCommand*)pCommand);
		ODS::Core::Uuid sessionId;
		hdaCmdHelper.GetSessionId(&sessionId);
		std::vector<std::string> queries;
		std::string query;
		for (itr = requestMap.cbegin(); itr != requestMap.cend(); ++itr) {
			switch (itr->first) {
			case ODS::HdaFunctionType::VALUE_LIST:
				query = BuildCmdValueList(pCommand, itr->second);
				if (!query.empty()) {
					queries.push_back(query);
				}
				break;
			case ODS::HdaFunctionType::VALUE_LIST_CONDITION:
				query = BuildCmdValueListConditions(pCommand, itr->second);
				if (!query.empty()) {
					queries.push_back(query);
				}
				break;
			case ODS::HdaFunctionType::LAST_VALUE:
				query = BuildCmdLastValue(pCommand, itr->second);
				if (!query.empty()) {
					queries.push_back(query);
				}
				break;
			case ODS::HdaFunctionType::FIRST_VALUE:
				query = BuildCmdFirstValue(pCommand, itr->second);
				if (!query.empty()) {
					queries.push_back(query);
				}
				break;
			case ODS::HdaFunctionType::TIMESTAMP_OF_LAST_VALUE:
				query = BuildCmdTimeStampLastValue(pCommand, itr->second);
				if (!query.empty()) {
					queries.push_back(query);
				}
				break;
			case ODS::HdaFunctionType::TIMESTAMP_OF_FIRST_VALUE:
				query = BuildCmdTimeStampFirstValue(pCommand, itr->second);
				if (!query.empty()) {
					queries.push_back(query);
				}
				break;
			case ODS::HdaFunctionType::AVG_VALUE:
				query = BuildCmdAvgValue(pCommand, itr->second);
				if (!query.empty()) {
					queries.push_back(query);
				}
				break;
			case ODS::HdaFunctionType::SUM_VALUE:
				query = BuildCmdSumValue(pCommand, itr->second);
				if (!query.empty()) {
					queries.push_back(query);
				}
				break;
			case ODS::HdaFunctionType::MIN_VALUE:
				query = BuildCmdMinValue(pCommand, itr->second);
				if (!query.empty()) {
					queries.push_back(query);
				}
				break;
			case ODS::HdaFunctionType::MAX_VALUE:
				query = BuildCmdMaxValue(pCommand, itr->second);
				if (!query.empty()) {
					queries.push_back(query);
				}
				break;
			case ODS::HdaFunctionType::TIMESTAMP_OF_MINIMUM_VALUE:
				query = BuildCmdTimeStampMinValue(pCommand, itr->second);
				if (!query.empty()) {
					queries.push_back(query);
				}
				break;
			case ODS::HdaFunctionType::TIMESTAMP_OF_MAXIMUM_VALUE:
				query = BuildCmdTimeStampMaxValue(pCommand, itr->second);
				if (!query.empty()) {
					queries.push_back(query);
				}
				break;
			default:
				break;
			}
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

std::string DrvFTSQLHdaItem::HdaCommandHandler::BuildCmdValueList(const ODS::HdaCommand* pCommand, const std::vector<ODS::HdaFunction*>& rFuncList)
{
	ODS::HdaCommandHelper ch((ODS::HdaCommand*)pCommand);
	SYSTEMTIME currTime;
	SYSTEMTIME start;
	SYSTEMTIME end;
	ch.GetTimePeriod(&start, &end, &currTime);
	SYSTEMTIME startUtc, endUtc;
	ODS::TimeUtils::SysTimeLocalToUtc(start, &startUtc);
	ODS::TimeUtils::SysTimeLocalToUtc(end, &endUtc);

	for (std::vector<ODS::HdaFunction*>::const_iterator itr = rFuncList.cbegin(); itr != rFuncList.cend(); ++itr) {

	}
	return std::string();
}

std::string DrvFTSQLHdaItem::HdaCommandHandler::BuildCmdValueListConditions(const ODS::HdaCommand* pCommand, const std::vector<ODS::HdaFunction*>& rFuncList)
{
	return std::string();
}

std::string DrvFTSQLHdaItem::HdaCommandHandler::BuildCmdFirstValue(const ODS::HdaCommand* pCommand, const std::vector<ODS::HdaFunction*>& rFuncList)
{
	return std::string();
}

std::string DrvFTSQLHdaItem::HdaCommandHandler::BuildCmdLastValue(const ODS::HdaCommand* pCommand, const std::vector<ODS::HdaFunction*>& rFuncList)
{
	return std::string();
}

std::string DrvFTSQLHdaItem::HdaCommandHandler::BuildCmdMinValue(const ODS::HdaCommand* pCommand, const std::vector<ODS::HdaFunction*>& rFuncList)
{
	return std::string();
}

std::string DrvFTSQLHdaItem::HdaCommandHandler::BuildCmdMaxValue(const ODS::HdaCommand* pCommand, const std::vector<ODS::HdaFunction*>& rFuncList)
{
	return std::string();
}

std::string DrvFTSQLHdaItem::HdaCommandHandler::BuildCmdSumValue(const ODS::HdaCommand* pCommand, const std::vector<ODS::HdaFunction*>& rFuncList)
{
	return std::string();
}

std::string DrvFTSQLHdaItem::HdaCommandHandler::BuildCmdAvgValue(const ODS::HdaCommand* pCommand, const std::vector<ODS::HdaFunction*>& rFuncList)
{
	return std::string();
}

std::string DrvFTSQLHdaItem::HdaCommandHandler::BuildCmdTimeStampFirstValue(const ODS::HdaCommand* pCommand, const std::vector<ODS::HdaFunction*>& rFuncList)
{
	return std::string();
}

std::string DrvFTSQLHdaItem::HdaCommandHandler::BuildCmdTimeStampLastValue(const ODS::HdaCommand* pCommand, const std::vector<ODS::HdaFunction*>& rFuncList)
{
	return std::string();
}

std::string DrvFTSQLHdaItem::HdaCommandHandler::BuildCmdTimeStampMaxValue(const ODS::HdaCommand* pCommand, const std::vector<ODS::HdaFunction*>& rFuncList)
{
	return std::string();
}

std::string DrvFTSQLHdaItem::HdaCommandHandler::BuildCmdTimeStampMinValue(const ODS::HdaCommand* pCommand, const std::vector<ODS::HdaFunction*>& rFuncList)
{
	return std::string();
}

DrvFTSQLHdaItem::ParamValueList DrvFTSQLHdaItem::HdaCommandHandler::GetParameterValueList(const ODS::HdaFunction* pHdaFunc)
{
	ODS::HdaFunctionHelper fh((ODS::HdaFunction*)pHdaFunc);
	std::vector<ODS::HdaFunctionParam*> paramList;
	fh.GetParameterList(&paramList);
	ODS::OdsString address;
	ODS::OdsString fullAddress;
	ODS::OdsString sql;
	bool prevPoint = false;
	bool postPoint = false;
	ODS::HdaFunctionParamLimit::LimitParam limit;
	int nSpecPoint = 0;
	int valueType = 0;
	for (std::vector<ODS::HdaFunctionParam*>::const_iterator itr = paramList.cbegin(); itr != paramList.cend(); ++itr) {
		int nParamType = (*itr)->GetType();
		
		switch (nParamType)
		{
		case ODS::HdaFunctionParam::TYPE_ITEM_ADDRESS:
			ParamUtils::GetAddress(address, fullAddress, *itr);
			break;
		case ODS::HdaFunctionParam::TYPE_SQL:
			sql = ParamUtils::GetSql(*itr);
			break;
		case ODS::HdaFunctionParam::TYPE_LIMIT:
			((ODS::HdaFunctionParamLimit*)*itr)->GetLimit(&limit);
			break;
		case ODS::HdaFunctionParam::TYPE_OBJECT:
			valueType = ((ODS::HdaFunctionParamObject*) * itr)->GetType();
			break;
		case ODS::HdaFunctionParam::TYPE_SPEC_POINT:
			nSpecPoint = ((ODS::HdaFunctionParamSpecPoint*) * itr)->GetSpecPointFlag();
			if (ODS::HdaFunctionParamSpecPoint::POINT_TYPE_PREV == nSpecPoint)
				prevPoint = true;
			else if (ODS::HdaFunctionParamSpecPoint::POINT_TYPE_POST == nSpecPoint)
				postPoint = true;
			else if (ODS::HdaFunctionParamSpecPoint::POINT_TYPE_BOTH == nSpecPoint)
			{
				prevPoint = true;
				postPoint = true;
			}
			break;
		default:
			break;
		}
	}

	return ParamValueList(std::string(address.GetString()), std::string(fullAddress.GetString()), std::string(sql.GetString()), prevPoint, postPoint, valueType,Limit(limit.m_nLimitSide,limit.m_nLimitOffset,limit.m_nLimitCount));
}