// 2017년 1학기 네트워크프로그래밍 숙제 2번
// 성명: 박민현 학번: 122179

#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <ctime>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "resource.h"
//235.7.8.10
#define BUFSIZE  512
#define NAMESIZE 10
#define IDSIZE 10
#define LOGIN "!@#*@#!"
#define CHANGENAME "!@#$!@#%"

char *multicastIP;
char *multicastPort;

#pragma region Function Declare
// 대화상자 프로시저
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
// 편집 컨트롤 출력 함수
void DisplayText(char *fmt, ...);
// 오류 출력 함수
void err_quit(char *msg);
void err_display(char *msg);
// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char *buf, int len, int flags);
// 소켓 통신 스레드 함수
DWORD WINAPI ClientMain(LPVOID arg);
bool checkClassDIP(char *ip);
bool checkPort(char inputPort[]);
#pragma endregion
#pragma region Variable Declare
SOCKET sock; // 소켓
SOCKET receiveSock; // 소켓
struct ip_mreq mreq;
SOCKADDR_IN remoteaddr;  //소켓 구조체
char buf[BUFSIZE + 1];	   // 데이터 송수신 버퍼
HANDLE hReadEvent, hWriteEvent; // 전송이벤트
HANDLE hLoginReadEvent, hLoginWriteEvent; // 로그인이벤트
<<<<<<< HEAD
HWND hIPCheckButton;   // IP체크 버튼
HWND hPortCheckButton; //Port체크 버튼
HWND hChangeNick;      // 닉네임 바꾸기 버튼
HWND hLoginButton;     // 접속 버튼
HWND hEditIP, hEditPort, hEditText, hShowText, hEditName; // 편집 컨트롤
bool twiceCheck = false;
#pragma endregion

bool IPcheck = false, Portcheck = false; 
char oldName[NAMESIZE];
char name[NAMESIZE];
char userIDString[IDSIZE];
int userID;

=======
HWND hSendButton, hIPCheckButton, hPortCheckButton, hChangeNick, hLoginButton, hExitButton, hResisterNick,
hEditIP, hEditPort, hEditText, hShowText, hEditName; // 편집 컨트롤

bool ipCheck = false, portCheck = false, nameCheck = false, loginCheck = false, registerNameCheck = false;
char oldName[NAMESIZE];		// User 구닉네임
char name[NAMESIZE];		// User 닉네임
char userIDString[IDSIZE];	// UserID 스트링
int userID;		// UserID
bool loginNameCheck = false; // 처음 로그인할 때 구분하는 변수
int oneToOneComm = 0;        // 1:1 통신을 구분하기 위한 변수 
struct tm t;	// 타이머 구조체
time_t timer;	// 타이머 
#pragma endregion

>>>>>>> develop
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
	//시간에 따른 랜덤 UserID값 생성
	srand(time(NULL));
	userID = rand();
	itoa(userID, userIDString, 10);

	// 이벤트 생성
	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (hReadEvent == NULL) return 1;
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hWriteEvent == NULL) return 1;
	hLoginReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (hLoginReadEvent == NULL) return 1;
	hLoginWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hLoginWriteEvent == NULL) return 1;

<<<<<<< HEAD

=======
>>>>>>> develop
	// 소켓 통신 스레드 생성
	CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);
	// 대화상자 생성
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

	// 이벤트 제거
	CloseHandle(hReadEvent);
	CloseHandle(hWriteEvent);
	CloseHandle(hLoginReadEvent);
	CloseHandle(hLoginWriteEvent);

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}

// 대화상자 프로시저
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
<<<<<<< HEAD
		hIPCheckButton = GetDlgItem(hDlg, IDIPCHECK);
		hPortCheckButton = GetDlgItem(hDlg, IDPORTCHECK);
		hLoginButton = GetDlgItem(hDlg, IDLOGIN);
		hChangeNick = GetDlgItem(hDlg, ID_NICKCHANGE);
		SendMessage(hEditText, EM_SETLIMITTEXT, BUFSIZE, 0);
=======
		hResisterNick = GetDlgItem(hDlg, IDNICKNAME);
		hChangeNick = GetDlgItem(hDlg, ID_NICKCHANGE);
		hIPCheckButton = GetDlgItem(hDlg, IDIPCHECK);
		hPortCheckButton = GetDlgItem(hDlg, IDPORTCHECK);
		hSendButton = GetDlgItem(hDlg, IDSEND);
		hLoginButton = GetDlgItem(hDlg, IDOK);
		hExitButton = GetDlgItem(hDlg, IDCANCEL);
>>>>>>> develop
#pragma endregion
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDIPCHECK:
			GetDlgItemText(hDlg, EditIP, multicastIP, BUFSIZE + 1);
			if (checkClassDIP(multicastIP) == false)
			{
				MessageBox(hDlg, "오직 올바른 ClassD주소만 입력할 수 있습니다", "IP오류", MB_OK);
				SetFocus(hEditIP);
				return TRUE;
			}
<<<<<<< HEAD
			IPcheck = true;
			return TRUE;
=======
			EnableWindow(hIPCheckButton, FALSE); //주소 편집 버튼 비활성화
			EnableWindow(hEditIP, FALSE); //주소 편집 컨트롤 비활성화
			ipCheck = true;
			return TRUE;

>>>>>>> develop
		case IDPORTCHECK:
			GetDlgItemText(hDlg, EditPORT, multicastPort, BUFSIZE + 1);
			if (checkPort(multicastPort) == false)
			{
				MessageBox(hDlg, "올바른 Port번호만 입력하세요", "Port오류", MB_OK);
				SetFocus(hEditIP);
				return TRUE;
<<<<<<< HEAD
			}
			Portcheck = true;
			return TRUE;
		case IDLOGIN:
			if (IPcheck == true && Portcheck == true) {
				SetEvent(hLoginWriteEvent);					   // 쓰기 완료 알리기
				SetFocus(hEditText);
			}
			else {
				MessageBox(hDlg, "IP또는 Port를 체크하세요", "접속 불가", MB_OK);
				SetFocus(hEditIP);
			}
			//if (twiceCheck == true) {
			//	int retval;
			//	// 멀티캐스트 그룹 탈퇴
			//    retval = setsockopt(receiveSock, IPPROTO_IP, IP_DROP_MEMBERSHIP,
			//		(char *)&mreq, sizeof(mreq));
			//	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

			//	// 소켓 주소 구조체 초기화
			//	ZeroMemory(&remoteaddr, sizeof(remoteaddr));
			//	remoteaddr.sin_family = AF_INET;
			//	remoteaddr.sin_addr.s_addr = inet_addr("235.7.8.9");
			//	remoteaddr.sin_port = htons(atoi(multicastPort));

			//	// 멀티캐스트 그룹 가입
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
=======
			}
			//235.7.8.10
			EnableWindow(hPortCheckButton, FALSE); //포트 편집 버튼 비활성화
			EnableWindow(hEditPort, FALSE); //포트 편집 컨트롤 비활성화
			portCheck = true;
			return TRUE;


		case IDOK:
			if (ipCheck == false || portCheck == false) {
				MessageBox(hDlg, "IP또는 Port를 체크하세요", "접속 불가", MB_OK);
				SetFocus(hEditIP);
				return TRUE;
			}
			if (nameCheck == false) {
				MessageBox(hDlg, "Name 체크를 해주세요", "접속 불가", MB_OK);
				SetFocus(hEditName);
				return TRUE;
			}
			EnableWindow(hLoginButton, FALSE);				//접속 컨트롤 비활성화
			WaitForSingleObject(hLoginReadEvent, INFINITE); // 읽기 완료 기다리기

			GetDlgItemText(hDlg, EditName, name, NAMESIZE + 1);

			SetEvent(hLoginWriteEvent);					   // 쓰기 완료 알리기
			SetFocus(hEditText);

			// Login에 대한 Buf 내용 전송
			loginCheck = true;
			sprintf(buf, "%s/%s/%s", LOGIN, name, userIDString);
			SetEvent(hWriteEvent);					   // 쓰기 완료 알리기
			WaitForSingleObject(hReadEvent, INFINITE); // 읽기 완료 기다리기

			return  TRUE;

		case IDNICKNAME:
			GetDlgItemText(hDlg, EditName, name, NAMESIZE + 1);
			nameCheck = true;
			EnableWindow(hResisterNick, FALSE); //접속 컨트롤 비활성화
			registerNameCheck = true;
			return TRUE;

		case ID_NICKCHANGE:
			// Name을 등록하기전에 누르면 허용X
			if (registerNameCheck == false)
				break;

			strcpy(oldName, name);
			GetDlgItemText(hDlg, EditName, name, NAMESIZE + 1);
			// 이전과 같은 이름이면 허용X
			if (!strcmp(oldName, name)) {
				break;
			}

			timer = time(NULL);      // 현재 시각을 초 단위로 얻기
			localtime_s(&t, &timer); // 초 단위의 시간을 분리하여 구조체에 넣기

			DisplayText("(%d일 %d시 %d분) %s에서 %s으로 NickName을 변경하였습니다.\n",
				t.tm_mday, t.tm_hour, t.tm_min, oldName, name);

			if (oneToOneComm == 1) {
				sprintf(buf, "%s", CHANGENAME);
				SetEvent(hWriteEvent);					   // 쓰기 완료 알리기
				WaitForSingleObject(hReadEvent, INFINITE); // 읽기 완료 기다리기

				Sleep(100);
				sprintf(buf, "%s/%s/%s/", LOGIN, name, userIDString);
			}
			else {
				sprintf(buf, "%s/%s/%s/", LOGIN, name, userIDString);
			}
			SetEvent(hWriteEvent);					   // 쓰기 완료 알리기
			WaitForSingleObject(hReadEvent, INFINITE); // 읽기 완료 기다리기
>>>>>>> develop
			break;

		case IDSEND:
			if (loginCheck == false) {
				MessageBox(hDlg, "접속을 먼저 해주세요", "접속 불가", MB_OK);
				return TRUE;
			}
			WaitForSingleObject(hReadEvent, INFINITE); // 읽기 완료 기다리기
			GetDlgItemText(hDlg, EditName, name, NAMESIZE + 1);
			GetDlgItemText(hDlg, EditText, buf, BUFSIZE + 1);
			SetEvent(hWriteEvent);					   // 쓰기 완료 알리기
<<<<<<< HEAD
			SendMessage(hEditText, EM_SETSEL, 0, -1);
			//MessageBox(hSendButton, "마우스 왼쪽 버튼을 눌렀습니다", "메시지 박스", MB_ICONERROR | MB_OK);
=======
>>>>>>> develop

			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;

		}
		return FALSE;
	}
	return FALSE;
}

// 편집 컨트롤 출력 함수
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

// 클라이언트 시작 부분
DWORD WINAPI OneToOneMain(LPVOID arg)
{
	int retval;
	char flagBuf[5];

	while (1) {
		// 쓰기 완료 기다리기
		WaitForSingleObject(hLoginWriteEvent, INFINITE);

		int remoteAddrSize = sizeof(remoteaddr);
		// FlAG 받기
		retval = recvfrom(sock, flagBuf, 5, 0,
			(SOCKADDR *)&remoteaddr, &remoteAddrSize);
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			continue;
		}
		//235.7.8.10
		flagBuf[retval] = '\0';
		DisplayText("%s\n", flagBuf);
	}

	return 0;
}

// 클라이언트와 데이터 통신
DWORD WINAPI Receiver(LPVOID arg)
{
#pragma region InitReceiver
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	receiveSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (receiveSock == INVALID_SOCKET) err_quit("socket()");

	// SO_REUSEADDR 옵션 설정
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

	// 멀티캐스트 그룹 가입
	mreq.imr_multiaddr.s_addr = inet_addr(multicastIP);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	retval = setsockopt(receiveSock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		(char *)&mreq, sizeof(mreq));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");
#pragma endregion

	// 데이터 통신에 사용할 변수
	SOCKADDR_IN peeraddr;
	int addrlen;
	char receiveBuf[BUFSIZE + 1];
	char tempBuf[BUFSIZE + 1];
	char *splitBuf[5] = { NULL };
	// 멀티캐스트 데이터 받기
	while (1) {
		// 데이터 받기
		addrlen = sizeof(peeraddr);

		retval = recvfrom(receiveSock, receiveBuf, BUFSIZE, 0,
			(SOCKADDR *)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			continue;
		}
		// 받은 데이터 출력
		receiveBuf[retval] = '\0';


		char *checkBuf = &receiveBuf[strlen(receiveBuf) - 1];
		if (!strcmp(checkBuf, "@"))
		{
			receiveBuf[retval - 1] = '\0';
			if (!strcmp(receiveBuf, userIDString))
			{
				oneToOneComm = 1;
				DisplayText("같은 이름의 User가 이미 채팅에 존재합니다. 대화를 나눠 닉네임을 바꿔주세요.\n");
			}
			continue;
		}

<<<<<<< HEAD
		retval = recvfrom(receiveSock, buf, BUFSIZE, 0,
			(SOCKADDR *)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			continue;
		}

		// 닉네임 체크
		if (!strcmp(nameBuf, name) && strcmp(idBuf, userIDString)) {

			// Flag 보내기
			retval = sendto(receiveSock, "O", strlen("O"), 0,
				(SOCKADDR *)&peeraddr, sizeof(peeraddr));
			if (retval == SOCKET_ERROR) {
				err_display("sendto()");
				continue;
			}

			//MessageBox(hSendButton, nameBuf, "메시지 박스", MB_ICONERROR | MB_OK);
		}


		// Flag 보내기
		retval = sendto(receiveSock, "X", strlen("X"), 0,
			(SOCKADDR *)&peeraddr, sizeof(peeraddr));
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}

		SetEvent(hReadEvent);			 // 읽기 완료 알리기
		struct tm t;
		time_t timer;
=======
		// 닉네임 변경시 oneToOneComm 초기화
		if (!strcmp("!@#$!@#", receiveBuf)) {
			if (oneToOneComm != 0) {
				DisplayText("일대일 대화를 종료합니다.\n");
			}
			oneToOneComm = 0;
			continue;
		}

		int cnt = 1;
		for (int i = 0; i < strlen(receiveBuf); i++) {
			if (receiveBuf[i] == '/') {
				cnt++;
				continue;
			}
		}
>>>>>>> develop

		strncpy(tempBuf, receiveBuf, sizeof(tempBuf));
		char *splitChar = strtok(tempBuf, "/");
		for (int i = 0; i < cnt; i++)
		{
			splitBuf[i] = splitChar;
			splitChar = strtok(NULL, "/");
		}
		strcpy(tempBuf, splitBuf[0]);

		// 1. User가 로그인했을 경우 or 중복된 NickName으로 바꿨을 경우
		if (!strcmp(LOGIN, tempBuf)) {
			if (cnt == 3) {
				sprintf(receiveBuf, "닉네임 %s님이 %s", splitBuf[1], "채팅방에 입장하셨습니다!");
				DisplayText("%s\n", receiveBuf);
			}
			if (!strcmp(splitBuf[1], name) && strcmp(splitBuf[2], userIDString)) {
				oneToOneComm = 1;

				DisplayText("같은 이름의 User가 채팅에 존재합니다. 대화를 나눠 닉네임을 바꿔주세요.\n");
				memset(buf, 0, sizeof(char) * BUFSIZE);
				sprintf(buf, "%s@", splitBuf[2]);
				WaitForSingleObject(hReadEvent, INFINITE); // 읽기 완료 기다리기
				SetEvent(hWriteEvent);					   // 쓰기 완료 알리기
			}
			continue;
		}

		// 2. 같은 Status 상태의 Client간 통신 구현
		char status[1];
		itoa(oneToOneComm, status, 10);
		if (splitBuf[2] == NULL) {
			continue;
		}
		if (!strcmp(splitBuf[3], status)) {
			sprintf(receiveBuf, "[%s:%d] %s", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port), tempBuf);
			DisplayText("%s\n", receiveBuf);
		}
		else if (!strcmp(splitBuf[3], "0")) {
			sprintf(receiveBuf, "[%s:%d] %s", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port), tempBuf);
			DisplayText("%s\n", receiveBuf);
		}
	}

	// 멀티캐스트 그룹 탈퇴
	retval = setsockopt(receiveSock, IPPROTO_IP, IP_DROP_MEMBERSHIP,
		(char *)&mreq, sizeof(mreq));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	// closesocket()
	closesocket(receiveSock);

	// 윈속 종료
	WSACleanup();
	return 0;
}

// 클라이언트 시작 부분
DWORD WINAPI ClientMain(LPVOID arg)
{
	int retval;

	while (1) {
		// 쓰기 완료 기다리기
		WaitForSingleObject(hLoginWriteEvent, INFINITE);

		// 윈속 초기화
		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
			return 1;

		// socket()
		sock = socket(AF_INET, SOCK_DGRAM, 0);
		if (sock == INVALID_SOCKET) err_quit("socket()");

		// 멀티캐스트 TTL 설정
		int ttl = 2;
		retval = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL,
			(char *)&ttl, sizeof(ttl));
		if (retval == SOCKET_ERROR) err_quit("setsockopt()");

		// 소켓 주소 구조체 초기화
		ZeroMemory(&remoteaddr, sizeof(remoteaddr));
		remoteaddr.sin_family = AF_INET;
		remoteaddr.sin_addr.s_addr = inet_addr(multicastIP);
		remoteaddr.sin_port = htons(atoi(multicastPort));

		SetEvent(hLoginReadEvent);			 // 읽기 완료 알리기
		break;
	}

	// 데이터 통신에 사용할 변수
	int len;
	char sendBuf[BUFSIZE + 1];
	HANDLE hThread;

	//리시버 스레드 생성
	hThread = CreateThread(NULL, 0, Receiver,
		(LPVOID)sock, 0, NULL);
	if (hThread == NULL) { closesocket(sock); }
	else { CloseHandle(hThread); }

	// 서버와 데이터 통신
	while (1) {
		// 쓰기 완료 기다리기
		WaitForSingleObject(hWriteEvent, INFINITE);
		// 문자열 길이가 0이면 보내지 않음
		if (strlen(buf) == 0) {
			SetEvent(hReadEvent); // 읽기 완료 알리기
			continue;
		}

		// Login에 대한 Data 전송일 경우
		if (loginNameCheck == false) {
			loginNameCheck = true;

			sprintf(sendBuf, "%s", buf);
		}
		else {
			timer = time(NULL);    // 현재 시각을 초 단위로 얻기
			localtime_s(&t, &timer); // 초 단위의 시간을 분리하여 구조체에 넣기

			char *checkBuf = &buf[strlen(buf) - 1];
			// oneToOneComm이 0인 상황에서 닉네임을 바꿨을 경우
			if (!strcmp(checkBuf, "/")) {
				sprintf(sendBuf, buf);
			}
			// 중복된 닉네임의 Client를 검출하기 위한 경우
			else if (!strcmp(checkBuf, "@")) {
				sprintf(sendBuf, buf);
			}
			// 중복된 상태의 NickName을 바꿨을 경우
			else if (!strcmp(buf, CHANGENAME)) {
				sprintf(sendBuf, buf);
			}
			// 나머지 모든 상황
			else {
				sprintf(sendBuf, "[%s] (%d일%d시%d분) : %s/%s/%s/%d", name, t.tm_mday, t.tm_hour, t.tm_min, buf, name, userIDString, oneToOneComm);
			}
		}
		// 데이터 보내기
		retval = sendto(sock, sendBuf, strlen(sendBuf), 0,
			(SOCKADDR *)&remoteaddr, sizeof(remoteaddr));
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}
		SetEvent(hReadEvent);			 // 읽기 완료 알리기

		//DisplayText("%s\n", buf);
	}
	return 0;
}
<<<<<<< HEAD
//
//bool checkClassDIP(char inputIP[])
//{
//	char checkClassDIP[3];
//	for (int i = 0; i < 3; i++)
//	{
//		checkClassDIP[i] = inputIP[i];
//	}
//	int convertInputIP = atoi(checkClassDIP);
//
//	if (convertInputIP >= 224 && convertInputIP < 240)
//		return true;
//	else
//		return false;
//}
=======

>>>>>>> develop
bool checkClassDIP(char *ip)
{
	// 공백 예외처리
	if (strcmp(ip, " ") == 0)
		return false;
<<<<<<< HEAD
	
=======

>>>>>>> develop
	// IP 길이 예외처리
	int len = strlen(ip);
	if (len > 15 || len < 7)
		return false;

	int nNumCount = 0;
	int nDotCount = 0;
	char checkClassDIP1[3], checkClassDIP2[3], checkClassDIP3[3], checkClassDIP4[3];
	int i = 0, k = 0;

	for (i = 0; i < len; i++)
	{
		// 0~9 아닌 숫자 걸르기
		if (ip[i] < '0' || ip[i] > '9')
		{
			// . 기준으로 IP 분리하기
			if (ip[i] == '.')
			{
				++nDotCount;
				k = 0;
				nNumCount = 0;
			}
			else
				return false;
		}
		// 올바른 숫자 입력했을 경우 . 기준으로 해당 문자열에 IP 집어 넣기
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
	// .이 3개 아니면 false
	if (nDotCount != 3)
<<<<<<< HEAD
		return false;

	int convertInputIP1 = atoi(checkClassDIP1);
	int convertInputIP2 = atoi(checkClassDIP2);
	int convertInputIP3 = atoi(checkClassDIP3);
	int convertInputIP4 = atoi(checkClassDIP4);
	
	// class D IP 검사하기
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
=======
		return false;
>>>>>>> develop

	int convertInputIP1 = atoi(checkClassDIP1);
	int convertInputIP2 = atoi(checkClassDIP2);
	int convertInputIP3 = atoi(checkClassDIP3);
	int convertInputIP4 = atoi(checkClassDIP4);

	// class D IP 검사하기
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