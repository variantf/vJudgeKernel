#include "StdAfx.h"
#include "MAIN.h"
#include "DELETE_TEMP.h"

LONGLONG MAIN::SecurityCode = 0;

MAIN::MAIN(SOCKET _Client)
{
	sockConn = _Client;
	DWORD nNetTimeout=5000;
	//ÉèÖÃ·¢ËÍ³¬Ê±
	if(0 != setsockopt(sockConn,SOL_SOCKET,SO_SNDTIMEO,(char*)&nNetTimeout,sizeof(int)))
		throw runtime_error("set network send timeout error");
	if(0 != setsockopt(sockConn,SOL_SOCKET,SO_RCVTIMEO,(char*)&nNetTimeout,sizeof(int)))
		throw runtime_error("set network recv timeout error");
	Log = new REPORT_MESSAGE();
}


MAIN::~MAIN(void)
{
	closesocket(sockConn);
	delete Log;
}

void MAIN::Start(){
	try{
		//lock
		EnterCriticalSection(&cs);
		while(MESSAGE *Data = getmessage()){
			Out* rData = work(Data);
			sendback(rData,sizeof(Out)+rData->MsgLen-1);
			delete[] Data;
			delete[] rData;
		}
	}
	catch(runtime_error e){
		if(WSAGetLastError()){
			Log->Report(e.what(),WSAGetLastError());
		}
	}
	//unlock
	DELETE_TEMP Delete;
	Delete.Start();
	LeaveCriticalSection(&cs);
}

MAIN::Out* MAIN::work(MESSAGE* In){
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
				COMPILE *Compile=new COMPILE();
				out->ErrCode = Compile->Compile(compile->cmd,hOut,(DWORD)compile->Time,(DWORD)compile->Memory,compile->Codelen);
				Read(hOut,out->Msg,&out->MsgLen);
				delete[] compile;
				delete Compile;
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
				TEST *Test = new TEST();
				out->ErrCode = Test->Test_Single(test->char_data+test->offBin,test->char_data+test->offCmp,(DWORD)test->Time,(DWORD)test->Memory,&out->rTime,&out->uMemory,hIn,hOut,hRes);
				Read(hRes,out->Msg,&out->MsgLen);
				out->rTime/=10000;
				delete[] test;
				delete Test;
			}
			break;
		}
	}
	catch(runtime_error e){
		Log->Report(e.what(),GetLastError());
		out->ErrCode = -1;
	}
	return out;
}

void MAIN::readdata(void* buf,int sz){
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

MAIN::MESSAGE* MAIN::getmessage(){
	int DataHead[2];
	readdata((char*)DataHead,8);
	MESSAGE* buffer =(MESSAGE*) new BYTE[DataHead[1]+8];
	buffer->Datasize=DataHead[1];
	buffer->Mid=DataHead[0];
	readdata((char*)buffer->DATA,buffer->Datasize);
	return buffer;
}

void MAIN::sendback(void *Data,int sz){
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

void MAIN::WaitThreadpool(PVOID hIOCP){
	NETWORK_IO *Msg = NULL;
	SOCKET Client = NULL;
	OVERLAPPED *Overlapped;
	DWORD BytesRead;
	while(TRUE){
		if(GetQueuedCompletionStatus(hIOCP,&BytesRead,(PULONG_PTR)&Msg,&Overlapped,INFINITE)){
			if(BytesRead == sizeof(LONGLONG)){
				LONGLONG Password;
				memcpy(&Password,Msg->pBuf->buf,sizeof(LONGLONG));
				if(Password == SecurityCode){
					MAIN *Work = new MAIN(Msg->Client);
					Work->Start();
					delete Work;
				}else
					closesocket(Msg->Client);
			}
			delete[] Msg->pBuf->buf;
			delete Msg->pBuf;
			delete Msg;
			delete Overlapped;
		}
	}
}