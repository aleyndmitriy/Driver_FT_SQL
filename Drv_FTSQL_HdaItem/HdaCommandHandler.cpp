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
		SYSTEMTIME currTime;
		SYSTEMTIME start;
		SYSTEMTIME end;
		hdaCmdHelper.GetTimePeriod(&start, &end, &currTime);
		SYSTEMTIME startUtc, endUtc;
		ODS::TimeUtils::SysTimeLocalToUtc(start, &startUtc);
		ODS::TimeUtils::SysTimeLocalToUtc(end, &endUtc);
		std::map<int, std::vector<std::string> > queriesList;
		std::string sessionID = std::string(sessionId.ToString().GetString());
		CreateQueriesList(requestMap, queriesList, startUtc, endUtc, sessionID);
		if (queriesList.empty()) {
			return ODS::ERR::DB_NO_DATA;
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

std::vector<std::string> DrvFTSQLHdaItem::HdaCommandHandler::BuildCmdValueList(const SYSTEMTIME& startTime, const SYSTEMTIME& endTime, const std::vector<ODS::HdaFunction*>& rFuncList, const std::map<std::string, TagItemRecord>& tags)
{
	std::vector<std::string> vec;
	for (std::vector<ODS::HdaFunction*>::const_iterator itr = rFuncList.cbegin(); itr != rFuncList.cend(); ++itr) {
		ParamValueList paramList = GetParameterValueList(*itr);
		std::map<std::string, TagItemRecord >::const_iterator tagItr = tags.find(paramList.GetAddress());
		if (tagItr != tags.cend()) {

			vec.push_back(m_database->CreateStatementValueList(std::move(paramList), startTime, endTime, tagItr->second.GetTagDataType()));
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
			vec.push_back(m_database->CreateStatementConditionValueList(std::move(paramList), startTime, endTime, tagItr->second.GetTagDataType()));
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
			vec.push_back(m_database->CreateStatementFirstValue(std::move(paramList), startTime, endTime, tagItr->second.GetTagDataType()));
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
			vec.push_back(m_database->CreateStatementLastValue(std::move(paramList), startTime, endTime, tagItr->second.GetTagDataType()));
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
			vec.push_back(m_database->CreateStatementMinValue(std::move(paramList), startTime, endTime, tagItr->second.GetTagDataType()));
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
			vec.push_back(m_database->CreateStatementMaxValue(std::move(paramList), startTime, endTime, tagItr->second.GetTagDataType()));
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
			vec.push_back(m_database->CreateStatementSumValue(std::move(paramList), startTime, endTime, tagItr->second.GetTagDataType()));
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
			vec.push_back(m_database->CreateStatementAvgValue(std::move(paramList), startTime, endTime, tagItr->second.GetTagDataType()));
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
			vec.push_back(m_database->CreateStatementTimeStampFirstValue(std::move(paramList), startTime, endTime, tagItr->second.GetTagDataType()));
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
			vec.push_back(m_database->CreateStatementTimeStampLastValue(std::move(paramList), startTime, endTime, tagItr->second.GetTagDataType()));
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
			vec.push_back(m_database->CreateStatementTimeStampMaxValue(std::move(paramList), startTime, endTime, tagItr->second.GetTagDataType()));
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
			vec.push_back(m_database->CreateStatementTimeStampMinValue(std::move(paramList), startTime, endTime, tagItr->second.GetTagDataType()));
		}
	}
	return vec;
}

void DrvFTSQLHdaItem::HdaCommandHandler::CreateQueriesList(const std::map<int, std::vector<ODS::HdaFunction*> >& requestFunctions, std::map<int, std::vector<std::string> >& queriesList, const SYSTEMTIME& startUtc, const SYSTEMTIME& endUtc, const std::string& sessionId)
{
	queriesList.clear();
	std::map<std::string, TagItemRecord> tags = m_database->GetTags(sessionId);
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

ODS::Tvq DrvFTSQLHdaItem::HdaCommandHandler::CreateTvqFromRecord(const Record& record) const
{
	VARIANT vValue;
	ODS::Data::Value value;
	std::string str;
	const TIMESTAMP_STRUCT* timeStampStruct = nullptr;
	SYSTEMTIME dataTime = { 0 };
	//SYSTEMTIME utcDataTime = { 0 };
	SYSTEMTIME localDataTime = { 0 };
	float val = 0.0;
	std::string miliStr;
	ODS::Tvq tvq;
	for (Record::const_iterator itr = record.cbegin(); itr != record.cend(); ++itr) {
		switch (itr->second.first)
		{
		case SQL_FLOAT:
			::VariantInit(&vValue);
			vValue.vt = VT_R8;
			vValue.dblVal = std::stod(itr->second.second);
			tvq.SetValue(vValue);
			::VariantClear(&vValue);
			break;
		case SQL_C_FLOAT:
			::VariantInit(&vValue);
			vValue.vt = VT_R8;
			val = std::stof(itr->second.second);
			vValue.dblVal = val;
			tvq.SetValue(vValue);
			::VariantClear(&vValue);
			break;
		case SQL_C_CHAR:
			tvq.SetValue(ODS::Data::Value(itr->second.second.c_str()));
			break;
		case SQL_C_TYPE_TIMESTAMP:
			timeStampStruct = reinterpret_cast<const TIMESTAMP_STRUCT*>(itr->second.second .c_str());
			dataTime.wYear = timeStampStruct->year;
			dataTime.wMonth = timeStampStruct->month;
			dataTime.wDay = timeStampStruct->day;
			dataTime.wHour = timeStampStruct->hour;
			dataTime.wMinute = timeStampStruct->minute;
			dataTime.wSecond = timeStampStruct->second;
			miliStr = std::to_string(timeStampStruct->fraction);
			if (miliStr.length() > 3) {
				miliStr.erase(3, miliStr.length() - 3);
			}
			dataTime.wMilliseconds = std::stoul(miliStr);
			//ODS::OdbcLib::ConvertTimestampStructToSysTime(dataTime, &utcDataTime);
			ODS::TimeUtils::SysTimeUtcToLocal(dataTime, &localDataTime);
			tvq.SetTimestamp(&localDataTime);
			break;
		default:
			break;
		}
	}
	tvq.SetQuality(ODS::Tvq::QUALITY_GOOD);
	return tvq;
	
}