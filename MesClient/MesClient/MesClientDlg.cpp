
// MesClientDlg.cpp: файл реализации
//

#include "pch.h"
#include "framework.h"
#include "MesClient.h"
#include "MesClientDlg.h"
#include "afxdialogex.h"
#include <winsock2.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DEFAULT_NAME "Alice"
#define DEFAULT_SERVER "localhost"
#define DEFAULT_PORT 666
#define DEFAULT_BUFFER 2048
CMesClientDlg* dlg;
HWND   hWnd_LB;  // Для вывода в других потоках.
UINT Client(PVOID lp);//главная функция рабочего потока.

// Диалоговое окно CAboutDlg используется для описания сведений о приложении

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV

// Реализация
protected:
	DECLARE_MESSAGE_MAP()
public:
//	virtual HRESULT accDoDefaultAction(VARIANT varChild);
//	void SetConnected(bool IsConnected);
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// Диалоговое окно CMesClientDlg



CMesClientDlg::CMesClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MESCLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMesClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTBOX, m_ListBox);
}

BEGIN_MESSAGE_MAP(CMesClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CONNECT, &CMesClientDlg::OnClickedConnect)
	ON_BN_CLICKED(IDC_SEND, &CMesClientDlg::OnClickedSend)
END_MESSAGE_MAP()


// Обработчики сообщений CMesClientDlg

BOOL CMesClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Добавление пункта "О программе..." в системное меню.

	// IDM_ABOUTBOX должен быть в пределах системной команды.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Задает значок для этого диалогового окна.  Среда делает это автоматически,
	//  если главное окно приложения не является диалоговым
	SetIcon(m_hIcon, TRUE);			// Крупный значок
	SetIcon(m_hIcon, FALSE);		// Мелкий значок
	dlg = this;
	// TODO: добавьте дополнительную инициализацию
	char Str[128];

	GetDlgItem(IDC_SERVER)->SetWindowText(DEFAULT_SERVER);
	GetDlgItem(IDC_NAME)->SetWindowText(DEFAULT_NAME);
	GetDlgItem(IDC_NAME)->EnableWindow(TRUE);
	sprintf_s(Str, sizeof(Str), "%d", DEFAULT_PORT);
	GetDlgItem(IDC_PORT)->SetWindowText(Str);
	SetConnected(false);
	hWnd_LB = GetDlgItem(IDC_LISTBOX)->m_hWnd;

	return TRUE;  // возврат значения TRUE, если фокус не передан элементу управления
}

void CMesClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// При добавлении кнопки свертывания в диалоговое окно нужно воспользоваться приведенным ниже кодом,
//  чтобы нарисовать значок.  Для приложений MFC, использующих модель документов или представлений,
//  это автоматически выполняется рабочей областью.

void CMesClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // контекст устройства для рисования

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Выравнивание значка по центру клиентского прямоугольника
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Нарисуйте значок
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// Система вызывает эту функцию для получения отображения курсора при перемещении
//  свернутого окна.
HCURSOR CMesClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMesClientDlg::SetConnected(bool Connected)
{
	// TODO: Добавьте сюда код реализации.
	m_IsConnected = Connected;

	//Делаем активными кнопку отправки сообщения и поле ввода сообщения
	GetDlgItem(IDC_SEND)->EnableWindow(Connected);
	GetDlgItem(IDC_MESSAGE)->EnableWindow(Connected);
	GetDlgItem(IDC_NAME)->EnableWindow(!Connected);
	GetDlgItem(IDC_SERVER)->EnableWindow(!Connected);
	GetDlgItem(IDC_PORT)->EnableWindow(!Connected);
	GetDlgItem(IDC_CONNECT)->EnableWindow(!Connected);

}


void CMesClientDlg::OnClickedConnect()
{
	// TODO: добавьте свой код обработчика уведомлений
	char ServerNum[128];// Имя или IP-адрес сервера
	int iPort;// Порт

	WSADATA	wsd;

	struct sockaddr_in 	server;
	struct hostent* host = NULL;

	char Str[256];

	GetDlgItem(IDC_SERVER)->GetWindowText(ServerNum, sizeof(ServerNum));
	GetDlgItem(IDC_PORT)->GetWindowText(Str, sizeof(Str));
	iPort = atoi(Str);
	if (iPort <= 0 || iPort >= 0x10000)
	{
		m_ListBox.AddString((LPTSTR)"Неправильно введён порт.");
		return;
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		m_ListBox.AddString((LPTSTR)"Ошибка загрузки Winsock library!");
		return;
	}

	//Создание сокета
	m_sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_sClient == INVALID_SOCKET)
	{
		sprintf_s(Str, sizeof(Str),
			"При вызове socket() произошла ошибка: %d\n", WSAGetLastError());
		m_ListBox.AddString((LPTSTR)Str);
		return;
	}
	server.sin_family = AF_INET;
	server.sin_port = htons(iPort);
	server.sin_addr.s_addr = inet_addr(ServerNum);

	if (server.sin_addr.s_addr == INADDR_NONE)
	{
		host = gethostbyname(ServerNum);
		if (host == NULL)
		{
			sprintf_s(Str, sizeof(Str),
				"Не возможно подключиться к серверу: %s", ServerNum);
			m_ListBox.AddString((LPTSTR)Str);
			return;
		}
		CopyMemory(&server.sin_addr, host->h_addr_list[0], host->h_length);
	}
	if (connect(m_sClient, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		sprintf_s(Str, sizeof(Str),
			"При вызове connect() произошла ошибка %d", WSAGetLastError());
		m_ListBox.AddString((LPTSTR)Str);
		return;
	}
	SetConnected(true);
	GetDlgItem(IDC_CURRENT_CONN)->SetWindowText("Подключён");
	/*
	* Запуск основного рабочего потока.
	*/
	AfxBeginThread(Client, NULL);
}


void CMesClientDlg::OnClickedSend()
{
	// TODO: добавьте свой код обработчика уведомлений
	char Message[1024], Name[512], sendMessage[2048], Str[256];		// Сообщение, имя для отправки
	BOOL bSendOnly = FALSE;	// Только отправка данных
	
	int ret;

	char Buffer[DEFAULT_BUFFER];

	GetDlgItem(IDC_MESSAGE)->GetWindowText(Message, sizeof(Message));
	GetDlgItem(IDC_NAME)->GetWindowText(Name, sizeof(Name));
	sprintf_s(sendMessage, sizeof(sendMessage), "[%s]: %s", Name, Message);
	// Отправка данных 
	//
	ret = send(m_sClient, sendMessage, strlen(sendMessage), 0);
	if (ret == SOCKET_ERROR)
	{
		sprintf_s(Str, sizeof(Str), "send() failed: %d", WSAGetLastError());
		m_ListBox.AddString((LPTSTR)Str);
		return;
	}
	GetDlgItem(IDC_MESSAGE)->SetWindowText("");
}

UINT Client(LPVOID lp) 
{
	int ret;

	char recMessage[DEFAULT_BUFFER];

	char Str[256];

	CListBox* pLB = (CListBox*)(CListBox::FromHandle(hWnd_LB));
	while (true)
	{
		ret = recv(dlg->m_sClient, recMessage, DEFAULT_BUFFER, 0);
		if (ret == 0)	// Корректное завершение
			continue;
		else if (ret == SOCKET_ERROR)
		{
			sprintf_s(Str, sizeof(Str), "recv() failed: %d", WSAGetLastError());
			pLB->AddString((LPTSTR)Str);
			return 1;
		}

		CString tmp(recMessage);
		tmp.SetAt(ret, 0);
		pLB->AddString(tmp);
	}
}
