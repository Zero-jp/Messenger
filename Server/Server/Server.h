
// Server.h: главный файл заголовка для приложения PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error "включить pch.h до включения этого файла в PCH"
#endif

#include "resource.h"		// основные символы


// CServerApp:
// Сведения о реализации этого класса: Server.cpp
//

class CServerApp : public CWinApp
{
public:
	CServerApp();

// Переопределение
public:
	virtual BOOL InitInstance();

// Реализация

	DECLARE_MESSAGE_MAP()
//	afx_msg void OnStart();
//	afx_msg void OnCancel();
};

extern CServerApp theApp;
