#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <ctime>
#include <string.h>
#include <stdio.h>
#include "resource.h"

#define MULTICASTIP "235.7.8.10"
#define REMOTEPORT 9000
#define BUFSIZE    512
#define NAMESIZE 10
#define IDSIZE 10

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
#pragma endregion
#pragma region Variable Declare
SOCKET sock; // ����
SOCKET receiveSock; // ����
struct ip_mreq mreq;
SOCKADDR_IN remoteaddr;  //���� ����ü
char buf[BUFSIZE + 1];	   // ������ �ۼ��� ����
HANDLE hReadEvent, hWriteEvent; // �����̺�Ʈ
HANDLE hLoginReadEvent, hLoginWriteEvent; // �α����̺�Ʈ
HWND hSendButton;    // ������ ��ư
HWND hChangeNick;    // �г��� �ٲٱ� ��ư
HWND hLoginButton;   // ���� ��ư
HWND hExitButton;    // ������ ��ư
HWND hEditIP, hEditPort, hEditText, hShowText, hEditName; // ���� ��Ʈ��
bool twiceCheck = false;
#pragma endregion

char name[NAMESIZE];
char userIDString[IDSIZE];
int userID;

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
		hChangeNick = GetDlgItem(hDlg, ID_NICKCHANGE);
		hSendButton = GetDlgItem(hDlg, IDOK);
		hLoginButton = GetDlgItem(hDlg, IDSEND);
		hExitButton = GetDlgItem(hDlg, IDCANCEL);
		SendMessage(hEditText, EM_SETLIMITTEXT, BUFSIZE, 0);
#pragma endregion
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {

		case IDOK:
			WaitForSingleObject(hLoginReadEvent, INFINITE); // �б� �Ϸ� ��ٸ���
			GetDlgItemText(hDlg, EditIP, multicastIP, BUFSIZE + 1);
			GetDlgItemText(hDlg, EditPORT, multicastPort, BUFSIZE + 1);
			GetDlgItemText(hDlg, EditName, name, NAMESIZE+1);
			SetEvent(hLoginWriteEvent);					   // ���� �Ϸ� �˸���
			SetFocus(hEditText);
			SendMessage(hEditIP, EM_SETSEL, 0, -1);
			SendMessage(hEditPort, EM_SETSEL, 0, -1);
			SendMessage(hEditName, EM_SETSEL, 0, -1);

			//if (twiceCheck == true) {
			//	int retval;
			//	// ��Ƽĳ��Ʈ �׷� Ż��
			//    retval = setsockopt(receiveSock, IPPROTO_IP, IP_DROP_MEMBERSHIP,
			//		(char *)&mreq, sizeof(mreq));
			//	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

			//	// ���� �ּ� ����ü �ʱ�ȭ
			//	ZeroMemory(&remoteaddr, sizeof(remoteaddr));
			//	remoteaddr.sin_family = AF_INET;
			//	remoteaddr.sin_addr.s_addr = inet_addr("235.7.8.9");
			//	remoteaddr.sin_port = htons(atoi(multicastPort));

			//	// ��Ƽĳ��Ʈ �׷� ����
			//	mreq.imr_multiaddr.s_addr = inet_addr("235.7.8.9");
			//	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
			//	retval = setsockopt(receiveSock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
			//		(char *)&mreq, sizeof(mreq));
			//	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

			//	twiceCheck = false;
			//}
			//twiceCheck = true;
			return  TRUE;
		case ID_NICKCHANGE:
			GetDlgItemText(hDlg, EditName, name, NAMESIZE + 1);
			break;

		case IDSEND:
			WaitForSingleObject(hReadEvent, INFINITE); // �б� �Ϸ� ��ٸ���
			GetDlgItemText(hDlg, EditText, buf, BUFSIZE + 1);
			/*GetDlgItemText(hDlg, EditIP, multicastIP, 16);
			MessageBox(hSendButton, multicastIP, "�޽��� �ڽ�", MB_ICONERROR | MB_OK);*/

			SetEvent(hWriteEvent);					   // ���� �Ϸ� �˸���
			SendMessage(hEditText, EM_SETSEL, 0, -1);
			//MessageBox(hSendButton, "���콺 ���� ��ư�� �������ϴ�", "�޽��� �ڽ�", MB_ICONERROR | MB_OK);
/*
			for(int i=0; i<numberOfpeople; i++)
				MessageBox(hSendButton, user[i].nickName, "�޽��� �ڽ�", MB_ICONERROR | MB_OK);
*/
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

	// ������ ��ſ� ����� ����
	SOCKADDR_IN peeraddr;
	int addrlen;
	char nameBuf[NAMESIZE + 1];
	char idBuf[IDSIZE + 1];
	char buf[BUFSIZE + 1];
	// ��Ƽĳ��Ʈ ������ �ޱ�
	while (1) {
		// ������ �ޱ�
		addrlen = sizeof(peeraddr);

		// �г��� �ޱ�
		retval = recvfrom(receiveSock, nameBuf, NAMESIZE, 0,
			(SOCKADDR *)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			continue;
		}
		nameBuf[retval] = '\0';
		//235.7.8.10

		// ID �ޱ�
		retval = recvfrom(receiveSock, idBuf, IDSIZE, 0,
			(SOCKADDR *)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			continue;
		}
		idBuf[retval] = '\0';

		if(!strcmp(nameBuf,name) && strcmp(idBuf,userIDString))
			MessageBox(hSendButton, nameBuf, "�޽��� �ڽ�", MB_ICONERROR | MB_OK);

		retval = recvfrom(receiveSock, buf, BUFSIZE, 0,
			(SOCKADDR *)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			continue;
		}

		// ���� ������ ���
		buf[retval] = '\0';
		//printf("\n[UDP/%s:%d] %s\n", inet_ntoa(peeraddr.sin_addr), 
		//	ntohs(peeraddr.sin_port), buf);
		DisplayText("%s ", nameBuf);
		DisplayText("[%s:%d] : %s\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port), buf);
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

		//strcpy(user[numberOfpeople++].nickName, name);
		
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

		// NickName ������
		retval = sendto(sock, name, strlen(name), 0,
			(SOCKADDR *)&remoteaddr, sizeof(remoteaddr));
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}

		// ID ������
		retval = sendto(sock, userIDString, strlen(userIDString), 0,
			(SOCKADDR *)&remoteaddr, sizeof(remoteaddr));
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}

		// ������ ������
		retval = sendto(sock, buf, strlen(buf), 0,
			(SOCKADDR *)&remoteaddr, sizeof(remoteaddr));
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}
		//DisplayText("%s\n", buf);


		SetEvent(hReadEvent);			 // �б� �Ϸ� �˸���
	}

	return 0;
}