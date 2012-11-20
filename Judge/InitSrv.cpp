#include "stdafx.h"
#include "InitSrv.h"

int InitSrv::InitSrv(){
	try{
		if(!LoadConfig())
			throw runtime_error("LoadConfig Failed");
		InitializeCriticalSection(&cs);
		NETWORK *Network = new NETWORK();
		DWORD tid;
		MyHandle hThread;
		if(NULL == (hThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)StartNewThread<NETWORK>,(PVOID)Network,0,&tid)))
			throw runtime_error("CreateThread Failed");
	}
	catch(...){
		return FALSE;
	}
	return TRUE;
}

int InitSrv::LoadConfig(){
	HKEY hKey;
	if(ERROR_SUCCESS != RegOpenKeyW(HKEY_LOCAL_MACHINE,L"SYSTEM\\CurrentControlSet\\Services\\v-Judge_Kernel",&hKey))
		return false;
	try{
		DWORD type,sz;
		if(ERROR_SUCCESS != RegQueryValueExW(hKey,L"Username",NULL,&type,NULL,&sz))
			return false;
		WCHAR *Username = (WCHAR*)new BYTE[sz];
		if(ERROR_SUCCESS != RegQueryValueExW(hKey,L"Username",NULL,&type,(LPBYTE)Username,&sz))
			return false;

		sz=sizeof(DWORD);
		if(ERROR_SUCCESS != RegQueryValueExW(hKey,L"Port",NULL,&type,(LPBYTE)&NETWORK::port,&sz))
			return false;

		if(ERROR_SUCCESS != RegQueryValueExW(hKey,L"Password",NULL,&type,NULL,&sz))
			return false;
		WCHAR *Password = (WCHAR*)new BYTE[sz];
		if(ERROR_SUCCESS != RegQueryValueExW(hKey,L"Password",NULL,&type,(LPBYTE)Password,&sz))
			return false;

		if(ERROR_SUCCESS != RegQueryValueExW(hKey,L"Temp",NULL,&type,NULL,&sz))
			return false;
		WCHAR *wTempPath = (WCHAR*)new BYTE[sz];
		if(ERROR_SUCCESS != RegQueryValueExW(hKey,L"Temp",NULL,&type,(LPBYTE)wTempPath,&sz))
			return false;
		CreateDirectoryW(wTempPath,NULL);
		if(0 == (GetFileAttributesW(wTempPath)&FILE_ATTRIBUTE_DIRECTORY))
			return false;
		if(!SetCurrentDirectoryW(wTempPath))
			return false;
		DELETE_TEMP *Delete = new DELETE_TEMP();
		PTP_TIMER pTimer = CreateThreadpoolTimer(TimeoutCallback<DELETE_TEMP>,(PVOID)Delete,NULL);
		if(NULL == pTimer)
			return false;
		FILETIME start;
		start.dwHighDateTime = -1;
		start.dwLowDateTime  = -1;
		SetThreadpoolTimer(pTimer,&start,60000,0);

		RUN::environment.Add(L"Temp",wstring(wTempPath));
		RUN::environment.Add(L"Tmp",wstring(wTempPath));

		delete[] wTempPath;

		sz=sizeof(double);
		if(ERROR_SUCCESS != RegQueryValueExW(hKey,L"tm_Ratio",0,&type,(BYTE*)&RUN::tm_ratio,&sz))
			return false;
		sz=sizeof(double);
		if(ERROR_SUCCESS != RegQueryValueExW(hKey,L"tm_Offset",0,&type,(BYTE*)&RUN::tm_offset,&sz))
			return false;

		sz=sizeof(LONGLONG);
		if(ERROR_SUCCESS != RegQueryValueExW(hKey,L"SecurityCode",0,&type,(BYTE*)&MAIN::SecurityCode,&sz))
			return false;

		if(!RUN::hToken.InitWithAuth(Username,Password))
			return false;
	}
	catch(...){
		return false;
	}
	RegCloseKey(hKey);
	return true;
}