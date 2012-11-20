#include "StdAfx.h"
#include "DELETE_TEMP.h"

void DELETE_TEMP::Start(){
	WIN32_FIND_DATAW fdat;
	HANDLE h = FindFirstFileW(L"*",&fdat);
	do{
		if(wcscmp(fdat.cFileName,L".") ==0 ||
			wcscmp(fdat.cFileName,L"..") == 0)
			continue;
		SYSTEMTIME sysnow;
		FILETIME fnow;
		GetSystemTime(&sysnow);
		SystemTimeToFileTime(&sysnow,&fnow);
		ULONGLONG tm = ((ULONGLONG)(fnow.dwHighDateTime - fdat.ftLastAccessTime.dwHighDateTime)<<32ULL)+
			(fnow.dwLowDateTime-fdat.ftLastAccessTime.dwLowDateTime);
		if(tm > PER_SECOND * 300){
			SHFILEOPSTRUCTW shf;
			memset(&shf,0,sizeof(shf));
			shf.fFlags = FOF_NO_UI;
			shf.wFunc = FO_DELETE;
			int sz;
			WCHAR *wPathName=new WCHAR[sz = Path.size()+wcslen(fdat.cFileName)+5];
			wcscpy_s(wPathName,sz,Path.c_str());
			wcscat_s(wPathName,sz,fdat.cFileName);
			wPathName[sz-4] = '\0';
			shf.pFrom = wPathName;
			SHFileOperationW(&shf);
			delete[] wPathName;
		}
	}while(FindNextFileW(h,&fdat));
	FindClose(h);
}