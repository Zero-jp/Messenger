
// ServerDlg.cpp: файл реализации
//

#include "pch.h"
#include "framework.h"
#include "Server.h"
#include "ServerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include <WinSock2.h>;

//Значения по умолчанию
#define	PORT 666	//Номер порта.
#define DATA_BUFSIZE 9000	//Размер буфера.

int iPort = PORT;	//Порт для прослушивания подключений.

typedef struct _SOCKET_INFORMATION {
	CHAR Buffer[DATA_BUFSIZE];
	WSABUF DataBuf;
	SOCKET Socket;
	DWORD BytesSEND;
	DWORD BytesRECV;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;

/*
* Создание события и запись его в массив.
* Создаёт и связывает структуру SOCKET_INFORMATION с сокетом.
*/
BOOL CreateSocketInformation(SOCKET s, char* Str, CListBox* pLB);

/*
* Удаляет объкты создаваемые функцией CreateSocketInformation().
*/
void FreeSocketInformation(DWORD Event, char* Str, CListBox* pLB);

DWORD EventTotal = 0;//Колличество событий.
WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];//Массив с событиями.
LPSOCKET_INFORMATION SocketArray[WSA_MAXIMUM_WAIT_EVENTS];//Массив с сокетами.

HWND   hWnd_LB;  // Для вывода в других потоках.

UINT Server(PVOID lp);//главная функция рабочего потока.

// Диалоговое окно CServerDlg



CServerDlg::CServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTBOX, m_ListBox);
}

BEGIN_MESSAGE_MAP(CServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_START, &CServerDlg::OnClickedStart)
	ON_BN_CLICKED(IDC_CANCEL, &CServerDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// Обработчики сообщений CServerDlg

BOOL CServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Задает значок для этого диалогового окна.  Среда делает это автоматически,
	//  если главное окно приложения не является диалоговым
	SetIcon(m_hIcon, TRUE);			// Крупный значок
	SetIcon(m_hIcon, FALSE);		// Мелкий значок

	// TODO: добавьте дополнительную инициализацию

	/*
	* Добавление изначально введённого порта.
	*/
	char Str[128];

	sprintf_s(Str, sizeof(Str), "%d", iPort);
	GetDlgItem(IDC_PORT)->SetWindowText(Str);

	return TRUE;  // возврат значения TRUE, если фокус не передан элементу управления
}

// При добавлении кнопки свертывания в диалоговое окно нужно воспользоваться приведенным ниже кодом,
//  чтобы нарисовать значок.  Для приложений MFC, использующих модель документов или представлений,
//  это автоматически выполняется рабочей областью.

void CServerDlg::OnPaint()
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
HCURSOR CServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CServerDlg::OnClickedStart()
{
	// TODO: добавьте свой код обработчика уведомлений
	char Str[81];

	hWnd_LB = m_ListBox.m_hWnd;   // Для ListenThread
	GetDlgItem(IDC_PORT)->GetWindowText(Str, sizeof(Str));//Получение номера порта.
	iPort = atoi(Str);//Запоминание номера порта, как числа с плавующей точкой.

	//Проверка на корректный порт.
	if (iPort <= 0 || iPort >= 0x10000)
	{
		AfxMessageBox("Некорректно введён номер порта.");
		return;
	}

	/*
	* Запуск основного рабочего потока.
	*/
	AfxBeginThread(Server, NULL);

	/*
	* Делаем кнопку "Старт" неактивной, чтоб избежать повторного запуска.
	*/
	GetDlgItem(IDC_START)->EnableWindow(false);
}

/*
* Главная функция рабочего потока
*/
UINT Server(LPVOID lp)
{
	/*
	* Описание локальных переменных.
	*/
	SOCKET Listen;
	SOCKET Accept;
	SOCKADDR_IN InternetAddr;
	DWORD Event;
	WSANETWORKEVENTS NetworkEvents;
	WSADATA wsaData;
	DWORD Ret;
	DWORD Flags;
	DWORD RecvBytes;
	DWORD SendBytes;
	char  Str[200];
	CListBox* pLB = (CListBox*)(CListBox::FromHandle(hWnd_LB));

	if ((Ret = WSAStartup(0x0202, &wsaData)) != 0)
	{
		sprintf_s(Str, sizeof(Str), "При поытке обращения к WSAStartup() произошла ошибка %d", Ret);
		pLB->AddString(Str);
		return 1;
	}

	/*
	* Создание сокета.
	*/

	//Создание сокета.
	if ((Listen = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		sprintf_s(Str, sizeof(Str), "При поытке обращения к socket() произошла ошибка %d", WSAGetLastError());
		pLB->AddString(Str);
		return 1;
	}

	//Создание события и структуры сокета.
	CreateSocketInformation(Listen, Str, pLB);

	//Связывание сокета с нужными событиями.
	if (WSAEventSelect(Listen, EventArray[EventTotal - 1], FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR)
	{
		sprintf_s(Str, sizeof(Str), "При поытке обращения к WSAEventSelect() произошла ошибка %d", WSAGetLastError());
		pLB->AddString(Str);
		return 1;
	}

	/*
	* Привязывание сокета к интерфейсу и выставление его на прослушивание.
	*/
	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	InternetAddr.sin_port = htons(iPort);

	//Привязывание сокета к интерфейсу.
	if (bind(Listen, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		sprintf_s(Str, sizeof(Str), "При поытке обращения к bind() произошла ошибка %d", WSAGetLastError());
		pLB->AddString(Str);
		return 1;
	}

	//Выставление сокета на прослушивание.
	if (listen(Listen, 5))
	{
		sprintf_s(Str, sizeof(Str), "При поытке обращения к  listen() произошла ошибка %d", WSAGetLastError());
		pLB->AddString(Str);
		return 1;
	}

	/*
	* Обработка происзодящих событий.
	*/
	while (TRUE)
	{
		//Дожидаемся уведомления о событии на любом сокете.
		if ((Event = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE, WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
		{
			sprintf_s(Str, sizeof(Str), "При поытке обращения к WSAWaitForMultipleEvents произошла ошибка %d", WSAGetLastError());
			pLB->AddString(Str);
			return 1;
		}

		//Занесение информации о событии в структуру NetworkEvents.
		if (WSAEnumNetworkEvents(
			SocketArray[Event - WSA_WAIT_EVENT_0]->Socket, EventArray[Event - WSA_WAIT_EVENT_0], &NetworkEvents) == SOCKET_ERROR)
		{
			sprintf_s(Str, sizeof(Str), "При поытке обращения к WSAEnumNetworkEvents произошла ошибка %d", WSAGetLastError());
			pLB->AddString(Str);
			return 1;
		}

		//Обработка типа события.
		if (NetworkEvents.lNetworkEvents & FD_ACCEPT)
		{
			if (NetworkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
			{
				sprintf_s(Str, sizeof(Str), "При поытке обращения к FD_ACCEPT произошла ошибка %d", NetworkEvents.iErrorCode[FD_ACCEPT_BIT]);
				pLB->AddString(Str);
				break;
			}

			// Прием нового соединения и добавление его в списки сокетов и событий.
			if ((Accept = accept(SocketArray[Event - WSA_WAIT_EVENT_0]->Socket, NULL, NULL)) == INVALID_SOCKET)
			{
				sprintf_s(Str, sizeof(Str), "При поытке обращения к accept() произошла ошибка %d", WSAGetLastError());
				pLB->AddString(Str);
				break;
			}

			// Слишком много сокетов. Закрываем соединение.
			if (EventTotal > WSA_MAXIMUM_WAIT_EVENTS)
			{
				sprintf_s(Str, sizeof(Str), "Слишком много подключённых сокетов - закрытие сокета.");
				pLB->AddString(Str);
				closesocket(Accept);
				break;
			}

			CreateSocketInformation(Accept, Str, pLB);

			if (WSAEventSelect(Accept, EventArray[EventTotal - 1], FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
			{
				sprintf_s(Str, sizeof(Str), "При поытке обращения к WSAEventSelect() произошла ошибка %d", WSAGetLastError());
				pLB->AddString(Str);
				return 1;
			}

			sprintf_s(Str, sizeof(Str), "Сокет %d подключён", Accept);
			pLB->AddString(Str);
		}

		// Пытаемся читать или писать данные,  если произошло соответствующее событие
		if (NetworkEvents.lNetworkEvents & FD_READ || NetworkEvents.lNetworkEvents & FD_WRITE)
		{
			if (NetworkEvents.lNetworkEvents & FD_READ && NetworkEvents.iErrorCode[FD_READ_BIT] != 0)
			{
				sprintf_s(Str, sizeof(Str), "При поытке обращения к FD_READ произошла ошибка %d", NetworkEvents.iErrorCode[FD_READ_BIT]);
				pLB->AddString(Str);
				break;
			}

			if (NetworkEvents.lNetworkEvents & FD_WRITE && NetworkEvents.iErrorCode[FD_WRITE_BIT] != 0)
			{
				sprintf_s(Str, sizeof(Str), "При поытке обращения к FD_WRITE произошла ошибка %d", NetworkEvents.iErrorCode[FD_WRITE_BIT]);
				pLB->AddString(Str);
				break;
			}

			LPSOCKET_INFORMATION SocketInfo = SocketArray[Event - WSA_WAIT_EVENT_0];

			// Читаем данные только если приемный буфер пуст
			SocketInfo->DataBuf.buf = SocketInfo->Buffer;
			SocketInfo->DataBuf.len = DATA_BUFSIZE;

			Flags = 0;
			if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags, NULL, NULL) == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSAEWOULDBLOCK)
				{
					sprintf_s(Str, sizeof(Str), "При поытке обращения к WSARecv() произошла ошибка %d", WSAGetLastError());
					pLB->AddString(Str);
					FreeSocketInformation(Event - WSA_WAIT_EVENT_0, Str, pLB);
					return 1;
				}
			}
			else
			{
				SocketInfo->BytesRECV = RecvBytes;
				// Вывод сообщения, если требуется
				unsigned l = sizeof(Str) - 1;
				if (l > RecvBytes) l = RecvBytes;
				strncpy_s(Str, SocketInfo->Buffer, l);
				Str[l] = 0;
				pLB->AddString(Str);
			}
				

			// Отправка данных, если это возможно
			SocketInfo->BytesSEND = 0;
			if (SocketInfo->BytesRECV > SocketInfo->BytesSEND)
			{
				SocketInfo->DataBuf.buf = SocketInfo->Buffer + SocketInfo->BytesSEND;
				SocketInfo->DataBuf.len = SocketInfo->BytesRECV - SocketInfo->BytesSEND;

				for (int i = 1; i < EventTotal; i++) {
					if (WSASend(SocketArray[i]->Socket, &(SocketInfo->DataBuf), 1, &SendBytes, 0, NULL, NULL) == SOCKET_ERROR)
					{
						if (WSAGetLastError() != WSAEWOULDBLOCK)
						{
							sprintf_s(Str, sizeof(Str), "При поытке обращения к WSASend() произошла ошибка %d", WSAGetLastError());
							pLB->AddString(Str);
							FreeSocketInformation(Event - WSA_WAIT_EVENT_0, Str, pLB);
						}

						// Произошла ошибка WSAEWOULDBLOCK. 
						// Событие FD_WRITE будет отправлено, когда
						// в буфере будет больше свободного места
					}
					else
					{
						SocketArray[i]->BytesSEND += SendBytes;

						if (SocketArray[i]->BytesSEND == SocketArray[i]->BytesRECV)
						{
							SocketArray[i]->BytesSEND = 0;
							SocketArray[i]->BytesRECV = 0;
						}
					}
				}
			}
		}

		//Обработка разрыва соединения.(очистка)
		if (NetworkEvents.lNetworkEvents & FD_CLOSE)
		{
			sprintf_s(Str, sizeof(Str), "Закрытие информации сокета %d", SocketArray[Event - WSA_WAIT_EVENT_0]->Socket);
			pLB->AddString(Str);

			FreeSocketInformation(Event - WSA_WAIT_EVENT_0, Str, pLB);
		}
	}
	return 0;
}

//Создание сокета.
BOOL CreateSocketInformation(SOCKET s, char* Str, CListBox* pLB)
{
	// TODO: Добавьте сюда код реализации.
	LPSOCKET_INFORMATION SI;

	if ((EventArray[EventTotal] = WSACreateEvent()) ==
		WSA_INVALID_EVENT)
	{
		sprintf_s(Str, sizeof(Str), "При поытке обращения к WSACreateEvent() произошла ошибка %d", WSAGetLastError());
		pLB->AddString(Str);
		return FALSE;
	}

	if ((SI = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
		sizeof(SOCKET_INFORMATION))) == NULL)
	{
		sprintf_s(Str, sizeof(Str), "При поытке обращения к GlobalAlloc() произошла ошибка %d", GetLastError());
		pLB->AddString(Str);
		return FALSE;
	}

	// Подготовка структуры SocketInfo для использования.
	SI->Socket = s;
	SI->BytesSEND = 0;
	SI->BytesRECV = 0;

	SocketArray[EventTotal] = SI;
	EventTotal++;
	return(TRUE);
}

//Удаление событий и сокетов.
void FreeSocketInformation(DWORD Event, char* Str, CListBox* pLB)
{
	// TODO: Добавьте сюда код реализации.
	LPSOCKET_INFORMATION SI = SocketArray[Event];
	DWORD i;

	closesocket(SI->Socket);
	GlobalFree(SI);
	WSACloseEvent(EventArray[Event]);

	// Сжатие массивов сокетов и событий

	for (i = Event; i < EventTotal; i++)
	{
		EventArray[i] = EventArray[i + 1];
		SocketArray[i] = SocketArray[i + 1];
	}

	EventTotal--;
}



void CServerDlg::OnBnClickedCancel()
{
	// TODO: добавьте свой код обработчика уведомлений
	CDialogEx::OnCancel();
}
