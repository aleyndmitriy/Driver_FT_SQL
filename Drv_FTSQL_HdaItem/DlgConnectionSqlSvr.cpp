// DlgConnectionSqlSvr.cpp: файл реализации
//

#include "pch.h"
#include "Drv_FTSQL_HdaItem.h"
#include "DlgConnectionSqlSvr.h"
#include "afxdialogex.h"


// Диалоговое окно DlgConnectionSqlSvr

IMPLEMENT_DYNAMIC(DlgConnectionSqlSvr, CDialogEx)

DlgConnectionSqlSvr::DlgConnectionSqlSvr(std::function<ODS::UI::IAbstractUIFacrory * (void)> uiFactiryGetter, std::shared_ptr<DrvFTSQLHdaItem::ConnectionAttributes> attributes, CWnd* pParent)
	: CDialogEx(IDD_CONNECTION_SQL_SVR, pParent), m_uiFactoryGetter(uiFactiryGetter),m_connectAttributes(attributes)
{

}

DlgConnectionSqlSvr::~DlgConnectionSqlSvr()
{
	m_connectAttributes.reset();
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
END_MESSAGE_MAP()


// Обработчики сообщений DlgConnectionSqlSvr


void DlgConnectionSqlSvr::OnCbnDropdownComboServerName()
{
	// TODO: добавьте свой код обработчика уведомлений
}


void DlgConnectionSqlSvr::OnCbnSelchangeComboAuthType()
{
	// TODO: добавьте свой код обработчика уведомлений
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
	// TODO: добавьте свой код обработчика уведомлений
	CDialogEx::OnOK();
}
