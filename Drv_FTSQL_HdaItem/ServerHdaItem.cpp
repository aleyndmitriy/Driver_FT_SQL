#include "pch.h"
#include"ServerHdaItem.h"
#include <OdsErr.h>

void* DrvFTSQLHdaItem::ServerHdaItem::GetInterface(int nIfcId)
{
	return this;
}

int DrvFTSQLHdaItem::ServerHdaItem::Init(TCHAR* szCfgString)
{
	return ODS::ERR::OK;
}

int DrvFTSQLHdaItem::ServerHdaItem::Shut()
{
	return ODS::ERR::OK;
}

int DrvFTSQLHdaItem::ServerHdaItem::IsHdaFunctionSupported(int nFuncType)
{
	return ODS::ERR::OK;
}

int DrvFTSQLHdaItem::ServerHdaItem::Execute(ODS::HdaCommand* pCommand, ODS::HdaCommandResult** ppResult)
{
	return ODS::ERR::OK;
}

int DrvFTSQLHdaItem::ServerHdaItem::DestroyResult(ODS::HdaCommandResult* pResult)
{
	return ODS::ERR::OK;
}