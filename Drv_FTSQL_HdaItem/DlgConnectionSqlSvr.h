#pragma once


// Диалоговое окно DlgConnectionSqlSvr

class DlgConnectionSqlSvr : public CDialogEx
{
	DECLARE_DYNAMIC(DlgConnectionSqlSvr)

public:
	DlgConnectionSqlSvr(CWnd* pParent = nullptr);   // стандартный конструктор
	virtual ~DlgConnectionSqlSvr();

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CONNECTION_SQL_SVR };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV

	DECLARE_MESSAGE_MAP()
};
