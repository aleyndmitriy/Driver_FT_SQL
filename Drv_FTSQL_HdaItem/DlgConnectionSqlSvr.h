#pragma once

#include "resource.h"
#include <Interface\IAbstractUIFacrory.h>
#include <memory>
#include <functional>
#include <CommonUI\IDbBrowser.h>
#include <afxwin.h>

class DlgConnectionSqlSvr : public CDialog {
	DECLARE_DYNAMIC(DlgConnectionSqlSvr)
public:
	DlgConnectionSqlSvr() = delete;
	explicit DlgConnectionSqlSvr(std::function< ODS::UI::IAbstractUIFacrory * (void)> uiFactoryGetter, LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
	explicit DlgConnectionSqlSvr(std::function< ODS::UI::IAbstractUIFacrory * (void)> uiFactoryGetter, UINT nIDTemplate, CWnd* pParentWnd = NULL);

	enum { IDD = IDD_CONNECTION_SQL_SVR };
private:
	std::function<ODS::UI::IAbstractUIFacrory * (void)>  m_uiFactoryGetter;
};