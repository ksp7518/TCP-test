#include <windows.h>
#include <TCHAR.H>
#define WM_ASYNC WM_USER+2
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, 
						 WPARAM wParam, LPARAM lParam);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpszCmdLine, int nCmdShow)
{   
	HWND 	  hwnd;
	MSG 	  msg;
	WNDCLASS  WndClass;   
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = WndProc;		
	WndClass.cbClsExtra	= 0;		
	WndClass.cbWndExtra	= 0;		
	WndClass.hInstance = hInstance;		
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);	
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);	
	WndClass.lpszMenuName = NULL;		
	WndClass.lpszClassName = _T("Window Class Name");	
	RegisterClass(&WndClass);	
	hwnd = CreateWindow(
		_T("Window Class Name"),
		_T("Client Window"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,	
		CW_USEDEFAULT,	
		CW_USEDEFAULT,	
		CW_USEDEFAULT,	
		NULL,	
		NULL,	
		hInstance,	
		NULL	 
	);
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);	
		DispatchMessage(&msg);	
	}   
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, 
						 WPARAM wParam, LPARAM lParam)     
{
	HDC hdc;
	PAINTSTRUCT ps;
	static WSADATA wsadata;
	static SOCKET s;
	static SOCKADDR_IN addr = { 0 };
	static TCHAR msg[200];
	static TCHAR str[100];
	static char buffer[100];
	static int count;
	int msgLen;

	switch (iMsg) 
	{
	case WM_CREATE:  
		WSAStartup(MAKEWORD(2, 2), &wsadata);
		s = socket(AF_INET, SOCK_STREAM, 0);
		addr.sin_family = AF_INET;
		addr.sin_port = 20;
		addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		WSAAsyncSelect(s, hwnd, WM_ASYNC, FD_READ);
		if(connect(s, (LPSOCKADDR)&addr, sizeof(addr)) == -1) 
			return 0;
		break;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		if (_tcscmp(msg, _T(""))) TextOut(hdc, 0, 30, msg, (int)_tcslen(msg));
		TextOut(hdc, 0,0, str,_tcslen(str));
		EndPaint(hwnd, &ps);
		break;
	case WM_ASYNC:
		switch(lParam) 
		{
		case FD_READ:
			msgLen = recv(s, buffer, 100, 0);
			buffer[msgLen] = NULL;
#ifdef _UNICODE
			msgLen = MultiByteToWideChar(CP_ACP, 0, buffer, strlen(buffer), NULL, NULL);
			MultiByteToWideChar(CP_ACP, 0, buffer, strlen(buffer), msg, msgLen);
			msg[msgLen] = NULL;
#else
			strcpy_s(msg, buffer);
#endif
			InvalidateRgn(hwnd, NULL, TRUE);
			break;
		default:
			break;
		}
		break;
	case WM_CHAR:
		if (wParam == VK_RETURN)
			if (s == INVALID_SOCKET) 
				return 0;
			else
			{
#ifdef _UNICODE
				msgLen = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
				WideCharToMultiByte(CP_ACP, 0, str, -1, buffer, msgLen, NULL, NULL);
#else
				strcpy_s(buffer, str);
				msgLen = strlen(buffer);
#endif
				send(s, (LPSTR )buffer, msgLen+1, 0);
				count = 0;
				return 0;
			}
		str[count++] = wParam;
		str[count] = NULL;
		InvalidateRgn(hwnd, NULL, TRUE);
		return 0;
	case WM_DESTROY:
		closesocket(s);
		WSACleanup();
		PostQuitMessage(0);
		break;
	} 
	return DefWindowProc(hwnd, iMsg, wParam, lParam); 
}