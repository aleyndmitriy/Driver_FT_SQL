// DlgConnectionSqlSvr.cpp: файл реализации
//

#include "pch.h"
#include "Drv_FTSQL_HdaItem.h"
#include "DlgConnectionSqlSvr.h"
#include "afxdialogex.h"


// Диалоговое окно DlgConnectionSqlSvr

IMPLEMENT_DYNAMIC(DlgConnectionSqlSvr, CDialogEx)

DlgConnectionSqlSvr::DlgConnectionSqlSvr(std::function<ODS::UI::IAbstractUIFacrory * (void)> uiFactiryGetter, std::shared_ptr<DrvFTSQLHdaItem::ConnectionAttributes> attributes, CWnd* pParent)
	: CDialogEx(IDD_CONNECTION_SQL_SVR, pParent), m_uiFactoryGetter(uiFactiryGetter), m_sqlBrowser(nullptr), m_connectAttributes(attributes), m_databasesList()
{

}

DlgConnectionSqlSvr::~DlgConnectionSqlSvr()
{
	m_connectAttributes.reset();
	m_sqlBrowser.reset();
	m_databasesList.clear();
}

void DlgConnectionSqlSvr::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_SERVER_NAME, m_cbServer);
	DDX_Control(pDX, IDC_COMBO_AUTH_TYPE, m_cbAuth);
	DDX_Control(pDX, IDC_EDIT_USERNAME, m_editUserName);
	DDX_Control(pDX, IDC_EDIT_PASSWORD, m_editPassword);
	DDX_Control(pDX, IDC_COMBO_DATABASE, m_cbDatabase);
	DDX_Control(pDX, IDC_EDIT_DATA_QUALITY, m_editDataQuality);
	DDX_Control(pDX, IDC_EDIT_DAYS_BACK, m_editDays);
}


BEGIN_MESSAGE_MAP(DlgConnectionSqlSvr, CDialogEx)
	ON_CBN_DROPDOWN(IDC_COMBO_SERVER_NAME, &DlgConnectionSqlSvr::OnCbnDropdownComboServerName)
	ON_CBN_SELCHANGE(IDC_COMBO_AUTH_TYPE, &DlgConnectionSqlSvr::OnCbnSelchangeComboAuthType)
	ON_CBN_DROPDOWN(IDC_COMBO_DATABASE, &DlgConnectionSqlSvr::OnCbnDropdownComboDatabase)
	ON_BN_CLICKED(IDC_BUTTON_TESTCONNECTION, &DlgConnectionSqlSvr::OnBnClickedButtonTestconnection)
	ON_BN_CLICKED(IDCANCEL, &DlgConnectionSqlSvr::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &DlgConnectionSqlSvr::OnBnClickedOk)
	ON_CBN_SELENDOK(IDC_COMBO_SERVER_NAME, &DlgConnectionSqlSvr::OnCbnSelendokComboServerName)
END_MESSAGE_MAP()


// Обработчики сообщений DlgConnectionSqlSvr


void DlgConnectionSqlSvr::OnCbnDropdownComboServerName()
{
	m_cbServer.ResetContent();
	int pos = m_cbServer.AddString(TEXT("(local)"));
	m_cbServer.SetItemData(pos, 0);
	pos = m_cbServer.AddString(TEXT("<Browse for more...>"));
	m_cbServer.SetItemData(pos, 1);
	m_databasesList.clear();
}


void DlgConnectionSqlSvr::OnCbnSelchangeComboAuthType()
{
	int index = m_cbAuth.GetCurSel();
	if (!index) {
		m_editUserName.Clear();
		m_editPassword.Clear();
		m_editUserName.EnableWindow(FALSE);
		m_editPassword.EnableWindow(FALSE);
	}
	else {
		m_editUserName.EnableWindow(TRUE);
		m_editPassword.EnableWindow(TRUE);
	}
}


void DlgConnectionSqlSvr::OnCbnDropdownComboDatabase()
{
	// TODO: добавьте свой код обработчика уведомлений
}


void DlgConnectionSqlSvr::OnBnClickedButtonTestconnection()
{
	// TODO: добавьте свой код обработчика уведомлений
}


void DlgConnectionSqlSvr::OnBnClickedCancel()
{
	// TODO: добавьте свой код обработчика уведомлений
	CDialogEx::OnCancel();
}


void DlgConnectionSqlSvr::OnBnClickedOk()
{
	int index = m_cbServer.GetCurSel();
	TCHAR * server = (TCHAR*)m_cbServer.GetItemData(index);

	CDialogEx::OnOK();
}


void DlgConnectionSqlSvr::OnCbnSelendokComboServerName()
{
	int index = m_cbServer.GetCurSel();
	if (index) {
		m_cbAuth.SetCurSel(0);
		m_cbDatabase.ResetContent();
		m_editUserName.Clear();
		m_editPassword.Clear();
		m_editUserName.EnableWindow(FALSE);
		m_editPassword.EnableWindow(FALSE);
		ODS::OdsString server;
		m_sqlBrowser.reset(dynamic_cast<ODS::Resources::ISqlBrowser*>(m_uiFactoryGetter()->CreateSqlBrowser()));
		m_sqlBrowser->OpenSourcesBrowserDlg(this->m_hWnd, "Browse SQL Servers", server);
		if (server.IsEmpty()) {
			m_cbServer.SetCurSel(0);
		}
		else {
			int ind = m_cbServer.FindString(0, server.GetString());
			if (ind == CB_ERR) {
				ind = m_cbServer.AddString(server.GetString());
				m_cbServer.SetCurSel(ind);
			}
			else {
				m_cbServer.SetCurSel(ind);
			}
		}
	}
	else {
		m_cbServer.SetCurSel(0);
	}
	m_sqlBrowser.reset();
}
