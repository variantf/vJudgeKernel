#include "stdafx.h"
#include "TEST.h"


DWORD TEST::GetStaticMemory(const WCHAR* path){
	MyHandle hPE = CreateFileW(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(INVALID_HANDLE_VALUE == hPE){
		throw runtime_error("CreateFileA");
		assert(false);
	}
	IMAGE_DOS_HEADER idh;
	DWORD cnt;
	if(!ReadFile(hPE,&idh,sizeof(idh),&cnt,NULL)){
		throw runtime_error("ReadFile");
		assert(false);
	}
	if(INVALID_SET_FILE_POINTER == SetFilePointer(hPE,idh.e_lfanew,NULL,FILE_BEGIN)){
		throw runtime_error("SetFilePointer");
		assert(false);
	}
	IMAGE_NT_HEADERS inh;
	if(!ReadFile(hPE,&inh,sizeof(inh),&cnt,NULL)){
		throw runtime_error("ReadFile");
		assert(false);
	}
	return inh.OptionalHeader.SizeOfImage;

}

int TEST::Test_Single(const char *path,const char *compare,DWORD Time,DWORD Memory,
		long long* rTime,long long* uMemory,HANDLE hIn,HANDLE hOut,HANDLE hRes,
		wstring inputFile,wstring outputFile){
	DWORD exitcode = TERROR;
	BOOL isTLE;
	int result = TERROR;
	wstring TmpOutFile;
	MyHandle hTmp = GetTempFile(TmpOutFile);
	if(INVALID_HANDLE_VALUE == hTmp){
		throw runtime_error("GetTempFile");
		assert(false);
	}
	WCHAR* cmd = GetWideChar(path);
	DWORD sMemory = 0;
	if(PathFileExistsW(cmd) && wstring(cmd).substr(wcslen(cmd)-3,3) == wstring(L"exe"))
		sMemory = GetStaticMemory(cmd);
	if(Memory<sMemory){
		return Memory_Limit_Exceeded;
	}
	if(!SetHandleInformation(hOut,HANDLE_FLAG_INHERIT,0))
		throw runtime_error("SetHandleInherit 0");
	RUN *Run = new RUN();
	if(*compare)
		isTLE = Run->Run_Binary(cmd,Time,Memory,hIn,hTmp,hRes,1,rTime,uMemory,TRUE,NULL,&exitcode);
	else
		isTLE = Run->Run_Binary(cmd,Time,Memory,hIn,hRes,hRes,1,rTime,uMemory,TRUE,NULL,&exitcode);
	delete[] cmd;
	if(isTLE)
		result = Time_Limit_Exceeded;
	else
	if(Memory < *uMemory)
		result = Memory_Limit_Exceeded;
	else
	if(*compare){
		if(exitcode != 0)
			result = Runtime_Error;
		else{
			if(!SetHandleInformation(hOut,HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT))
				throw runtime_error("SetHandleInherit");
			LONGLONG rTime,uMemory;
			WCHAR * wcmp = GetWideChar(compare);
			wstring cmp = wstring(wcmp).append(L" ").append(outputFile).append(L" ");
			cmp.append(TmpOutFile).append(L" ");
			cmp.append(inputFile);
			isTLE = Run->Run_Binary(wcmp,Time,Memory,NULL,hRes,hRes,1,&rTime,&uMemory,TRUE,NULL,&exitcode);
			delete[] wcmp;
			if((exitcode != ACCEPT && exitcode != WRONG_ANSWER && exitcode != Output_Limit_Execeeded) || isTLE)
				result = CMP_ERROR;
			else
				result = exitcode;
		}
	}else{
		if(exitcode != ACCEPT && exitcode != WRONG_ANSWER)
			result = CMP_ERROR;
		else
			result = exitcode; 
	}
	delete Run;
	return result;
}