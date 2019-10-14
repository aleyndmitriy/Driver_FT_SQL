#pragma once
#include<memory>
#include<functional>
#include <Interface\IAbstractUIFacrory.h>
#include <CommonUI/IDbBrowser.h>
#include"ConnectionAttributes.h"
#include"SQLServerDatabaseEngine.h"
// Диалоговое окно DlgConnectionSqlSvr

class DlgConnectionSqlSvr :  public CDialogEx
{
	DECLARE_DYNAMIC(DlgConnectionSqlSvr)

public:
	DlgConnectionSqlSvr(std::function<ODS::UI::IAbstractUIFacrory * (void)> uiFactiryGetter, std::shared_ptr<DrvFTSQLHdaItem::ConnectionAttributes> attributes, CWnd* pParent = nullptr);   // стандартный конструктор
	virtual ~DlgConnectionSqlSvr();

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CONNECTION_SQL_SVR };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV

	DECLARE_MESSAGE_MAP()
private:
	std::function<ODS::UI::IAbstractUIFacrory * (void)> m_uiFactoryGetter;
	std::unique_ptr<ODS::Resources::ISqlBrowser> m_sqlBrowser;
	std::shared_ptr<DrvFTSQLHdaItem::ConnectionAttributes> m_connectAttributes;
	std::shared_ptr<DrvFTSQLHdaItem::IDatabaseEngine> m_database;
	CComboBox m_cbServer;
	CComboBox m_cbAuth;
	CEdit m_editUserName;
	CEdit m_editPassword;
	CComboBox m_cbDatabase;
	CEdit m_editDataQuality;
	CEdit m_editDays;
	BOOL OnInitDialog() override;
	void LoadDatabasesList(const std::vector<std::string>& databases);
	void WarningMessage(std::string message);
	void ErrorMessage(std::string message);
	void StartLoading();
	void StopLoading();
	void ConnectToServer();
	void CheckConnectToDatabase();
	void ReadAttributes();
public:
	afx_msg void OnCbnDropdownComboServerName();
	afx_msg void OnCbnSelchangeComboAuthType();
	afx_msg void OnCbnDropdownComboDatabase();
	afx_msg void OnBnClickedButtonTestconnection();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnCbnSelendokComboServerName();
};
