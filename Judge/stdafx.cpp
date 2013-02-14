// stdafx.cpp : 只包括标准包含文件的源文件
// Judge.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

// TODO: 在 STDAFX.H 中
// 引用任何所需的附加头文件，而不是在此文件中引用

MyHandle::MyHandle(HANDLE h):_h(h){
}

MyHandle::MyHandle(){
		_h=0;
}

MyHandle::~MyHandle(){
	if(_h)
		CloseHandle(_h),_h=NULL;
}

HANDLE MyHandle::operator=(HANDLE h){
	if(_h)
		CloseHandle(_h);
	_h=h;
	return _h;
}

HANDLE* MyHandle::operator&(){
		return &_h;
}

MyHandle::operator HANDLE(){
	return _h;
}

HANDLE GetTempFile(wstring& FileName,BOOL Delete,const WCHAR * suffix){
	WCHAR TmpOutFile[MAX_PATH];
	if(!GetTempFileNameW(L".",L"v-J_K",0,TmpOutFile)){
		throw runtime_error("GetTempFileName");
		assert(false);
	}
	FileName = TmpOutFile;
	if(suffix){
		FileName.append(suffix);
		DeleteFileW(TmpOutFile);
	}
	SECURITY_ATTRIBUTES sa;
	sa.bInheritHandle=TRUE;
	sa.lpSecurityDescriptor=NULL;
	sa.nLength=sizeof(sa);
	return CreateFileW(FileName.c_str(),GENERIC_WRITE|GENERIC_READ,FILE_SHARE_READ,&sa,CREATE_ALWAYS,FILE_ATTRIBUTE_TEMPORARY|((Delete)?FILE_FLAG_DELETE_ON_CLOSE:0),NULL);

}

void Read(HANDLE hFile,char* buffer,unsigned int* buf_read){
	if(INVALID_SET_FILE_POINTER == SetFilePointer(hFile,0,NULL,FILE_BEGIN))
		throw runtime_error("SetFilePointer");
	if(!ReadFile(hFile,buffer,10 * 1024,(PDWORD)buf_read,NULL))
		throw runtime_error("ReadFile");
}


WCHAR* GetWideChar(const char * str){
		DWORD len=strlen(str);
		WCHAR *cmd=new WCHAR[len+5];
		memset(cmd,0,sizeof(WCHAR)*(len+5));
		MultiByteToWideChar(CP_ACP,0,str,len,cmd,len+5);
		return cmd;
}

CRITICAL_SECTION cs;
