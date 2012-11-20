#include "resource.h"
#include "resource1.h"
#include "MAINLOOP.h"
#include "DELETE_TEMP.h"
#include "RUN.h"
#include "NETWORK.h"
#include "MAIN.h"
#include "Install.h"
#include "InitSrv.h"

#define MYWM_NOTIFYICON WM_USER+1
#define SVCNAME L"v-Judge_Kernel"
#define MAX_LOADSTRING 100

SERVICE_STATUS          gSvcStatus; 
SERVICE_STATUS_HANDLE   gSvcStatusHandle; 
HANDLE                  ghSvcStopEvent;

// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

VOID WINAPI SvcCtrlHandler( DWORD ); 
VOID WINAPI SvcMain( DWORD, LPTSTR * ); 

VOID ReportSvcStatus( DWORD, DWORD, DWORD );
VOID SvcInit( DWORD, LPTSTR * ); 
VOID SvcReportEvent( LPTSTR );
BOOL SvcInstall(void);
BOOL SvcUninstall(void);
BOOL SetDenied(WCHAR* Username);