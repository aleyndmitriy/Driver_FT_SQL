// DlgConnectionSqlSvr.cpp: файл реализации
//

#include "pch.h"
#include "Drv_FTSQL_HdaItem.h"
#include "DlgConnectionSqlSvr.h"
#include "afxdialogex.h"


// Диалоговое окно DlgConnectionSqlSvr

IMPLEMENT_DYNAMIC(DlgConnectionSqlSvr, CDialogEx)

DlgConnectionSqlSvr::DlgConnectionSqlSvr(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CONNECTION_SQL_SVR, pParent)
{

}

DlgConnectionSqlSvr::~DlgConnectionSqlSvr()
{
}

void DlgConnectionSqlSvr::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(DlgConnectionSqlSvr, CDialogEx)
END_MESSAGE_MAP()


// Обработчики сообщений DlgConnectionSqlSvr
