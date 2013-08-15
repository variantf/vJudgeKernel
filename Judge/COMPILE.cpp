#include "stdafx.h"
#include "COMPILE.h"
int COMPILE::Compile(const char *char_data,HANDLE hOut,DWORD Time,DWORD Memory,unsigned int CodeLen){
	long long m;
	long long t;
	SECURITY_ATTRIBUTES sa;
	sa.bInheritHandle=TRUE;
	sa.lpSecurityDescriptor=NULL;
	sa.nLength=sizeof(sa);
	if(INVALID_HANDLE_VALUE == hOut)	return UNDEFINE_ERROR;

	WCHAR *tcmd = GetWideChar(char_data+CodeLen);
	wstring cmd(tcmd),Ssuffix,Esuffix;
	delete[] tcmd;
	wstring Sname,Ename;
	{
		size_t posl = cmd.find(L"{S."),posr;
		if(posl != cmd.npos){
			posr = cmd.find(L"S}");
			if(posr != cmd.npos){
				Ssuffix = cmd.substr(posl+2,posr-posl-2);
				cmd.erase(posl,posr-posl+2);
			}
		}
		posl = cmd.find(L"{E.");
		if(posl != cmd.npos){
			posr = cmd.find(L"E}");
			if(posr != cmd.npos){
				Esuffix = cmd.substr(posl+2,posr-posl-2);
				cmd.erase(posl,posr-posl+2);
			}
		}

		MyHandle hSrc,hExe;
		if(cmd.find(L"{Source}")!=cmd.npos){
			hSrc = GetTempFile(Sname,FALSE,Ssuffix.c_str());
			cmd.replace(cmd.find(L"{Source}"),strlen("{Source}"),wstring(L"\"").append(Sname).append(L"\""));
		}else
		if((posl=cmd.find(L"{Source="))!=cmd.npos){
			posr = cmd.find(L"}",posl);
			Sname = cmd.substr(posl+8,posr-posl-8).append(Ssuffix);
			hSrc = CreateFileW(Sname.c_str(),GENERIC_WRITE|GENERIC_READ,0,&sa,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
			cmd.replace(posl,posr-posl+1,wstring(L"\"").append(Sname).append(L"\""));
		}
		DWORD written;
		if(hSrc != NULL && (!WriteFile(hSrc,char_data,CodeLen,&written,NULL))){
			throw runtime_error("WriteCode");
			if(written != CodeLen)
				throw runtime_error("WriteCode");
		}
		if(cmd.find(L"{Execute}")!=cmd.npos){
			hExe = GetTempFile(Ename,FALSE,Esuffix.c_str());
			cmd.replace(cmd.find(L"{Execute}"),strlen("{Execute}"),wstring(L"\"").append(Ename).append(L"\""));
		}else
		if((posl = cmd.find(L"{Execute="))!=cmd.npos){
			posr = cmd.find(L"}",posl);
			Ename = cmd.substr(posl+9,posr-posl-9).append(Esuffix);
			hExe = CreateFileW(Ename.c_str(),GENERIC_WRITE|GENERIC_READ,0,&sa,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
			cmd.replace(posl,posr-posl+1,wstring(L"\"").append(Ename).append(L"\""));
		}
	}

	DWORD ext;
	RUN *Run = new RUN();
	BOOL isTLE = Run->Run_Binary((WCHAR*)cmd.c_str(),Time,Memory,NULL,hOut,hOut,10,&t,&m,FALSE,NULL,&ext);
	delete Run;
	int result = UNDEFINE_ERROR;
	if(ext == 0 && (!isTLE)){
		SetFilePointer(hOut,0,NULL,FILE_BEGIN);
		SetEndOfFile(hOut);
		DWORD written;
		int cb;
		cb = WideCharToMultiByte(CP_ACP,0,Ename.c_str(),-1,NULL,0,NULL,NULL);
		char *buf=new char[cb];
		WideCharToMultiByte(CP_ACP,0,Ename.c_str(),-1,buf,cb,NULL,NULL);
		WriteFile(hOut,buf,cb,&written,NULL);
		delete[] buf;
		result = SUCCESS;
	}else{
		DeleteFileW(Ename.c_str());
		if(isTLE)
			result = Time_Limit_Exceeded;
		else
			result = CERROR;
	}
	DeleteFileW(Sname.c_str());
	return result;
}