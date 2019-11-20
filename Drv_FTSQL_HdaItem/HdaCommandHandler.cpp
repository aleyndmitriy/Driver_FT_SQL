#include"pch.h"
#include"HdaCommandHandler.h"
#include<OdsErr.h>
#include<HdaFunction.h>
#include <HdaFunctionResult.h>
#include<HdaFunctionTypes.h>
#include <HdaFunctionParam.h>
#include <HdaCommandParam.h>
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
	ODS::HdaFunction** pList = nullptr;
	int nCount = 0;
	int iRes = pCommand->GetFunctionList(&pList, &nCount);
	ExecuteCommand(pCommand, *pList, nCount, &resultList);
	for (std::vector<ODS::HdaFunctionResult*>::const_iterator iterRes = resultList.cbegin(); iterRes != resultList.cend(); iterRes++)
	{
		pResult->AddFunctionResult(*iterRes);
	}
	pCommand->DestroyFunctionList(pList, nCount);
	return ODS::ERR::OK;
}

int DrvFTSQLHdaItem::HdaCommandHandler::ExecuteCommand(ODS::HdaCommand* pCommand, ODS::HdaFunction* funcList, int listSize, std::vector<ODS::HdaFunctionResult*>* pResultList)
{
	std::map<int,std::vector<ODS::HdaFunction*> > requestMap;
	int res = AnalyzeCommand(pCommand, funcList, listSize, requestMap);
	if (res == ODS::ERR::OK) {
		std::map<int, std::vector<ODS::HdaFunction*> >::const_iterator itr = requestMap.find(ODS::HdaFunctionType::OPEN_SESSION);
		if (itr != requestMap.cend() && itr->second.size() == 1) {
			return HandleOpenSession(itr->second[0], pResultList);
		}
		itr = requestMap.find(ODS::HdaFunctionType::CLOSE_SESSION);
		if (itr != requestMap.cend() && itr->second.size() == 1) {
			return HandleCloseSession(itr->second[0], pResultList);
		}
		ODS::HdaCommandParam** ppCmdParamList = NULL;
		int nCount = 0;
		int iRes = pCommand->GetParamList(&ppCmdParamList, &nCount);
		ODS::Core::Uuid sessionId = ODS::Core::Uuid::EmptyUuid();
		SYSTEMTIME currTime;
		SYSTEMTIME start;
		SYSTEMTIME end;
		if (iRes == ODS::ERR::OK)
		{
			for (int i = 0; i < nCount; i++)
			{
				if (ppCmdParamList[i]->GetType() == ODS::HdaCommandParam::TYPE_TIME_PERIOD)
				{
					ODS::HdaCommandParamTimePeriod* pTimePeriod = (ODS::HdaCommandParamTimePeriod*)ppCmdParamList[i];

					if (ODS::ERR::OK != pTimePeriod->GetTimePeriod(&start, &end, &currTime))
					{
						pCommand->DestroyParamList(ppCmdParamList, nCount);
						return ODS::ERR::BAD_PARAM;
					}
				}
				if (ppCmdParamList[i]->GetType() == ODS::HdaCommandParam::TYPE_SESSION)
				{
					ODS::HdaCommandParamSession* pSession = (ODS::HdaCommandParamSession*)ppCmdParamList[i];
					sessionId = pSession->GetSessionId();
				}
			}
			pCommand->DestroyParamList(ppCmdParamList, nCount);
		}
		SYSTEMTIME startUtc, endUtc;
		ODS::TimeUtils::SysTimeLocalToUtc(start, &startUtc);
		ODS::TimeUtils::SysTimeLocalToUtc(end, &endUtc);
		std::map<int, std::vector<std::string> > queriesList;
		std::string sessionID;
		if (sessionId.IsEmpty()) {
			sessionID = m_database->OpenConnection();
			if (sessionID.empty()) {
				return ODS::ERR::DB_CONNECTION_FAILED;
			}
		}
		else {
			sessionID = std::string(sessionId.ToString().GetString());
		}
		CreateQueriesList(requestMap, queriesList, startUtc, endUtc, sessionID);
		if (queriesList.empty()) {
			return ODS::ERR::DB_NO_DATA;
		}
		ExecuteQueriesList(requestMap, queriesList, pResultList, startUtc, endUtc, sessionID);
		if (sessionId.IsEmpty()) {
			m_database->CloseConnectionWithUUID(sessionID);
		}
		return ODS::ERR::OK;
	}
	else {
		return res;
	}
}

int DrvFTSQLHdaItem::HdaCommandHandler::AnalyzeCommand(ODS::HdaCommand* pCommand, ODS::HdaFunction* list, int listSize, std::map<int, std::vector<ODS::HdaFunction*> >& requestMap)
{
	requestMap.clear();
	std::vector<ODS::HdaFunction*> funcList;
	for (int i = 0; i < listSize; i++) {
		funcList.push_back(list + i);
	}
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
	ODS::HdaFunctionParam** pParam = nullptr;
	int nCount = 0;
	int res = pFunc->GetParameterList(&pParam, &nCount);
	ODS::Core::Uuid sessionId;
	if (pParam != nullptr && nCount > 0) {
		for (int ind = 0; ind < nCount; ind++) {
			if ((*pParam + ind)->GetType() == ODS::HdaFunctionParam::TYPE_SESSION) {
				ODS::HdaFunctionParamSession* ptrParamSession = dynamic_cast<ODS::HdaFunctionParamSession*>(*pParam + ind);
				if (ptrParamSession != nullptr) {
					sessionId = ptrParamSession->GetSessionId();
				}
				break;
			}
		}
	}
	else {
		return ODS::ERR::BAD_PARAM;
	}
	pFunc->DestroyParameterList(pParam, nCount);
	std::string uuid = std::string(sessionId.ToString().GetString());
	m_database->CloseConnectionWithUUID(uuid);
	Log::GetInstance()->WriteInfoDebug(_T("CloseSession ok,  session id %s"), (LPCTSTR)sessionId.ToString());
	ODS::HdaFunctionResultSession* pSession = new ODS::HdaFunctionResultSession;
	pSession->SetContext(pFunc->GetContext());
	pSession->SetRc(ODS::ERR::OK);
	pResultList->push_back(pSession);
	return ODS::ERR::OK;
}

std::vector<std::string> DrvFTSQLHdaItem::HdaCommandHandler::BuildCmdValueList(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList, const std::map<std::string, TagItemRecord>& tags)
{
	std::vector<std::string> vec;
	for (std::vector<ODS::HdaFunction*>::const_iterator itr = rFuncList.cbegin(); itr != rFuncList.cend(); ++itr) {
		ParamValueList paramList = GetParameterValueList(*itr);
		std::map<std::string, TagItemRecord >::const_iterator tagItr = tags.find(paramList.GetAddress());
		if (tagItr != tags.cend()) {
			vec.push_back(m_database->CreateStatementValueList(std::move(paramList), startTime, endTime, tags));
		}
	}
	return vec;
}

std::vector<std::string> DrvFTSQLHdaItem::HdaCommandHandler::BuildCmdValueListConditions(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList, const std::map<std::string, TagItemRecord>& tags)
{
	std::vector<std::string> vec;
	for (std::vector<ODS::HdaFunction*>::const_iterator itr = rFuncList.cbegin(); itr != rFuncList.cend(); ++itr) {
		ParamValueList paramList = GetParameterValueList(*itr);
		std::map<std::string, TagItemRecord >::const_iterator tagItr = tags.find(paramList.GetAddress());
		if (tagItr != tags.cend()) {
			vec.push_back(m_database->CreateStatementConditionValueList(std::move(paramList), startTime, endTime, tags));
		}
	}
	return vec;
}

std::vector<std::string> DrvFTSQLHdaItem::HdaCommandHandler::BuildCmdFirstValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList, const std::map<std::string, TagItemRecord>& tags)
{
	std::vector<std::string> vec;
	for (std::vector<ODS::HdaFunction*>::const_iterator itr = rFuncList.cbegin(); itr != rFuncList.cend(); ++itr) {
		ParamValueList paramList = GetParameterValueList(*itr);
		std::map<std::string, TagItemRecord >::const_iterator tagItr = tags.find(paramList.GetAddress());
		if (tagItr != tags.cend()) {
			vec.push_back(m_database->CreateStatementFirstValue(std::move(paramList), startTime, endTime, tags));
		}
	}
	return vec;
}

std::vector<std::string> DrvFTSQLHdaItem::HdaCommandHandler::BuildCmdLastValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList, const std::map<std::string, TagItemRecord>& tags)
{
	std::vector<std::string> vec;
	for (std::vector<ODS::HdaFunction*>::const_iterator itr = rFuncList.cbegin(); itr != rFuncList.cend(); ++itr) {
		ParamValueList paramList = GetParameterValueList(*itr);
		std::map<std::string, TagItemRecord >::const_iterator tagItr = tags.find(paramList.GetAddress());
		if (tagItr != tags.cend()) {
			vec.push_back(m_database->CreateStatementLastValue(std::move(paramList), startTime, endTime, tags));
		}
		
	}
	return vec;
}

std::vector<std::string> DrvFTSQLHdaItem::HdaCommandHandler::BuildCmdMinValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList, const std::map<std::string, TagItemRecord>& tags)
{
	std::vector<std::string> vec;
	for (std::vector<ODS::HdaFunction*>::const_iterator itr = rFuncList.cbegin(); itr != rFuncList.cend(); ++itr) {
		ParamValueList paramList = GetParameterValueList(*itr);
		std::map<std::string, TagItemRecord >::const_iterator tagItr = tags.find(paramList.GetAddress());
		if (tagItr != tags.cend()) {
			vec.push_back(m_database->CreateStatementMinValue(std::move(paramList), startTime, endTime, tags));
		}
	}
	return vec;
}

std::vector<std::string> DrvFTSQLHdaItem::HdaCommandHandler::BuildCmdMaxValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList, const std::map<std::string, TagItemRecord>& tags)
{
	std::vector<std::string> vec;
	for (std::vector<ODS::HdaFunction*>::const_iterator itr = rFuncList.cbegin(); itr != rFuncList.cend(); ++itr) {
		ParamValueList paramList = GetParameterValueList(*itr);
		std::map<std::string, TagItemRecord >::const_iterator tagItr = tags.find(paramList.GetAddress());
		if (tagItr != tags.cend()) {
			vec.push_back(m_database->CreateStatementMaxValue(std::move(paramList), startTime, endTime, tags));
		}
	}
	return vec;
}

std::vector<std::string> DrvFTSQLHdaItem::HdaCommandHandler::BuildCmdSumValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList, const std::map<std::string, TagItemRecord>& tags)
{
	std::vector<std::string> vec;
	for (std::vector<ODS::HdaFunction*>::const_iterator itr = rFuncList.cbegin(); itr != rFuncList.cend(); ++itr) {
		ParamValueList paramList = GetParameterValueList(*itr);
		std::map<std::string, TagItemRecord >::const_iterator tagItr = tags.find(paramList.GetAddress());
		if (tagItr != tags.cend()) {
			vec.push_back(m_database->CreateStatementSumValue(std::move(paramList), startTime, endTime, tags));
		}
	}
	return vec;
}

std::vector<std::string> DrvFTSQLHdaItem::HdaCommandHandler::BuildCmdAvgValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList, const std::map<std::string, TagItemRecord>& tags)
{
	std::vector<std::string> vec;
	for (std::vector<ODS::HdaFunction*>::const_iterator itr = rFuncList.cbegin(); itr != rFuncList.cend(); ++itr) {
		ParamValueList paramList = GetParameterValueList(*itr);
		std::map<std::string, TagItemRecord >::const_iterator tagItr = tags.find(paramList.GetAddress());
		if (tagItr != tags.cend()) {
			vec.push_back(m_database->CreateStatementAvgValue(std::move(paramList), startTime, endTime, tags));
		}
	}
	return vec;
}

std::vector<std::string> DrvFTSQLHdaItem::HdaCommandHandler::BuildCmdTimeStampFirstValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList, const std::map<std::string, TagItemRecord>& tags)
{
	std::vector<std::string> vec;
	for (std::vector<ODS::HdaFunction*>::const_iterator itr = rFuncList.cbegin(); itr != rFuncList.cend(); ++itr) {
		ParamValueList paramList = GetParameterValueList(*itr);
		std::map<std::string, TagItemRecord >::const_iterator tagItr = tags.find(paramList.GetAddress());
		if (tagItr != tags.cend()) {
			vec.push_back(m_database->CreateStatementTimeStampFirstValue(std::move(paramList), startTime, endTime, tags));
		}
	}
	return vec;
}

std::vector<std::string> DrvFTSQLHdaItem::HdaCommandHandler::BuildCmdTimeStampLastValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList, const std::map<std::string, TagItemRecord>& tags)
{
	std::vector<std::string> vec;
	for (std::vector<ODS::HdaFunction*>::const_iterator itr = rFuncList.cbegin(); itr != rFuncList.cend(); ++itr) {
		ParamValueList paramList = GetParameterValueList(*itr);
		std::map<std::string, TagItemRecord >::const_iterator tagItr = tags.find(paramList.GetAddress());
		if (tagItr != tags.cend()) {
			vec.push_back(m_database->CreateStatementTimeStampLastValue(std::move(paramList), startTime, endTime, tags));
		}
	}
	return vec;
}

std::vector<std::string> DrvFTSQLHdaItem::HdaCommandHandler::BuildCmdTimeStampMaxValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList, const std::map<std::string, TagItemRecord>& tags)
{
	std::vector<std::string> vec;
	for (std::vector<ODS::HdaFunction*>::const_iterator itr = rFuncList.cbegin(); itr != rFuncList.cend(); ++itr) {
		ParamValueList paramList = GetParameterValueList(*itr);
		std::map<std::string, TagItemRecord >::const_iterator tagItr = tags.find(paramList.GetAddress());
		if (tagItr != tags.cend()) {
			vec.push_back(m_database->CreateStatementTimeStampMaxValue(std::move(paramList), startTime, endTime, tags));
		}
	}
	return vec;
}

std::vector<std::string> DrvFTSQLHdaItem::HdaCommandHandler::BuildCmdTimeStampMinValue(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList, const std::map<std::string, TagItemRecord>& tags)
{
	std::vector<std::string> vec;
	for (std::vector<ODS::HdaFunction*>::const_iterator itr = rFuncList.cbegin(); itr != rFuncList.cend(); ++itr) {
		ParamValueList paramList = GetParameterValueList(*itr);
		std::map<std::string, TagItemRecord >::const_iterator tagItr = tags.find(paramList.GetAddress());
		if (tagItr != tags.cend()) {
			vec.push_back(m_database->CreateStatementTimeStampMinValue(std::move(paramList), startTime, endTime, tags));
		}
	}
	return vec;
}

void DrvFTSQLHdaItem::HdaCommandHandler::CreateQueriesList(const std::map<int, std::vector<ODS::HdaFunction*> >& requestFunctions, std::map<int, std::vector<std::string> >& queriesList, const SYSTEMTIME& startUtc, const SYSTEMTIME& endUtc, const std::string& sessionId)
{
	queriesList.clear();
	std::map<std::string, TagItemRecord> tags = m_database->GetTags(sessionId,std::vector<std::string>());
	if (tags.empty()) {
		return;
	}
	std::pair<int, std::vector<std::string> > pair;
	std::pair<std::map<int, std::vector<std::string> >::iterator, bool > insertedPair;
	for (std::map<int, std::vector<ODS::HdaFunction*> >::const_iterator itr = requestFunctions.cbegin(); itr != requestFunctions.cend(); ++itr) {
		switch (itr->first) {
		case ODS::HdaFunctionType::VALUE_LIST:
			pair = std::make_pair<int, std::vector<std::string> >(int(ODS::HdaFunctionType::VALUE_LIST), BuildCmdValueList(startUtc, endUtc, itr->second, tags));
			break;
		case ODS::HdaFunctionType::VALUE_LIST_CONDITION:
			pair = std::make_pair<int, std::vector<std::string> >(int(ODS::HdaFunctionType::VALUE_LIST_CONDITION), BuildCmdValueListConditions(startUtc, endUtc, itr->second, tags));
			break;
		case ODS::HdaFunctionType::LAST_VALUE:
			pair = std::make_pair<int, std::vector<std::string> >(int(ODS::HdaFunctionType::LAST_VALUE), BuildCmdLastValue(startUtc, endUtc, itr->second, tags));
			break;
		case ODS::HdaFunctionType::FIRST_VALUE:
			pair = std::make_pair<int, std::vector<std::string> >(int(ODS::HdaFunctionType::FIRST_VALUE), BuildCmdFirstValue(startUtc, endUtc, itr->second, tags));
			break;
		case ODS::HdaFunctionType::TIMESTAMP_OF_LAST_VALUE:
			pair = std::make_pair<int, std::vector<std::string> >(int(ODS::HdaFunctionType::TIMESTAMP_OF_LAST_VALUE), BuildCmdTimeStampLastValue(startUtc, endUtc, itr->second, tags));
			break;
		case ODS::HdaFunctionType::TIMESTAMP_OF_FIRST_VALUE:
			pair = std::make_pair<int, std::vector<std::string> >(int(ODS::HdaFunctionType::TIMESTAMP_OF_FIRST_VALUE), BuildCmdTimeStampFirstValue(startUtc, endUtc, itr->second, tags));
			break;
		case ODS::HdaFunctionType::AVG_VALUE:
			pair = std::make_pair<int, std::vector<std::string> >(int(ODS::HdaFunctionType::AVG_VALUE), BuildCmdAvgValue(startUtc, endUtc, itr->second, tags));
			break;
		case ODS::HdaFunctionType::SUM_VALUE:
			pair = std::make_pair<int, std::vector<std::string> >(int(ODS::HdaFunctionType::SUM_VALUE), BuildCmdSumValue(startUtc, endUtc, itr->second, tags));
			break;
		case ODS::HdaFunctionType::MIN_VALUE:
			pair = std::make_pair<int, std::vector<std::string> >(int(ODS::HdaFunctionType::MIN_VALUE), BuildCmdMinValue(startUtc, endUtc, itr->second, tags));
			break;
		case ODS::HdaFunctionType::MAX_VALUE:
			pair = std::make_pair<int, std::vector<std::string> >(int(ODS::HdaFunctionType::MAX_VALUE), BuildCmdMaxValue(startUtc, endUtc, itr->second, tags));
			break;
		case ODS::HdaFunctionType::TIMESTAMP_OF_MINIMUM_VALUE:
			pair = std::make_pair<int, std::vector<std::string> >(int(ODS::HdaFunctionType::TIMESTAMP_OF_MINIMUM_VALUE), BuildCmdTimeStampMinValue(startUtc, endUtc, itr->second, tags));
			break;
		case ODS::HdaFunctionType::TIMESTAMP_OF_MAXIMUM_VALUE:
			pair = std::make_pair<int, std::vector<std::string> >(int(ODS::HdaFunctionType::TIMESTAMP_OF_MAXIMUM_VALUE), BuildCmdTimeStampMaxValue(startUtc, endUtc, itr->second, tags));
			break;
		default:
			break;
		}
		insertedPair = queriesList.insert(pair);
	}
}

void DrvFTSQLHdaItem::HdaCommandHandler::ExecuteQueriesList(const std::map<int, std::vector<ODS::HdaFunction*> >& requestFunctions, const std::map<int, std::vector<std::string> >& queriesList, std::vector<ODS::HdaFunctionResult*>* pResultList, const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::string& sessionId)
{
	SYSTEMTIME localStartDataTime = { 0 };
	SYSTEMTIME localEndDataTime = { 0 };
	ODS::TimeUtils::SysTimeUtcToLocal(startTime, &localStartDataTime);
	ODS::TimeUtils::SysTimeUtcToLocal(endTime, &localEndDataTime);
	for (std::map<int, std::vector<std::string> >::const_iterator queriesIterator = queriesList.cbegin(); queriesIterator != queriesList.cend(); ++queriesIterator) {
		size_t length = queriesIterator->second.size();
		for (size_t index = 0; index < length; ++index) {
			std::vector<Record> vec = m_database->GetRecords(queriesIterator->second.at(index), sessionId);
			if (vec.empty()) {
				continue;
			}
			std::map<int, std::vector<ODS::HdaFunction*> >::const_iterator funcIterator = requestFunctions.find(queriesIterator->first);
			if (funcIterator != requestFunctions.cend() && index < funcIterator->second.size()) {
				if (funcIterator->first == ODS::HdaFunctionType::VALUE_LIST_CONDITION) {
					ODS::HdaFunctionResultVLC* pFuncResult = new ODS::HdaFunctionResultVLC;
					std::vector<bool> conditions;
					std::vector<ODS::TvqListElementDescription> listDesc;
					pFuncResult->SetContext(funcIterator->second.at(index)->GetContext());
					
					for (std::vector<Record>::const_iterator itr = vec.cbegin(); itr != vec.cend(); ++itr) {
						bool condition = false;
						ODS::Tvq* tvq = CreateTvqFromRecord(*itr, &condition);
						if (itr == vec.cbegin()) {
							if (ODS::TimeUtils::SysTimeCompare(tvq->GetTimestampLoc(), localStartDataTime) < 0) {
								ODS::TvqListElementDescription desc;
								desc.m_nIndex = 0;
								desc.m_ulFlags = ODS::TvqListElementDescription::PREV_POINT;
								listDesc.push_back(desc);
							}
						}
						if (itr == vec.cend() - 1) {
							if (ODS::TimeUtils::SysTimeCompare(tvq->GetTimestampLoc(), localEndDataTime) > 0) {
								ODS::TvqListElementDescription desc;
								desc.m_nIndex = vec.size() - 1;
								desc.m_ulFlags = ODS::TvqListElementDescription::POST_POINT;
								listDesc.push_back(desc);
							}
						}
						pFuncResult->AddTvq(tvq);
						conditions.push_back(condition);
					}
					bool* conditionsList = new bool[conditions.size()];
					for (size_t conditionItr = 0; conditionItr < conditions.size(); ++conditionItr) {
						*(conditionsList + conditionItr) = conditions.at(conditionItr);
					}
					pFuncResult->SetConditionValueList(conditionsList, conditions.size());
					
					if (!listDesc.empty()) {
						ODS::TvqListElementDescription* desc = new ODS::TvqListElementDescription[listDesc.size()];
						for (size_t descItr = 0; descItr < listDesc.size(); descItr++) {
							(desc + descItr)->m_nIndex = listDesc.at(descItr).m_nIndex;
							(desc + descItr)->m_ulFlags = listDesc.at(descItr).m_ulFlags;
						}
						pFuncResult->SetTvqDescList(desc, listDesc.size());
					}
					pResultList->push_back(pFuncResult);
				}
				else {
					ODS::HdaFunctionResultValueList* pFuncResult = new ODS::HdaFunctionResultValueList;
					pFuncResult->SetContext(funcIterator->second.at(index)->GetContext());
					std::vector<ODS::TvqListElementDescription> listDesc;
					for (std::vector<Record>::const_iterator itr = vec.cbegin(); itr != vec.cend(); ++itr) {
						ODS::Tvq* tvq = CreateTvqFromRecord(*itr, nullptr);
						SYSTEMTIME tm = tvq->GetTimestampLoc();
						if (tm.wYear != 0) {
							if (itr == vec.cbegin()) {

								if (ODS::TimeUtils::SysTimeCompare(tvq->GetTimestampLoc(), localStartDataTime) < 0) {
									ODS::TvqListElementDescription desc;
									desc.m_nIndex = 0;
									desc.m_ulFlags = ODS::TvqListElementDescription::PREV_POINT;
									listDesc.push_back(desc);
								}
							}
							if (itr == vec.cend() - 1) {
								if (ODS::TimeUtils::SysTimeCompare(tvq->GetTimestampLoc(), localEndDataTime) > 0) {
									ODS::TvqListElementDescription desc;
									desc.m_nIndex = vec.size() - 1;
									desc.m_ulFlags = ODS::TvqListElementDescription::POST_POINT;
									listDesc.push_back(desc);
								}
							}
						}
						pFuncResult->AddTvq(tvq);
					}
					if (!listDesc.empty()) {
						ODS::TvqListElementDescription* desc = new ODS::TvqListElementDescription[listDesc.size()];
						for (size_t descItr = 0; descItr < listDesc.size(); descItr++) {
							(desc + descItr)->m_nIndex = listDesc.at(descItr).m_nIndex;
							(desc + descItr)->m_ulFlags = listDesc.at(descItr).m_ulFlags;
						}
						pFuncResult->SetTvqDescList(desc, listDesc.size());
					}
					pResultList->push_back(pFuncResult);
				}
			}
		}
	}
}

DrvFTSQLHdaItem::ParamValueList DrvFTSQLHdaItem::HdaCommandHandler::GetParameterValueList(const ODS::HdaFunction* pHdaFunc)
{
	
	std::vector<ODS::HdaFunctionParam*> paramList;
	ODS::HdaFunctionParam** pParam = nullptr;
	int nCount = 0;
	int res = pHdaFunc->GetParameterList(&pParam, &nCount);
	for (int i = 0; i < nCount; i++) {
		paramList.push_back(pParam[i]);
	}
	ODS::OdsString address;
	ODS::OdsString fullAddress;
	ODS::OdsString sql;
	bool prevPoint = false;
	bool postPoint = false;
	ODS::HdaFunctionParamLimit::LimitParam limit;
	limit.m_nLimitCount = 0;
	limit.m_nLimitOffset = 0;
	limit.m_nLimitSide = 0;
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
	pHdaFunc->DestroyParameterList(pParam, nCount);
	ParamValueList valList(std::string(address.GetString()), std::string(fullAddress.GetString()), std::string(sql.GetString()), prevPoint, postPoint, valueType, Limit(limit.m_nLimitSide, limit.m_nLimitOffset, limit.m_nLimitCount));
	return valList;
}

ODS::Tvq* DrvFTSQLHdaItem::HdaCommandHandler::CreateTvqFromRecord(const Record& record, bool* condition) const
{
	VARIANT vValue;
	std::string str;
	const TIMESTAMP_STRUCT* timeStampStruct = nullptr;
	SYSTEMTIME dataTime = { 0 };
	//SYSTEMTIME utcDataTime = { 0 };
	SYSTEMTIME localDataTime = { 0 };
	float val = 0.0;
	WORD millisec = 0;
	ODS::Tvq* tvq = new ODS::Tvq();
	for (Record::const_iterator itr = record.cbegin(); itr != record.cend(); ++itr) {
		switch (itr->second.first)
		{
		case SQL_FLOAT:
			::VariantInit(&vValue);
			vValue.vt = VT_R8;
			vValue.dblVal = std::stod(itr->second.second);
			tvq->SetValue(vValue);
			::VariantClear(&vValue);
			break;
		case SQL_C_FLOAT:
			::VariantInit(&vValue);
			vValue.vt = VT_R8;
			val = std::stof(itr->second.second);
			vValue.dblVal = val;
			tvq->SetValue(vValue);
			::VariantClear(&vValue);
			break;
		case SQL_C_SHORT:
		case SQL_C_TINYINT:
			if (!itr->second.second.empty() && itr->second.second.size() > 0) {
				millisec = std::stoi(itr->second.second);
			}
			break;
		case SQL_C_CHAR:
			tvq->SetValue(ODS::Data::Value(itr->second.second.c_str()));
			break;
		case SQL_C_TYPE_TIMESTAMP:
			if (itr->first == std::string(TAG_TABLE_COLUMN_DATE_TIME)) {
				timeStampStruct = reinterpret_cast<const TIMESTAMP_STRUCT*>(itr->second.second.c_str());
				dataTime.wYear = timeStampStruct->year;
				dataTime.wMonth = timeStampStruct->month;
				dataTime.wDay = timeStampStruct->day;
				dataTime.wHour = timeStampStruct->hour;
				dataTime.wMinute = timeStampStruct->minute;
				dataTime.wSecond = timeStampStruct->second;
			}
			else if (itr->first == std::string(TAG_TABLE_COLUMN_CONDITION_DATE_TIME) && condition != nullptr) {
				if (itr->second.second.empty() == false) {
					*condition = true;
				}
			}
			break;
		default:
			break;
		}
	}
	dataTime.wMilliseconds = millisec;
	//ODS::OdbcLib::ConvertTimestampStructToSysTime(dataTime, &utcDataTime);
	ODS::TimeUtils::SysTimeUtcToLocal(dataTime, &localDataTime);
	if (localDataTime.wYear != 0) {
		tvq->SetTimestamp(&localDataTime);
	}
	tvq->SetQuality(ODS::Tvq::QUALITY_GOOD);
	return tvq;
	
}