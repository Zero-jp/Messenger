
// MesClient.h: главный файл заголовка для приложения PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error "включить pch.h до включения этого файла в PCH"
#endif

#include "resource.h"		// основные символы


// CMesClientApp:
// Сведения о реализации этого класса: MesClient.cpp
//

class CMesClientApp : public CWinApp
{
public:
	CMesClientApp();

// Переопределение
public:
	virtual BOOL InitInstance();

// Реализация

	DECLARE_MESSAGE_MAP()
};

extern CMesClientApp theApp;
