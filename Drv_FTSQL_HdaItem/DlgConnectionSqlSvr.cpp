#include "DlgConnectionSqlSvr.h"

DlgConnectionSqlSvr::DlgConnectionSqlSvr(std::function< ODS::UI::IAbstractUIFacrory * (void)> uiFactoryGetter, LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL):
	CDialog(lpszTemplateName,pParentWnd), m_uiFactoryGetter(uiFactoryGetter)
{

}

DlgConnectionSqlSvr::DlgConnectionSqlSvr(std::function< ODS::UI::IAbstractUIFacrory * (void)> uiFactoryGetter, UINT nIDTemplate, CWnd* pParentWnd = NULL):
	CDialog(nIDTemplate, pParentWnd), m_uiFactoryGetter(uiFactoryGetter)
{

}