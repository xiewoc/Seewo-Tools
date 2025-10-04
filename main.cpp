//main.cpp 目前的功能有:动态壁纸（需ffpmeg） 
#include <iostream>
#include <windows.h>
#include <commdlg.h>
#include <malloc.h>
#include <shellapi.h>
#include <wlanapi.h>
#include <objbase.h>
#include <memory>
#include <string>
#include "Resource-user.h"
#include <shlwapi.h>
#include <TlHelp32.h>  
#include <stdlib.h>

#ifdef _MSC_VER
	#pragma comment(lib, "shlwapi.lib") // 链接 shlwapi 库
#endif

NOTIFYICONDATAA nid = {};
int cx = GetSystemMetrics(SM_CXSCREEN);//screen width
int cy = GetSystemMetrics(SM_CYSCREEN);//screen height

LPCSTR stringToLPCSTR(const std::string& str) {
    return str.c_str();
}

LPTSTR stringToLPTSTR(const std::string& str) {
    LPTSTR lptstr;
	// 计算所需的缓冲区大小（包括终止符）
    size_t size = str.size() + 1;
    lptstr = new TCHAR[size];
    // 复制字符串内容
    memcpy(lptstr, str.c_str(), size);
    return lptstr;
}

std::string GetExecutableDirectory() {
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    PathRemoveFileSpec(buffer);
    return std::string(buffer);
}

std::string GetFileFullPath(HWND hwnd,int type) {
    OPENFILENAME ofn;       // common dialog box structure
    char szFile[260];       // buffer for file name
    HANDLE hf;              // file handle
    
    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
    // use the contents of szFile to initialize itself.
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    switch(type){
    	case T_VIDEO:{
    		ofn.lpstrFilter = "Video File\0*.mp4;*.mkv;*.mov\0All\0*.*\0";
			break;
		}
		case T_PICTURE:{
			ofn.lpstrFilter = "Picture File\0*.bmp;*.jpg;*.png;*.jpeg;*.jfif;*.webp\0All\0*.*\0";
			break;
		}
		case T_AUDIO:{
			ofn.lpstrFilter = "Audio File\0*.mp3;*.m4a;*.wav;*.flac;*.wma\0All\0*.*\0";
			break;
		}
		default:{
			ofn.lpstrFilter = "All\0*.*\0";
			break;
		}
    		
	}
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_DONTADDTORECENT | OFN_FORCESHOWHIDDEN;
    
    if (GetOpenFileNameA(&ofn) == TRUE) {// 返回文件路径
        return std::string(szFile); 
    } else {// 处理失败情况
        return ""; 
    }
}

BOOL CALLBACK EnumWindowsProc(_In_ HWND hwnd, _In_ LPARAM Lparam)
{
	HWND hDefView = FindWindowEx(hwnd, 0, "SHELLDLL_DefView", 0);
	if (hDefView != nullptr) {
		// 找它的下一个窗口，类名为WorkerW，隐藏它
		HWND hWorkerw = FindWindowEx(0, hwnd, "WorkerW", 0);
		ShowWindow(hWorkerw, SW_HIDE);

		return FALSE;
	}
	return TRUE;
}

//以下为气泡通知及其相关 
void BallonMsg(int MsgType,HWND hwnd,std::string INFOTITLETEXT,std::string INFOTEXT){
	HINSTANCE hins = NULL;
	NOTIFYICONDATAA nidb = {};
	nidb.cbSize = sizeof(nidb);
	nidb.hWnd = hwnd;
	nidb.uFlags = NIF_MESSAGE | NIF_GUID | NIF_INFO;
	nidb.uCallbackMessage=WM_USER;
	
	size_t lenot = std::min(INFOTITLETEXT.length(), sizeof(nidb.szInfoTitle) - 1);
	#ifdef _MSC_VER // Microsoft Visual C++
	    strcpy_s(nidb.szInfoTitle, sizeof(nidb.szInfoTitle), INFOTITLETEXT.c_str());
	#else
	    strncpy(nidb.szInfoTitle, INFOTITLETEXT.c_str(), sizeof(nidb.szInfoTitle) - 1);
	    nidb.szInfoTitle[sizeof(nidb.szInfoTitle) - 1] = '\0';
	#endif
	
	
	size_t lenoi = std::min(INFOTEXT.length(), sizeof(nidb.szInfo) - 1);
	#ifdef _MSC_VER // Microsoft Visual C++
	    strcpy_s(nidb.szInfo, sizeof(nidb.szInfo), INFOTEXT.c_str());
	#else
	    strncpy(nidb.szInfo, INFOTEXT.c_str(), sizeof(nidb.szInfo) - 1);
	    nidb.szInfo[sizeof(nidb.szInfo) - 1] = '\0';
	#endif
	
    nidb.uTimeout=10000;
	switch(MsgType)
	{
		case 1:{
			nidb.dwInfoFlags=NIIF_INFO;
			Shell_NotifyIcon(NIM_MODIFY, &nidb);
			break;
		}
		case 2:{
			nidb.dwInfoFlags=NIIF_WARNING;
			Shell_NotifyIcon(NIM_MODIFY, &nidb);
			break;
		}
		case 3:{
			nidb.dwInfoFlags=NIIF_ERROR;
			Shell_NotifyIcon(NIM_MODIFY, &nidb);
			break;
		}
		case 4:{
			nidb.dwInfoFlags=NIIF_USER;
			Shell_NotifyIcon(NIM_MODIFY, &nidb);
			break;
		}
		default:{
			nidb.dwInfoFlags=NIIF_NONE | NIIF_NOSOUND;
			Shell_NotifyIcon(NIM_MODIFY, &nidb);
			break;
		}
	} 
 } 
 
void TrayWindowIcon(HINSTANCE hInstance,HWND hWnd,std::string TIPTEXT){
	HINSTANCE hins = hInstance; 
	nid.hWnd = hWnd;
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_GUID | NIF_INFO;
	nid.hIcon =LoadIcon(hins, MAKEINTRESOURCE(IDI_NOTIFICATIONICON)); 
	size_t len = std::min(TIPTEXT.length(), sizeof(nid.szTip) - 1);
	#ifdef _MSC_VER // Microsoft Visual C++
	    strcpy_s(nid.szTip, sizeof(nid.szTip), TIPTEXT.c_str());
	#else
	    strncpy(nid.szTip, TIPTEXT.c_str(), sizeof(nid.szTip) - 1);
	    nid.szTip[sizeof(nid.szTip) - 1] = '\0';
	#endif
	nid.uCallbackMessage=WM_USER;
	Shell_NotifyIcon(NIM_ADD, &nid);
}

void DeleteTrayWindowIcon(){
	Shell_NotifyIcon(NIM_DELETE, &nid);
}

int KillProc(char Kill_Name[])	
{
    // 为进程的所有线程拍个快照  
    HANDLE hSnapshort = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);  
    if( hSnapshort==INVALID_HANDLE_VALUE )  
    {
        return -1;  
    }  
  
    // 获得线程列表  
    PROCESSENTRY32 stcProcessInfo;  
    stcProcessInfo.dwSize = sizeof(stcProcessInfo);  
    BOOL  bRet = Process32First(hSnapshort, &stcProcessInfo);  
    while (bRet)  
    {  
		if(strcmp(stcProcessInfo.szExeFile,Kill_Name)==0)
		{	
			Sleep(2000);		//等待时间
            HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE,FALSE,stcProcessInfo.th32ProcessID);	//获取进程句柄
            ::TerminateProcess(hProcess,0);    //结束进程
            CloseHandle(hProcess);		
		}
        bRet = Process32Next(hSnapshort, &stcProcessInfo);  
    }  
  
    CloseHandle(hSnapshort);  
 
	return 0;
}

void OnTrayIcon(HWND hWnd,LPARAM lParam){
	POINT pt;//用于接收鼠标坐标
	MENUINFO mi;
	int menu_rtn;//用于接收菜单选项返回值
	HMENU hmenu = CreatePopupMenu();//生成菜单
	mi.cbSize = sizeof(MENUINFO);
	mi.fMask = MIM_STYLE;
	mi.dwStyle = MNS_NOCHECK | MNS_AUTODISMISS;
	mi.hbrBack = (HBRUSH)CreateSolidBrush(RGB(188,210,238));
	SetMenuInfo(hmenu,&mi);
	AppendMenu(hmenu, MF_GRAYED | MF_DISABLED, 0 , "壁纸相关");
	AppendMenu(hmenu, MF_STRING, IDM_SETLIVEWALLPAPER, "设置动态壁纸");
	AppendMenu(hmenu, MF_STRING, IDM_CLEARLIVEWALLPAPER , "清除动态壁纸");
	AppendMenu(hmenu, MF_SEPARATOR, 0 , NULL);
	AppendMenu(hmenu, MF_GRAYED | MF_DISABLED, 0 , "网络相关");
	AppendMenu(hmenu, MF_STRING, IDM_SETUPHOTSPOT, "设置动态壁纸");
	AppendMenu(hmenu, MF_SEPARATOR, 0 , NULL);
	AppendMenu(hmenu, MF_STRING, IDM_ABOUT, "关于");
	AppendMenu(hmenu, MF_STRING, IDM_EXIT, "退出");
	
	if (lParam == WM_RBUTTONDOWN||lParam == WM_LBUTTONDOWN)
	{
		GetCursorPos(&pt);//取鼠标坐标
		SetForegroundWindow(hWnd);
		menu_rtn = TrackPopupMenu(hmenu, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, hWnd, NULL );//显示菜单并获取选项ID
		if (menu_rtn == IDM_SETLIVEWALLPAPER){
	        std::string lpParameter = " " + GetFileFullPath(hWnd,T_VIDEO) + " -noborder -x " + std::to_string(cx) +" -y " + std::to_string(cy) + " -loop 0";
			STARTUPINFO si{0};
			PROCESS_INFORMATION pi{0};
			/*CreateProcess("ffplay.exe", (LPSTR)stringToLPTSTR(lpParameter), 0, 0, 0, 0, 0, GetExecutableDirectory().c_str(), &si, &pi)*/
			HINSTANCE hInst = ShellExecute(NULL, "open", "ffplay.exe", stringToLPCSTR(lpParameter), GetExecutableDirectory().c_str(), SW_HIDE | SW_MINIMIZE | SW_SHOWNOACTIVATE);
			if ((DWORD)hInst>32){
				Sleep(100);
				while(1){
				RECT wrect;													
				GetClientRect(FindWindow("SDL_app", 0),&wrect);
					if(wrect.right==cx&&wrect.bottom==cy){
						HWND hProgman = FindWindow("Progman", "Program Manager");				// 找到PM窗口
						if(hProgman==nullptr){
							MessageBox(NULL, "Could not find progman window!","Error!",MB_OK);
						}
						SendMessageTimeout(hProgman, 0x52C, 0, 0, SMTO_NORMAL, 0x3e8, 0);	// 给它发特殊消息
						HWND hFfplay = FindWindow("SDL_app", 0);				// 找到视频窗口
						HWND Nparent = SetParent(hFfplay, hProgman);							// 将视频窗口设置为PM的子窗口
						if(Nparent==NULL){
							MessageBox(NULL, "Could not set progman window as parent window!","Error!",MB_OK);
						}
						EnumWindows(EnumWindowsProc, 0);
						SetWindowLong(hFfplay, GWL_EXSTYLE, GetWindowLong(hFfplay, GWL_EXSTYLE) | WS_EX_TOOLWINDOW);
						std::string wn = GetExecutableDirectory() + "\\ffplay.exe";
//						SetWindowLong(FindWindow("ConsoleWindowClass", wn.c_str()), GWL_EXSTYLE, GetWindowLong(FindWindow("ConsoleWindowClass", wn.c_str()), GWL_EXSTYLE) | WS_EX_TOOLWINDOW);
						SetWindowPos(FindWindow("ConsoleWindowClass", wn.c_str()),HWND_BOTTOM,0,0,0,0,SWP_NOSIZE | SWP_NOMOVE | SWP_HIDEWINDOW);
						SetWindowPos(FindWindow("SysListView32", "FolderView"),HWND_TOPMOST,0,0,0,0,SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
						break;
					}
					Sleep(10);
				}
				// 找到第二个WorkerW窗口并隐藏它
        	}
		}
		if (menu_rtn == IDM_CLEARLIVEWALLPAPER){
			KillProc((LPSTR)"ffplay.exe");
			}
		if (menu_rtn == IDM_SETUPHOTSPOT){
			CreateHotSpot();
			}
		if (menu_rtn == IDM_ABOUT){
			ShellExecuteA(NULL, "open", "https://github.com/xiewoc", NULL, NULL, SW_SHOWNORMAL);
			}
		if (menu_rtn == IDM_EXIT){
		    BallonMsg(4,hWnd,"Seewo-Tool已退出"," ");
		    KillProc((LPSTR)"ffplay.exe");
		    Sleep(1000);
		    DeleteTrayWindowIcon();
		    PostQuitMessage(0);
		}
	}
}

/* This is where all the input to the window goes to */
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam){
	switch(Message) {
		
		/* Upon destruction, tell the main thread to stop */
		case WM_DESTROY: {
		    DeleteTrayWindowIcon();
			PostQuitMessage(0);
			break;
		}
		case WM_USER:{
			OnTrayIcon(hwnd, lParam);
			break;
		}
		/* All other messages (a lot of them) are processed using default procedures */
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

/* The 'main' function of Win32 GUI programs: this is where execution starts */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc; /* A properties struct of our window */
	HWND hwnd; /* A 'HANDLE', hence the H, or a pointer to our window */
	MSG msg; /* A temporary location for all messages */

	/* zero out the struct and set the stuff we want to modify */
	memset(&wc,0,sizeof(wc));
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = WndProc; /* This is where we will send messages to */
	wc.hInstance	 = hInstance;
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	
	/* White, COLOR_WINDOW is just a #define for a system color, try Ctrl+Clicking it */
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszClassName = "WindowClass";
	wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION); /* Load a standard icon */
	wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION); /* use the name "A" to use the project icon */

	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,"WindowClass","Caption",WS_MINIMIZE|WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, /* x */
		CW_USEDEFAULT, /* y */
		200, /* width */
		140, /* height */
		NULL,NULL,hInstance,NULL);

	if(hwnd == NULL) {
		MessageBox(NULL, "Window Creation Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}
	TrayWindowIcon(hInstance,hwnd,"Seewo-Tool");
	
	while(GetMessage(&msg, NULL, 0, 0) > 0) { /* If no error is received... */
		TranslateMessage(&msg); /* Translate key codes to chars if present */
		DispatchMessage(&msg); /* Send it to WndProc */
	}
	return msg.wParam;
}
