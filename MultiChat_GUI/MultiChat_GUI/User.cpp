#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <ctime>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "resource.h"

#define BUFSIZE  512
#define NAMESIZE 10
#define IDSIZE 10
#define LOGIN "!@#*@#!"
#define CHANGENAME "!@#$!@#%"

char *multicastIP;
char *multicastPort;

#pragma region Function Declare
// ��ȭ���� ���ν���
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
// ���� ��Ʈ�� ��� �Լ�
void DisplayText(char *fmt, ...);
// ���� ��� �Լ�
void err_quit(char *msg);
void err_display(char *msg);
// ����� ���� ������ ���� �Լ�
int recvn(SOCKET s, char *buf, int len, int flags);
// ���� ��� ������ �Լ�
DWORD WINAPI ClientMain(LPVOID arg);
bool checkClassDIP(char *ip);
bool checkPort(char inputPort[]);
#pragma endregion
#pragma region Variable Declare
SOCKET sock; // ����
SOCKET receiveSock; // ����
struct ip_mreq mreq;
SOCKADDR_IN remoteaddr;  //���� ����ü
char buf[BUFSIZE + 1];	   // ������ �ۼ��� ����
HANDLE hReadEvent, hWriteEvent; // �����̺�Ʈ
HANDLE hLoginReadEvent, hLoginWriteEvent; // �α����̺�Ʈ
HWND hSendButton, hIPCheckButton, hPortCheckButton, hChangeNick, hLoginButton, hExitButton, hResisterNick,
hEditIP, hEditPort, hEditText, hShowText, hEditName; // ���� ��Ʈ��
#pragma endregion

bool IPcheck = false, Portcheck = false, NameCheck = false, LoginCheck = false;
char oldName[NAMESIZE];
char name[NAMESIZE];
char userIDString[IDSIZE];
int userID;

// ó�� �α����� �� �����ϴ� ����
bool loginNameCheck = false;
// �ߺ� �г��� ����
bool dupNameCheck = false;
// 1:1 ����� �����ϱ� ���� ����
int oneToOneComm = 0;

void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	DisplayText("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	multicastIP = (char *)malloc(sizeof(char) * 15);
	multicastPort = (char *)malloc(sizeof(char) * 6);
	//�ð��� ���� ���� UserID�� ����
	srand(time(NULL));
	userID = rand();
	itoa(userID, userIDString, 10);

	// �̺�Ʈ ����
	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (hReadEvent == NULL) return 1;
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hWriteEvent == NULL) return 1;
	hLoginReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (hLoginReadEvent == NULL) return 1;
	hLoginWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hLoginWriteEvent == NULL) return 1;

	// ���� ��� ������ ����
	CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);
	// ��ȭ���� ����
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

	// �̺�Ʈ ����
	CloseHandle(hReadEvent);
	CloseHandle(hWriteEvent);
	CloseHandle(hLoginReadEvent);
	CloseHandle(hLoginWriteEvent);

	// closesocket()
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}

// ��ȭ���� ���ν���
BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
#pragma region Init Dialog
		hEditIP = GetDlgItem(hDlg, EditIP);
		hEditPort = GetDlgItem(hDlg, EditPORT);
		hEditText = GetDlgItem(hDlg, EditText);
		hEditName = GetDlgItem(hDlg, EditName);
		hShowText = GetDlgItem(hDlg, ShowText);
		hResisterNick = GetDlgItem(hDlg, IDNICKNAME);
		hChangeNick = GetDlgItem(hDlg, ID_NICKCHANGE);
		hIPCheckButton = GetDlgItem(hDlg, IDIPCHECK);
		hPortCheckButton = GetDlgItem(hDlg, IDPORTCHECK);
		hSendButton = GetDlgItem(hDlg, IDSEND);
		hLoginButton = GetDlgItem(hDlg, IDOK);
		hExitButton = GetDlgItem(hDlg, IDCANCEL);
#pragma endregion
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDIPCHECK:
			GetDlgItemText(hDlg, EditIP, multicastIP, BUFSIZE + 1);
			if (checkClassDIP(multicastIP) == false)
			{
				MessageBox(hDlg, "���� �ùٸ� ClassD�ּҸ� �Է��� �� �ֽ��ϴ�", "IP����", MB_OK);
				SetFocus(hEditIP);
				return TRUE;
			}
			EnableWindow(hIPCheckButton, FALSE); //�ּ� ���� ��ư ��Ȱ��ȭ
			EnableWindow(hEditIP, FALSE); //�ּ� ���� ��Ʈ�� ��Ȱ��ȭ
			IPcheck = true;
			return TRUE;
		case IDPORTCHECK:
			GetDlgItemText(hDlg, EditPORT, multicastPort, BUFSIZE + 1);
			if (checkPort(multicastPort) == false)
			{
				MessageBox(hDlg, "�ùٸ� Port��ȣ�� �Է��ϼ���", "Port����", MB_OK);
				SetFocus(hEditIP);
				return TRUE;
			}
			//235.7.8.10
			EnableWindow(hPortCheckButton, FALSE); //��Ʈ ���� ��ư ��Ȱ��ȭ
			EnableWindow(hEditPort, FALSE); //��Ʈ ���� ��Ʈ�� ��Ȱ��ȭ
			Portcheck = true;
			return TRUE;

		case IDNICKNAME:
			GetDlgItemText(hDlg, EditName, name, NAMESIZE + 1);
			NameCheck = true;
			EnableWindow(hResisterNick, FALSE); //���� ��Ʈ�� ��Ȱ��ȭ
			return TRUE;

		case IDOK:
			if (IPcheck == false || Portcheck == false) {
				MessageBox(hDlg, "IP�Ǵ� Port�� üũ�ϼ���", "���� �Ұ�", MB_OK);
				SetFocus(hEditIP);
				return TRUE;
			}
			if (NameCheck == false) {
				MessageBox(hDlg, "Name üũ�� ���ּ���", "���� �Ұ�", MB_OK);
				SetFocus(hEditName);
				return TRUE;
			}
			DisplayText("%s\n", userIDString);
			EnableWindow(hLoginButton, FALSE); //���� ��Ʈ�� ��Ȱ��ȭ
			WaitForSingleObject(hLoginReadEvent, INFINITE); // �б� �Ϸ� ��ٸ���

			GetDlgItemText(hDlg, EditName, name, NAMESIZE + 1);

			SetEvent(hLoginWriteEvent);					   // ���� �Ϸ� �˸���
			SetFocus(hEditText);

			// Login�� ���� Buf ���� ����
			LoginCheck = true;
			sprintf(buf, "%s/%s/%s", LOGIN, name, userIDString);
			//sprintf(buf, "%s/%s/%s/%s/", LOGIN, name, userIDString, "���� ä�ù濡 �����߽��ϴ�.");
			SetEvent(hWriteEvent);					   // ���� �Ϸ� �˸���
			WaitForSingleObject(hReadEvent, INFINITE); // �б� �Ϸ� ��ٸ���

			return  TRUE;

		case ID_NICKCHANGE:
			//loginNameCheck = false;
			strcpy(oldName, name);
			GetDlgItemText(hDlg, EditName, name, NAMESIZE + 1);
			DisplayText("%s���� %s���� NickName�� �����Ͽ����ϴ�.\n", oldName, name);
			sprintf(buf, "%s", CHANGENAME);
			SetEvent(hWriteEvent);					   // ���� �Ϸ� �˸���
			WaitForSingleObject(hReadEvent, INFINITE); // �б� �Ϸ� ��ٸ���

			break;

		case IDSEND:
			if (LoginCheck == false) {
				MessageBox(hDlg, "������ ���� ���ּ���", "���� �Ұ�", MB_OK);
				return TRUE;
			}
			WaitForSingleObject(hReadEvent, INFINITE); // �б� �Ϸ� ��ٸ���
			GetDlgItemText(hDlg, EditText, buf, BUFSIZE + 1);
			SetEvent(hWriteEvent);					   // ���� �Ϸ� �˸���

			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

// ���� ��Ʈ�� ��� �Լ�
void DisplayText(char *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);

	char cbuf[BUFSIZE + 256];
	vsprintf(cbuf, fmt, arg);

	int nLength = GetWindowTextLength(hShowText);
	SendMessage(hShowText, EM_SETSEL, nLength, nLength);
	SendMessage(hShowText, EM_REPLACESEL, FALSE, (LPARAM)cbuf);

	va_end(arg);
}

// Ŭ���̾�Ʈ�� ������ ���
DWORD WINAPI Receiver(LPVOID arg)
{
#pragma region InitReceiver
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	receiveSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (receiveSock == INVALID_SOCKET) err_quit("socket()");

	// SO_REUSEADDR �ɼ� ����
	BOOL optval = TRUE;
	retval = setsockopt(receiveSock, SOL_SOCKET,
		SO_REUSEADDR, (char *)&optval, sizeof(optval));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	// bind()
	SOCKADDR_IN localaddr;
	ZeroMemory(&localaddr, sizeof(localaddr));
	localaddr.sin_family = AF_INET;
	localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localaddr.sin_port = htons(atoi(multicastPort));
	retval = bind(receiveSock, (SOCKADDR *)&localaddr, sizeof(localaddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// ��Ƽĳ��Ʈ �׷� ����
	mreq.imr_multiaddr.s_addr = inet_addr(multicastIP);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	retval = setsockopt(receiveSock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		(char *)&mreq, sizeof(mreq));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");
#pragma endregion

	// ������ ��ſ� ����� ����
	SOCKADDR_IN peeraddr;
	int addrlen;
	char receiveBuf[BUFSIZE + 1];
	char tempBuf[BUFSIZE + 1];
	char tempBuf2[BUFSIZE + 1];
	char *splitBuf[4] = { NULL };
	char *splitBuf2[3] = { NULL };
	// ��Ƽĳ��Ʈ ������ �ޱ�
	while (1) {
		// ������ �ޱ�
		addrlen = sizeof(peeraddr);

		retval = recvfrom(receiveSock, receiveBuf, BUFSIZE, 0,
			(SOCKADDR *)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			continue;
		}
		// ���� ������ ���
		receiveBuf[retval] = '\0';
/*
		if (!strcmp(receiveBuf, "]")) {
			
			strncpy(tempBuf2, receiveBuf, sizeof(tempBuf2));
			char *splitChar2 = strtok(tempBuf, "/");
			for (int i = 0; i < 1; i++)
			{
				splitBuf2[i] = splitChar2;
				splitChar2 = strtok(NULL, "/");
			}
			if (!strcmp(splitBuf2[1], userIDString))
			{
				oneToOneComm = 1;
			}
		}*/
		// �г��� ����� oneToOneComm �ʱ�ȭ
		if (!strcmp("!@#$!@#", receiveBuf)) {
			if (oneToOneComm == 0)
				continue;
			oneToOneComm = 0;
			continue;
		}

		int cnt = 0;
		for (int i = 0; i < strlen(receiveBuf); i++) {
			if (receiveBuf[i] == '/')
			{
				break;
			}
			cnt++;
		}

		strncpy(tempBuf, receiveBuf, sizeof(tempBuf));
		char *splitChar = strtok(tempBuf, "/");
		for (int i = 0; i < cnt; i++)
		{
			splitBuf[i] = splitChar;
			splitChar = strtok(NULL, "/");
		}
		strcpy(tempBuf, splitBuf[0]);

		// 1. User�� �α������� ���
		if (!strcmp(LOGIN, tempBuf)) {
			sprintf(receiveBuf, "%s%s", splitBuf[1], "ä�ù濡 �����ϼ̽��ϴ�!");
			if (!strcmp(splitBuf[1], name) && strcmp(splitBuf[2], userIDString)) {
				oneToOneComm = 1;

				DisplayText("%s\n", splitBuf[2]);
				memset(buf, 0, sizeof(char) * BUFSIZE);
				sprintf(buf, "%s/%s","]",splitBuf[2]);
				WaitForSingleObject(hReadEvent, INFINITE); // �б� �Ϸ� ��ٸ���
				SetEvent(hWriteEvent);					   // ���� �Ϸ� �˸���
				//DisplayText("%s���� +1 �߽��ϴ�. \n", splitBuf[1]);
			}
			DisplayText("%s\n", receiveBuf);
			continue;
		}

		// 3. ���� Status ������ Client�� ��� ����
		char status[1];
		itoa(oneToOneComm, status, 10);
		if (!strcmp(splitBuf[3], status)) {
			sprintf(receiveBuf, "[%s:%d] %s", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port), tempBuf);
			DisplayText("%s\n", receiveBuf);
			//DisplayText("%s : %s\n", receiveBuf, status);
		}
	}

	// ��Ƽĳ��Ʈ �׷� Ż��
	retval = setsockopt(receiveSock, IPPROTO_IP, IP_DROP_MEMBERSHIP,
		(char *)&mreq, sizeof(mreq));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	// closesocket()
	closesocket(receiveSock);

	// ���� ����
	WSACleanup();
	return 0;
}


// Ŭ���̾�Ʈ ���� �κ�
DWORD WINAPI ClientMain(LPVOID arg)
{
	int retval;

	while (1) {
		// ���� �Ϸ� ��ٸ���
		WaitForSingleObject(hLoginWriteEvent, INFINITE);

		// ���� �ʱ�ȭ
		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
			return 1;

		// socket()
		sock = socket(AF_INET, SOCK_DGRAM, 0);
		if (sock == INVALID_SOCKET) err_quit("socket()");

		// ��Ƽĳ��Ʈ TTL ����
		int ttl = 2;
		retval = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL,
			(char *)&ttl, sizeof(ttl));
		if (retval == SOCKET_ERROR) err_quit("setsockopt()");

		// ���� �ּ� ����ü �ʱ�ȭ
		ZeroMemory(&remoteaddr, sizeof(remoteaddr));
		remoteaddr.sin_family = AF_INET;
		remoteaddr.sin_addr.s_addr = inet_addr(multicastIP);
		remoteaddr.sin_port = htons(atoi(multicastPort));

		SetEvent(hLoginReadEvent);			 // �б� �Ϸ� �˸���
		break;
	}

	// ������ ��ſ� ����� ����
	int len;
	char sendBuf[BUFSIZE + 1];
	char tempBuf[BUFSIZE + 1];
	HANDLE hThread;

	//���ù� ������ ����
	hThread = CreateThread(NULL, 0, Receiver,
		(LPVOID)sock, 0, NULL);
	if (hThread == NULL) { closesocket(sock); }
	else { CloseHandle(hThread); }

	// ������ ������ ���
	while (1) {
		// ���� �Ϸ� ��ٸ���
		WaitForSingleObject(hWriteEvent, INFINITE);
		// ���ڿ� ���̰� 0�̸� ������ ����
		if (strlen(buf) == 0) {
			SetEvent(hReadEvent); // �б� �Ϸ� �˸���
			continue;
		}

		// Login�� ���� Data ������ ���
		if (loginNameCheck == false) {
			loginNameCheck = true;

			sprintf(sendBuf, "%s", buf);	
		}
		//�г��� �ٲٱ��ư �г��� ����ϱ� ���� ���þȵǴ� �������
		//�г��� �ٲٱ��ư �г��� ����ϱ� ���� ���þȵǴ� �������
		//�г��� �ٲٱ��ư �г��� ����ϱ� ���� ���þȵǴ� �������
		//�г��� �ٲٱ��ư �г��� ����ϱ� ���� ���þȵǴ� �������
		//�г��� ä���� ���ܵ�
		else {
			struct tm t;
			time_t timer;
			//235.7.8.10

			timer = time(NULL);    // ���� �ð��� �� ������ ���
			localtime_s(&t, &timer); // �� ������ �ð��� �и��Ͽ� ����ü�� �ֱ�

			if (buf[0] == ']') {
				sprintf(sendBuf, buf);
			}
			else if (!strcmp(buf, CHANGENAME)) {
				sprintf(sendBuf, buf);
			}
			else {
				sprintf(sendBuf, "[%s] %d��%d��%d�� : %s/%s/%s/%d", name, t.tm_mday, t.tm_hour, t.tm_min, buf, name, userIDString, oneToOneComm);
			}
		}
		// ������ ������
		retval = sendto(sock, sendBuf, strlen(sendBuf), 0,
			(SOCKADDR *)&remoteaddr, sizeof(remoteaddr));
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}
		SetEvent(hReadEvent);			 // �б� �Ϸ� �˸���
	}

	return 0;
}

bool checkClassDIP(char *ip)
{
	// ���� ����ó��
	if (strcmp(ip, " ") == 0)
		return false;

	// IP ���� ����ó��
	int len = strlen(ip);
	if (len > 15 || len < 7)
		return false;

	int nNumCount = 0;
	int nDotCount = 0;
	char checkClassDIP1[3], checkClassDIP2[3], checkClassDIP3[3], checkClassDIP4[3];
	int i = 0, k = 0;

	for (i = 0; i < len; i++)
	{
		// 0~9 �ƴ� ���� �ɸ���
		if (ip[i] < '0' || ip[i] > '9')
		{
			// . �������� IP �и��ϱ�
			if (ip[i] == '.')
			{
				++nDotCount;
				k = 0;
				nNumCount = 0;
			}
			else
				return false;
		}
		// �ùٸ� ���� �Է����� ��� . �������� �ش� ���ڿ��� IP ���� �ֱ�
		else
		{
			if (nDotCount == 0) {
				checkClassDIP1[k++] = ip[i];
			}
			else if (nDotCount == 1) {
				checkClassDIP2[k++] = ip[i];
			}
			else if (nDotCount == 2) {
				checkClassDIP3[k++] = ip[i];
			}
			else if (nDotCount == 3) {
				checkClassDIP4[k++] = ip[i];
			}
			if (++nNumCount > 3)
				return false;
		}
	}
	// .�� 3�� �ƴϸ� false
	if (nDotCount != 3)
		return false;

	int convertInputIP1 = atoi(checkClassDIP1);
	int convertInputIP2 = atoi(checkClassDIP2);
	int convertInputIP3 = atoi(checkClassDIP3);
	int convertInputIP4 = atoi(checkClassDIP4);

	// class D IP �˻��ϱ�
	if (convertInputIP1 >= 224 && convertInputIP1 < 240) {
		if (convertInputIP2 >= 0 && convertInputIP2 <= 255) {
			if (convertInputIP3 >= 0 && convertInputIP3 <= 255) {
				if (convertInputIP4 >= 0 && convertInputIP4 <= 255) {
					return true;
				}
				return false;
			}
			return false;
		}
		return false;
	}
	return false;
}
bool checkPort(char inputPort[]) {
	int convertInputPort = atoi(inputPort);

	if (convertInputPort >= 1024 && convertInputPort <= 65535)
		return true;
	else
		return false;
}