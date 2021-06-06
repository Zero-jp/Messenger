
// ServerDlg.h: файл заголовка
//

#pragma once

struct ThreadArgs {
	HANDLE sc;//дискриптер кеша сервера
	HWND hWnd;//дескриптер окна сервера
};

UINT Server(LPVOID lp);


// Диалоговое окно CServerDlg
class CServerDlg : public CDialogEx
{
// Создание
public:
	CServerDlg(CWnd* pParent = nullptr);	// стандартный конструктор

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// поддержка DDX/DDV


// Реализация
protected:
	HICON m_hIcon;

	// Созданные функции схемы сообщений
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	CListBox m_ListBox;
	afx_msg void OnClickedStart();
	bool CreateSocketInformation(SOCKET s, char* Str, CListBox* pLB);
	afx_msg void OnBnClickedCancel();
};
