// Judge.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include "Judge.h"
#pragma region All of these codes are pasted from internet!

#pragma region Service

//
// Purpose: 
//   Installs a service in the SCM database
//
// Parameters:
//   None
// 
// Return value:
//   None
//
BOOL SvcInstall()
{	
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    WCHAR szPath[MAX_PATH];

    if( !GetModuleFileNameW( NULL, szPath, MAX_PATH ) )
    {
        return FALSE;
    }

    // Get a handle to the SCM database. 
 
    schSCManager = OpenSCManager( 
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 
 
    if (NULL == schSCManager) 
    {
        return FALSE;
    }

    // Create the service

    schService = CreateServiceW( 
        schSCManager,              // SCM database 
        SVCNAME,                   // name of service 
        SVCNAME,                   // service name to display 
        SERVICE_ALL_ACCESS,        // desired access 
        SERVICE_WIN32_OWN_PROCESS, // service type 
        SERVICE_DEMAND_START,      // start type 
        SERVICE_ERROR_NORMAL,      // error control type 
        szPath,                    // path to service's binary 
        NULL,                      // no load ordering group 
        NULL,                      // no tag identifier 
        NULL,                      // no dependencies 
        NULL,                      // LocalSystem account 
        NULL);                     // no password 
 
    if (schService == NULL) {
		schService = OpenServiceW(schSCManager,SVCNAME,SERVICE_ALL_ACCESS);
		if(schService == NULL){
			CloseServiceHandle(schSCManager);
			return FALSE;
		}
    }
	if(!Install::Install()){
		CloseServiceHandle(schService); 
		CloseServiceHandle(schSCManager);
		return FALSE;
	}
	if(!StartService(schService,0,NULL))
		return FALSE;

    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);
	return TRUE;
}


BOOL SvcUninstall(void){
	SC_HANDLE schSCManager;
    schSCManager = OpenSCManager( 
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 
 
    if (NULL == schSCManager)
        return FALSE;

	SC_HANDLE hService = OpenServiceW(schSCManager,SVCNAME,SERVICE_STOP|DELETE);
	if(hService == NULL){
		CloseServiceHandle(schSCManager);
		return FALSE;
	}
	SERVICE_STATUS status;
	BOOL ret=FALSE;
	ControlService(hService,SERVICE_CONTROL_STOP,&status);
	if(DeleteService(hService))
		ret=TRUE;
	CloseServiceHandle(hService);
	CloseServiceHandle(schSCManager);
	return ret;
}

//
// Purpose: 
//   Entry point for the service
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None.
//
VOID WINAPI SvcMain( DWORD dwArgc, LPTSTR *lpszArgv )
{
    // Register the handler function for the service
    gSvcStatusHandle = RegisterServiceCtrlHandlerW( 
        SVCNAME, 
        SvcCtrlHandler);

    if( !gSvcStatusHandle )
    { 
        SvcReportEvent(TEXT("RegisterServiceCtrlHandler")); 
        return; 
    } 

    // These SERVICE_STATUS members remain as set here

    gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS; 
    gSvcStatus.dwServiceSpecificExitCode = 0;    

    // Report initial status to the SCM

    ReportSvcStatus( SERVICE_START_PENDING, NO_ERROR, 3000 );

    // Perform service-specific initialization and work.

    SvcInit( dwArgc, lpszArgv );
}

//
// Purpose: 
//   The service code
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None
//
VOID SvcInit( DWORD dwArgc, LPTSTR *lpszArgv)
{
    // TO_DO: Declare and set any required variables.
    //   Be sure to periodically call ReportSvcStatus() with 
    //   SERVICE_START_PENDING. If initialization fails, call
    //   ReportSvcStatus with SERVICE_STOPPED.

    // Create an event. The control handler function, SvcCtrlHandler,
    // signals this event when it receives the stop control code.

    ghSvcStopEvent = CreateEvent(
                         NULL,    // default security attributes
                         TRUE,    // manual reset event
                         FALSE,   // not signaled
                         NULL);   // no name

    if ( ghSvcStopEvent == NULL)
    {
        ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );
        return;
    }

    // Report running status when initialization is complete.

	if(InitSrv::InitSrv())
		ReportSvcStatus( SERVICE_RUNNING, NO_ERROR, 0 );
	else{
		ReportSvcStatus(SERVICE_STOPPED,ERROR_APP_INIT_FAILURE,0);
		return;
	}

    // TO_DO: Perform work until service stops.

    while(1)
    {
        // Check whether to stop the service.

        WaitForSingleObject(ghSvcStopEvent, INFINITE);

        ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );
        return;
    }
}

//
// Purpose: 
//   Sets the current service status and reports it to the SCM.
//
// Parameters:
//   dwCurrentState - The current state (see SERVICE_STATUS)
//   dwWin32ExitCode - The system error code
//   dwWaitHint - Estimated time for pending operation, 
//     in milliseconds
// 
// Return value:
//   None
//
VOID ReportSvcStatus( DWORD dwCurrentState,
                      DWORD dwWin32ExitCode,
                      DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.

    gSvcStatus.dwCurrentState = dwCurrentState;
    gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    gSvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        gSvcStatus.dwControlsAccepted = 0;
    else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ( (dwCurrentState == SERVICE_RUNNING) ||
           (dwCurrentState == SERVICE_STOPPED) )
        gSvcStatus.dwCheckPoint = 0;
    else gSvcStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the SCM.
    SetServiceStatus( gSvcStatusHandle, &gSvcStatus );
}

//
// Purpose: 
//   Called by SCM whenever a control code is sent to the service
//   using the ControlService function.
//
// Parameters:
//   dwCtrl - control code
// 
// Return value:
//   None
//
VOID WINAPI SvcCtrlHandler( DWORD dwCtrl )
{
   // Handle the requested control code. 

   switch(dwCtrl) 
   {  
      case SERVICE_CONTROL_STOP: 
         ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

         // Signal the service to stop.

         SetEvent(ghSvcStopEvent);
         ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);
         
         return;
 
      case SERVICE_CONTROL_INTERROGATE: 
         break; 
 
      default: 
         break;
   } 
   
}

//
// Purpose: 
//   Logs messages to the event log
//
// Parameters:
//   szFunction - name of function that failed
// 
// Return value:
//   None
//
// Remarks:
//   The service must have an entry in the Application event log.
//
VOID SvcReportEvent(LPTSTR szFunction) 
{ 
    HANDLE hEventSource;
    LPCWSTR lpszStrings[2];
    WCHAR Buffer[80];

    hEventSource = RegisterEventSourceW(NULL, SVCNAME);

    if( NULL != hEventSource )
    {
        StringCchPrintfW(Buffer, 80, L"%s failed with %d", szFunction, GetLastError());

        lpszStrings[0] = SVCNAME;
        lpszStrings[1] = Buffer;

        ReportEventW(hEventSource,        // event log handle
                    EVENTLOG_ERROR_TYPE, // event type
                    0,                   // event category
                    EVENTLOG_ERROR_TYPE, // event identifier
                    NULL,                // no security identifier
                    2,                   // size of lpszStrings array
                    0,                   // no binary data
                    lpszStrings,         // array of strings
                    NULL);               // no binary data

        DeregisterEventSource(hEventSource);
    }
}

#pragma endregion Service


#pragma region Window_Form
int APIENTRY _tWinMain(
	_In_		HINSTANCE hInstance,
	_In_opt_	HINSTANCE hPrevInstance,
	_In_		LPTSTR    lpCmdLine,
	_In_		int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: 在此放置代码。

    // TO_DO: Add any additional services for the process to this table.
    SERVICE_TABLE_ENTRYW DispatchTable[] = 
    { 
        { SVCNAME, (LPSERVICE_MAIN_FUNCTIONW) SvcMain }, 
        { NULL, NULL } 
    }; 
 
    // This call returns when the service has stopped. 
    // The process should simply terminate when the call returns.

    if (StartServiceCtrlDispatcherW( DispatchTable )) 
    {
        return 0;
    }

	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_C_WINDOW, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_C_WINDOW));

	// 主消息循环:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
//  注释:
//
//    仅当希望
//    此代码与添加到 Windows 95 中的“RegisterClassEx”
//    函数之前的 Win32 系统兼容时，才需要此函数及其用法。调用此函数十分重要，
//    这样应用程序就可以获得关联的
//    “格式正确的”小图标。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON2));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_C_WINDOW);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON2));

	return RegisterClassEx(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // 将实例句柄存储在全局变量中

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, SW_HIDE);
   UpdateWindow(hWnd);

   	//对应的类声明文件中定义消息
	NOTIFYICONDATAW m_tnid;
	memset(&m_tnid,0,sizeof(m_tnid));

	//OnCreate 函数中return 之前添加托盘生成代码
	m_tnid.cbSize=sizeof(NOTIFYICONDATA); 
	m_tnid.hWnd=hWnd; 
	m_tnid.uFlags=NIF_MESSAGE|NIF_ICON|NIF_TIP; 
	m_tnid.uCallbackMessage=MYWM_NOTIFYICON;

	//用户定义的回调消息 
	wcscpy_s<128>(m_tnid.szTip,L"v-Judge-Kernel");
	m_tnid.uID=NULL;
	HICON hIcon; 
	hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_ICON2)); 
	m_tnid.hIcon=hIcon; 
	Shell_NotifyIconW(NIM_ADD,&m_tnid); 
	if(hIcon)
		DestroyIcon(hIcon);
   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: 处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case MYWM_NOTIFYICON: 
		if(lParam==WM_RBUTTONDOWN){    
			HMENU hMenu = LoadMenuW(hInst,MAKEINTRESOURCEW(IDR_MENU1));
			HMENU hSubMenu = GetSubMenu(hMenu,0);
			POINT pos; 
			GetCursorPos(&pos);
			SetForegroundWindow(hWnd);
			TrackPopupMenu(hSubMenu,TPM_LEFTALIGN,pos.x,pos.y,0,hWnd,NULL);
			DestroyMenu(hMenu);
		}
	break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 分析菜单选择:
		switch (wmId)
		{
		case ID_EXIT:
			NOTIFYICONDATAW m_tnid;
			memset(&m_tnid,0,sizeof(m_tnid));

			//OnCreate 函数中return 之前添加托盘生成代码
			m_tnid.cbSize=sizeof(NOTIFYICONDATA); 
			m_tnid.hWnd=hWnd; 
			m_tnid.uFlags=NIF_MESSAGE|NIF_ICON|NIF_TIP; 
			m_tnid.uCallbackMessage=MYWM_NOTIFYICON;

			//用户定义的回调消息 
			wcscpy_s<128>(m_tnid.szTip,L"v-Judge-Kernel");
			m_tnid.uID=NULL;
			HICON hIcon; 
			hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_ICON2)); 
			m_tnid.hIcon=hIcon; 
			Shell_NotifyIconW(NIM_DELETE,&m_tnid); 
			if(hIcon)
				DestroyIcon(hIcon);
			DestroyWindow(hWnd);
			break;
		case ID_UNINSTALL:
			if(IDYES == MessageBoxW(0,L"Are you sure to uninstall the service v-Judge_Kernel?",L"Warning!",MB_YESNO)){
				if(SvcUninstall())
					MessageBoxW(0,L"Successfully uninstall.",L"Message",MB_OK);
				else
					MessageBoxW(0,L"Unsuccessfully uninstall.",L"Message",MB_OK);
			}
			break;
		case ID_INSTALL:
			if(IDYES == MessageBoxW(0,L"Are you sure to install the service v-Judge_Kernel?",L"Warning!",MB_YESNO)){
				if(SvcInstall())
					MessageBoxW(0,L"Successfully install.",L"Message",MB_OK);
				else
					MessageBoxW(0,L"Unsuccessfully install.",L"Message",MB_OK);
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此添加任意绘图代码...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

#pragma endregion Window_Form

#pragma endregion All of these codes are pasted from internet!