#include "stdafx.h"
#include "MAINLOOP.h"
#include "COMPILE.h"
#include "TEST.h"
#include "RUN.h"
#include "Delete.h"
DWORD MAINLOOP::port = 0;
HANDLE MAINLOOP::Report = NULL;
SOCKET MAINLOOP::sockSvr = NULL,MAINLOOP::sockConn = NULL;
LONGLONG MAINLOOP::SecurityCode = 0;

BOOL MAINLOOP::Authority(){
	long long Password;
	try{
		readdata(&Password,sizeof(Password));
	}
	catch(...){
		return FALSE;
	}
	return Password == SecurityCode;
}

int MAINLOOP::UnSafeInit(WCHAR* Username,WCHAR* Password){
	HANDLE hToken;
	if (!LogonUser(Username,NULL,Password,LOGON32_LOGON_INTERACTIVE,LOGON32_PROVIDER_DEFAULT,&hToken))
		return false;
	RUN::hToken = hToken;
	return true;
}

int MAINLOOP::LoadConfig(){
	HKEY hKey;
	if(ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE,L"SYSTEM\\CurrentControlSet\\Services\\v-Judge_Kernel",&hKey))
		return false;
	__try{
		DWORD type,sz;
		if(ERROR_SUCCESS != RegQueryValueEx(hKey,L"Username",NULL,&type,NULL,&sz))
			return false;
		WCHAR *Username = (WCHAR*)new BYTE[sz];
		if(ERROR_SUCCESS != RegQueryValueEx(hKey,L"Username",NULL,&type,(LPBYTE)Username,&sz))
			return false;

		sz=sizeof(port);
		if(ERROR_SUCCESS != RegQueryValueEx(hKey,L"Port",NULL,&type,(LPBYTE)&port,&sz))
			return false;

		if(ERROR_SUCCESS != RegQueryValueEx(hKey,L"Password",NULL,&type,NULL,&sz))
			return false;
		WCHAR *Password = (WCHAR*)new BYTE[sz];
		if(ERROR_SUCCESS != RegQueryValueEx(hKey,L"Password",NULL,&type,(LPBYTE)Password,&sz))
			return false;

		if(ERROR_SUCCESS != RegQueryValueEx(hKey,L"Temp",NULL,&type,NULL,&sz))
			return false;
		WCHAR *wTempPath = (WCHAR*)new BYTE[sz];
		if(ERROR_SUCCESS != RegQueryValueEx(hKey,L"Temp",NULL,&type,(LPBYTE)wTempPath,&sz))
			return false;
		CreateDirectory(wTempPath,NULL);
		if(0 == (GetFileAttributes(wTempPath)&FILE_ATTRIBUTE_DIRECTORY))
			return false;
		if(!SetCurrentDirectory(wTempPath))
			return false;
		Delete::Path=wTempPath;
		DWORD tid;
		CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Delete::DeleteThread,NULL,0,&tid);
		delete[] wTempPath;

		sz=sizeof(double);
		if(ERROR_SUCCESS != RegQueryValueEx(hKey,L"tm_Ratio",0,&type,(BYTE*)&RUN::tm_ratio,&sz))
			return false;
		sz=sizeof(double);
		if(ERROR_SUCCESS != RegQueryValueEx(hKey,L"tm_Offset",0,&type,(BYTE*)&RUN::tm_offset,&sz))
			return false;

		sz=sizeof(SecurityCode);
		if(ERROR_SUCCESS != RegQueryValueEx(hKey,L"SecurityCode",0,&type,(BYTE*)&SecurityCode,&sz))
			return false;

		if(!UnSafeInit(Username,Password))
			return false;
	}
	__finally{
		RegCloseKey(hKey);
	}
	return true;
}

MAINLOOP::Out* MAINLOOP::work(MESSAGE* In){
	wstring TmpFile;
	wstring TmpIn;
	wstring TmpOut;
	Out* out=(Out*)new BYTE[sizeof(Out)+MAX_MSG_LEN];
	memset(out,0,sizeof(Out)+MAX_MSG_LEN);
	try{
		switch(In->Mid){
		case M_COMPILE:
			{
				COMPILE_IN *compile=(COMPILE_IN*)new BYTE[(In->Datasize)];
				memcpy(compile,In->DATA,In->Datasize);
				MyHandle hOut = GetTempFile(TmpFile);
				if(INVALID_HANDLE_VALUE == hOut)
					throw runtime_error("TmpFile");
				out->ErrCode = COMPILE::Compile(compile->cmd,hOut,(DWORD)compile->Time,(DWORD)compile->Memory,compile->Codelen);
				Read(hOut,out->Msg,&out->MsgLen);
				delete[] compile;
			}
			break;
		case M_TEST:
			{
				TEST_IN *test=(TEST_IN*)new BYTE[(In->Datasize)];
				memcpy(test,In->DATA,In->Datasize);
				SECURITY_ATTRIBUTES sa;
				sa.bInheritHandle=TRUE;
				sa.lpSecurityDescriptor=NULL;
				sa.nLength=sizeof(sa);
				MyHandle hIn = GetTempFile(TmpIn);
				MyHandle hOut = GetTempFile(TmpOut);
				MyHandle hRes = GetTempFile(TmpFile);
				DWORD written;
				WriteFile(hIn,test->char_data+test->offIn,test->offOut-test->offIn,&written,NULL);
				if(written != test->offOut-test->offIn)
					throw runtime_error("WriteInFile");
				WriteFile(hOut,test->char_data+test->offOut,test->offCmp-test->offOut,&written,NULL);
				if(written != test->offCmp-test->offOut)
					throw runtime_error("WriteOutFile");
				SetFilePointer(hIn,0,NULL,FILE_BEGIN);
				SetFilePointer(hOut,0,NULL,FILE_BEGIN);
				if(INVALID_HANDLE_VALUE == hRes){
					throw runtime_error("TmpFile");
					assert(false);
				}
				if(INVALID_HANDLE_VALUE == hIn){
					throw runtime_error("TmpFile");
					assert(false);
				}
				if(INVALID_HANDLE_VALUE == hOut){
					throw runtime_error("TmpFile");
					assert(false);
				}
				out->ErrCode = TEST::Test_Single(test->char_data+test->offBin,test->char_data+test->offCmp,(DWORD)test->Time,(DWORD)test->Memory,&out->rTime,&out->uMemory,hIn,hOut,hRes);
				Read(hRes,out->Msg,&out->MsgLen);
				out->rTime/=10000;
				delete[] test;
			}
			break;
		}
	}
	catch(runtime_error e){
		ReportMessage(e.what(),GetLastError());
		out->ErrCode = -1;
	}
	return out;
}

void MAINLOOP::readdata(void* buf,int sz){
	char* ptr=(char*)buf,*ed=(char*)buf+sz;
	while(ptr<ed){
		int recved = recv(sockConn,ptr,ed-ptr,0);
		if(recved == SOCKET_ERROR)
			throw runtime_error("Network Error Recv");
		if(recved == 0)
			throw runtime_error("Network Disconnected");
		ptr+=recved;
	}
	SYSTEMTIME t;
	GetLocalTime(&t);
	printf("%d-%d-%d %d:%d:%d Read byte(s) %d\n",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,sz);
}

MAINLOOP::MESSAGE* MAINLOOP::getmessage(){


	int DataHead[2];
	readdata((char*)DataHead,8);
	MESSAGE* buffer =(MESSAGE*) new BYTE[DataHead[1]+8];
	buffer->Datasize=DataHead[1];
	buffer->Mid=DataHead[0];
	readdata((char*)buffer->DATA,buffer->Datasize);
	return buffer;
}

void MAINLOOP::sendback(void *Data,int sz){
	char *ptr=(char*)Data,*ed=(char*)Data+sz;
	while(ptr<ed){
		int sended = send(sockConn,ptr,ed-ptr,0);
		if(SOCKET_ERROR == sended){
			throw runtime_error("Network Error Send");
		}
		if(0 == sended)
			throw runtime_error("Network Disconnected");
		ptr+=sended;
	}
	SYSTEMTIME t;
	GetLocalTime(&t);
	printf("%d-%d-%d %d:%d:%d Send byte(s) %d\n",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,sz);
}

void MAINLOOP::init(){
	//版本协商
	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(1,1); //0x0101
	if(WSAStartup(wVersionRequested,&wsaData)){
		throw runtime_error("WSAStartup");
	}
	if(LOBYTE(wsaData.wVersion)!=1 || HIBYTE(wsaData.wVersion)!=1) 
		throw runtime_error("Network Error WSAStartup");
	//创建Socket
	sockSvr = socket(AF_INET,SOCK_STREAM,0);
	//创建IP地址和端口
	SOCKADDR_IN addrSvr;
	addrSvr.sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);
	addrSvr.sin_family = AF_INET;
	addrSvr.sin_port = htons((u_short)port);
	//绑定端口监听
	if(SOCKET_ERROR == bind(sockSvr,(SOCKADDR*)&addrSvr,sizeof(SOCKADDR)))
		throw runtime_error("Network Error bind");
	if(SOCKET_ERROR == listen(sockSvr,SOMAXCONN))
		throw runtime_error("Network Error listen");

	sockaddr_in addrClient;
	int len = sizeof(sockaddr);

	sockConn = accept(sockSvr,(sockaddr*)&addrClient,&len);

	if(INVALID_SOCKET == sockConn)
		throw runtime_error("accept");

	int nNetTimeout=5000;
	//设置发送超时
	setsockopt(sockConn,SOL_SOCKET,SO_SNDTIMEO,(char*)&nNetTimeout,sizeof(int));
	//设置接收超时
	setsockopt(sockConn,SOL_SOCKET,SO_RCVTIMEO,(char*)&nNetTimeout,sizeof(int));

	if(!Authority())
		throw runtime_error("Authority Error!");

	SYSTEMTIME t;
	GetLocalTime(&t);
	printf("%d-%d-%d %d:%d:%d Client Connected : %d.%d.%d.%d\n",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,addrClient.sin_addr.S_un.S_un_b.s_b1,addrClient.sin_addr.S_un.S_un_b.s_b2,
		addrClient.sin_addr.S_un.S_un_b.s_b3,addrClient.sin_addr.S_un.S_un_b.s_b4);
}

void MAINLOOP::main(){
	Report = RegisterEventSource(NULL,L"v-Judge_Kernel");
	if(NULL == Report)	return;
	ReportMessage("Server Started");
	if(LoadConfig())
		while(TRUE){
			try{
				init();
				while(MESSAGE *Data = getmessage()){
					Out* rData = work(Data);
					sendback(rData,sizeof(Out)+rData->MsgLen-1);
					delete[] Data;
					delete[] rData;
				}
			}
			catch(runtime_error e){
				if(WSAGetLastError()){
					ReportMessage(e.what(),WSAGetLastError());
				}
			}
			closesocket(sockSvr);
			WSACleanup();
		}
	else
		ReportMessage("LoadConfig",-1);
}

void MAINLOOP::ReportMessage(string msg,DWORD E){
		SYSTEMTIME t;
		GetLocalTime(&t);
		stringstream s;
		s<<t.wYear<<"-"<<t.wMonth<<"-"<<t.wDay<<" "<<t.wHour<<":"<<t.wMinute<<":"<<t.wSecond<<" "<<msg<<" "<<E<<endl;
		cout<<s;
		char *buf=new char[s.str().length()+1];
		memcpy_s(buf,s.str().length(),s.str().c_str(),s.str().length());
		buf[s.str().length()]=0;
		ReportEventA(Report,E?EVENTLOG_ERROR_TYPE:EVENTLOG_SUCCESS,0,0,NULL,1,0,(LPCSTR*)&buf,NULL);
}
