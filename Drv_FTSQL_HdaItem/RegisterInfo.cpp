#include "pch.h"
#include"RegisterInfo.h"
#include "Constants.h"

DrvFTSQLHdaItem::CRegisterInfo::CRegisterInfo()
{
	m_RegInfo.m_Version.m_nMajor = VERSION_MAJOR;
	m_RegInfo.m_Version.m_nMinor = VERSION_MINOR;
	memset(m_RegInfo.m_szObjKey, '\0', MAX_PATH);
	m_RegInfo.m_pObjCreationParam = nullptr;
	m_RegInfo.m_pFactory = nullptr;
	m_RegInfo.m_pPropertySet = new CPropertySet();
}

DrvFTSQLHdaItem::CRegisterInfo::~CRegisterInfo()
{
	m_RegInfo.m_Version.m_nMajor = VERSION_MAJOR;
	m_RegInfo.m_Version.m_nMinor = VERSION_MINOR;
	memset(m_RegInfo.m_szObjKey, '\0', MAX_PATH);
	m_RegInfo.m_pObjCreationParam = nullptr;
	m_RegInfo.m_pFactory = nullptr;
	delete m_RegInfo.m_pPropertySet;
}

DrvFTSQLHdaItem::CRegisterInfo::CRegisterInfo(const CRegisterInfo& src)
{
	m_RegInfo.m_Version.m_nMajor = src.m_RegInfo.m_Version.m_nMajor;
	m_RegInfo.m_Version.m_nMinor = src.m_RegInfo.m_Version.m_nMinor;
	_tcscpy_s(m_RegInfo.m_szObjKey, MAX_PATH, src.m_RegInfo.m_szObjKey);
	m_RegInfo.m_pObjCreationParam = src.m_RegInfo.m_pObjCreationParam;
	m_RegInfo.m_pFactory = src.m_RegInfo.m_pFactory;
	delete m_RegInfo.m_pPropertySet;
	m_RegInfo.m_pPropertySet = new CPropertySet();
}