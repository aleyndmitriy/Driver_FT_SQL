// DlgConnectionSqlSvr.cpp: файл реализации
//

#include "pch.h"
#include "Drv_FTSQL_HdaItem.h"
#include "DlgConnectionSqlSvr.h"
#include "afxdialogex.h"

// Диалоговое окно DlgConnectionSqlSvr

IMPLEMENT_DYNAMIC(DlgConnectionSqlSvr, CDialogEx)

DlgConnectionSqlSvr::DlgConnectionSqlSvr(std::function<ODS::UI::IAbstractUIFacrory * (void)> uiFactiryGetter, std::shared_ptr<DrvFTSQLHdaItem::ConnectionAttributes> attributes, CWnd* pParent)
	: CDialogEx(IDD_CONNECTION_SQL_SVR, pParent), m_uiFactoryGetter(uiFactiryGetter), m_sqlBrowser(nullptr),
	m_connectAttributes(attributes), m_database(std::make_shared<DrvFTSQLHdaItem::SQLServerDatabaseEngine>())
{
	
	
}

DlgConnectionSqlSvr::~DlgConnectionSqlSvr()
{
	m_connectAttributes.reset();
	m_sqlBrowser.reset();
	m_database.reset();
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


BOOL DlgConnectionSqlSvr::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	if (!m_connectAttributes->serverName.empty()) {
		int ind = m_cbServer.AddString(m_connectAttributes->serverName.c_str());
		m_cbServer.SetCurSel(ind);
	}
	if (!m_connectAttributes->databaseName.empty()) {
		int ind = m_cbDatabase.AddString(m_connectAttributes->databaseName.c_str());
		m_cbDatabase.SetCurSel(ind);
	}
	m_editUserName.SetSel(0, -1);
	m_editUserName.Clear();
	m_editPassword.SetSel(0, -1);
	m_editPassword.Clear();
	if (m_connectAttributes->isServerAuthentication) {
		m_cbAuth.SetCurSel(1);
		m_editUserName.EnableWindow(TRUE);
		m_editPassword.EnableWindow(TRUE);
		if (!m_connectAttributes->loginName.empty()) {
			m_editUserName.SetWindowTextA(m_connectAttributes->loginName.c_str());
		}
		if (!m_connectAttributes->password.empty()) {
			m_editPassword.SetWindowTextA(m_connectAttributes->password.c_str());
		}
	}
	else {
		m_cbAuth.SetCurSel(0);
		m_editUserName.EnableWindow(FALSE);
		m_editPassword.EnableWindow(FALSE);
	}

	return TRUE;
}
// Обработчики сообщений DlgConnectionSqlSvr


void DlgConnectionSqlSvr::OnCbnDropdownComboServerName()
{
	m_cbServer.ResetContent();
	int pos = m_cbServer.AddString(TEXT("(local)"));
	m_cbServer.SetItemData(pos, 0);
	pos = m_cbServer.AddString(TEXT("<Browse for more...>"));
	m_cbServer.SetItemData(pos, 1);
	m_cbDatabase.ResetContent();
}


void DlgConnectionSqlSvr::OnCbnSelchangeComboAuthType()
{
	int index = m_cbAuth.GetCurSel();
	if (!index) {
		m_editUserName.SetSel(0, -1);
		m_editUserName.Clear();
		m_editPassword.SetSel(0, -1);
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
	ConnectToServer();
}


void DlgConnectionSqlSvr::OnBnClickedButtonTestconnection()
{
	CheckConnectToDatabase();
}


void DlgConnectionSqlSvr::OnBnClickedCancel()
{
	// TODO: добавьте свой код обработчика уведомлений
	CDialogEx::OnCancel();
}

void DlgConnectionSqlSvr::ReadAttributes()
{
	CString str;
	int len = m_cbServer.GetWindowTextLengthA();
	m_cbServer.GetWindowTextA(str);
	m_connectAttributes->serverName = std::string(str.GetBuffer(len));
	str.ReleaseBuffer();
	str.Empty();
	int index = m_cbAuth.GetCurSel();
	if (!index) {
		m_connectAttributes->loginName.clear();
		m_connectAttributes->password.clear();
		m_connectAttributes->isServerAuthentication = false;
	}
	else {
		m_editUserName.GetWindowTextA(str);
		m_connectAttributes->loginName = std::string(str.GetBuffer());
		str.ReleaseBuffer();
		str.Empty();
		m_editPassword.GetWindowTextA(str);
		m_connectAttributes->password = std::string(str.GetBuffer());
		str.ReleaseBuffer();
		str.Empty();
		m_connectAttributes->isServerAuthentication = true;
	}
	len = m_cbDatabase.GetWindowTextLengthA();
	m_cbDatabase.GetWindowTextA(str);
	m_connectAttributes->databaseName = std::string(str.GetBuffer(len));
	str.ReleaseBuffer();
	str.Empty();
	m_editDataQuality.GetWindowTextA(str);
	m_connectAttributes->dataQuality = std::string(str.GetBuffer());
	str.ReleaseBuffer();
	str.Empty();
	m_editDays.GetWindowTextA(str);
	m_connectAttributes->daysBack = std::string(str.GetBuffer());
}

void DlgConnectionSqlSvr::OnBnClickedOk()
{
	ReadAttributes();
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

void DlgConnectionSqlSvr::LoadDatabasesList(const std::vector<std::string>& databases)
{
	m_cbDatabase.ResetContent();
	size_t index = 0;
	for (std::vector<std::string>::const_iterator itr = databases.cbegin(); itr != databases.cend(); ++itr)
	{
		int pos = m_cbDatabase.AddString(itr->c_str());
		m_cbDatabase.SetItemData(pos, index++);
	}
}

void DlgConnectionSqlSvr::WarningMessage(std::string message)
{
	MessageBox(TEXT(message.c_str()), "Warning", MB_ICONWARNING);
}

void DlgConnectionSqlSvr::ErrorMessage(std::string message)
{
	MessageBox(TEXT(message.c_str()), "Warning", MB_ICONSTOP);
}


void DlgConnectionSqlSvr::StartLoading()
{
	HCURSOR hCurs = LoadCursor(NULL, IDC_WAIT);
	SetCursor(hCurs);
}

void DlgConnectionSqlSvr::StopLoading()
{
	HCURSOR hCurs = LoadCursor(NULL, IDC_ARROW);
	SetCursor(hCurs);
}

void DlgConnectionSqlSvr::ConnectToServer()
{
	StartLoading();
	ReadAttributes();
	if (m_database->OpenConnectionIfNeeded(*m_connectAttributes)) {
		LoadDatabasesList(m_database->GetDatabasesList());
	}
	StopLoading();
}

void DlgConnectionSqlSvr::CheckConnectToDatabase()
{
	StartLoading();
	ReadAttributes();
	std::vector<std::string> databaseNames;
	LoadDatabasesList(databaseNames);
	m_database->CloseConnection();
		if (m_connectAttributes->driver.empty()) {
			m_connectAttributes->driver = std::string("SQL Server Native Client 11.0");
		}
		if (!m_connectAttributes->serverName.empty()) {
			if (!m_database->OpenConnectionIfNeeded(*m_connectAttributes)) {
				databaseNames.clear();
				LoadDatabasesList(databaseNames);
				StopLoading();
				ErrorMessage(std::string("Connection Test Failed!"));
			}
			else {
				databaseNames = m_database->GetDatabasesList();
				LoadDatabasesList(databaseNames);
				StopLoading();
				WarningMessage(std::string("Connection Test Succeed!"));
			}
		}
		else {
			StopLoading();
			ErrorMessage(std::string("Empty fields!"));
		}
}