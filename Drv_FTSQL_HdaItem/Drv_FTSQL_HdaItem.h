// Drv_FTSQL_HdaItem.h: основной файл заголовка для библиотеки DLL Drv_FTSQL_HdaItem
//

#pragma once

#ifndef __AFXWIN_H__
	#error "включить pch.h до включения этого файла в PCH"
#endif

#include "resource.h"		// основные символы


// CDrvFTSQLHdaItemApp
// Реализацию этого класса см. в файле Drv_FTSQL_HdaItem.cpp
//

class CDrvFTSQLHdaItemApp : public CWinApp
{
public:
	CDrvFTSQLHdaItemApp();

// Переопределение
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
