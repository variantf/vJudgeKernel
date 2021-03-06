// stdafx.h : ±Í◊ºœµÕ≥∞¸∫¨Œƒº˛µƒ∞¸∫¨Œƒº˛£¨
// ªÚ «æ≠≥£ π”√µ´≤ª≥£∏¸∏ƒµƒ
// Ãÿ∂®”⁄œÓƒøµƒ∞¸∫¨Œƒº˛
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <vector>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <atomic>
#include <WinSock2.h>
#include <Windows.h>
#include <Psapi.h>
#include <UserEnv.h>
#include <cassert>
#include <io.h>
#include <fcntl.h>
#include <ShellAPI.h>
#include <strsafe.h>
#include <fstream>
#include <string>
#include <sstream>
#include <ShellAPI.h>
#include <Shlwapi.h>
#include <AclAPI.h>
#include <ctime>

using namespace std;
#pragma pack(1)


//自制的一个释放Handle资源的类。可以直接当做HANDLE使用
class MyHandle{
private:
	HANDLE _h;
public:
	MyHandle(HANDLE h);
	MyHandle();
	~MyHandle();
	HANDLE* operator&();
	operator HANDLE();
	HANDLE operator = (HANDLE);
};
//申请一个临时文件
HANDLE GetTempFile(wstring& FileName,BOOL Delete = TRUE,const WCHAR *suffix =NULL);


WCHAR* GetWideChar(const char* str);

template<typename T>
void WINAPI StartNewThread(PVOID *Param){
	T *Me = (T*)Param;
	Me->Start();
	delete Me;
}

template<typename T>
VOID CALLBACK TimeoutCallback(PTP_CALLBACK_INSTANCE pInstance,PVOID pvContext,PTP_TIMER pTimer){
	T *Me = (T*)pvContext;
	Me->Start();
}

extern CRITICAL_SECTION cs;

struct NETWORK_IO{
	WSABUF *pBuf;
	SOCKET Client;
};