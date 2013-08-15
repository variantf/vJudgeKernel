#include "StdAfx.h"
#include "MAIN.h"
#include "DELETE_TEMP.h"
#include "InitSrv.h"

LONGLONG MAIN::SecurityCode = 0;

MAIN::MAIN(SOCKET _Client)
{
	sockConn = _Client;
	DWORD nNetTimeout=5000;
	//…Ë÷√∑¢ÀÕ≥¨ ±
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
		//EnterCriticalSection(&cs);
		//while(MESSAGE *Data = getmessage()){
		while(map<string,wstring> *Data=getmessage()){
			RUNNER_OUT* rData;
			try{
				 rData = work(Data);
			}catch(runtime_error e){
				stringstream ss;
				ss<<e.what()<<"  "<<GetLastError();
				rData=(RUNNER_OUT*)new BYTE[ss.str().length()+4];
				rData->sz=ss.str().length();
				strcpy(rData->DATA,ss.str().c_str());
			}
			sendback(rData,sizeof(RUNNER_OUT)+rData->sz-1);
			delete Data;
			delete[] rData;
		}
	}
	catch(runtime_error e){
		if(WSAGetLastError()){
			Log->Report(e.what(),WSAGetLastError());
		}
	}
	//unlock
	//DELETE_TEMP Delete;
	//Delete.Start();
	//LeaveCriticalSection(&cs);
}

MAIN::RUNNER_OUT* MAIN::work(map<string,wstring>* In){
	SECURITY_ATTRIBUTES sa;
	sa.bInheritHandle=TRUE;
	sa.lpSecurityDescriptor=NULL;
	sa.nLength=sizeof(sa);

	MyHandle hIn=CreateFile((*In)["standard-in"].c_str(),GENERIC_READ,
		FILE_SHARE_READ|FILE_SHARE_WRITE,&sa,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(INVALID_HANDLE_VALUE == hIn)
		throw runtime_error("CreateFile stdin");
	MyHandle hOut=CreateFile((*In)["standard-out"].c_str(),GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE,&sa,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(INVALID_HANDLE_VALUE == hOut)
		throw runtime_error("CreateFile stdout");
	MyHandle hErr=CreateFile((*In)["standard-err"].c_str(),GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE,&sa,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(INVALID_HANDLE_VALUE == hErr)
		throw runtime_error("CreateFile stderr");
	RUN r;
	DWORD time_lim,exit;
	size_t memory_lim;
	LONGLONG uMemory,rTime;
	wstringstream ss((*In)["memory-limit"]+TEXT("  ")+(*In)["time-limit"]);
	ss>>memory_lim>>time_lim;
	BOOL isTLE;
	if((*In)["restriction-level"]==TEXT("loose")){
		isTLE=r.Run_Binary((WCHAR*)(*In)["command"].c_str(),time_lim,memory_lim,hIn,hOut,hErr,10,
			&rTime,&uMemory,TRUE,(WCHAR*)(*In)["working-directory"].c_str(),&exit);
	}else{
		isTLE=r.Run_Binary((WCHAR*)(*In)["command"].c_str(),time_lim,memory_lim,hIn,hOut,hErr,1,
			&rTime,&uMemory,TRUE,(WCHAR*)(*In)["working-directory"].c_str(),&exit);
	}
	stringstream rets;
	rets<<"exit-code:"<<exit<<"\ntime:"<<rTime/10000<<"\nmemory:"<<uMemory<<"\ntype:";
	if(isTLE){
		rets<<"TLE\n";
	}else if(memory_lim<uMemory || r.isMLE ){
		rets<<"MLE\n";
	}else if(exit!=0){
		rets<<"FAILURE\n";
	}else{
		rets<<"SUCCESS\n";
	}
	RUNNER_OUT *ret=(RUNNER_OUT*)new BYTE[4+rets.str().length()];
	ret->sz=rets.str().length();
	strcpy(ret->DATA,rets.str().c_str());
	return ret;
}
/*
MAIN::Out* MAIN::work(map<string,string>* In){
	wstring TmpFile;
	wstring TmpIn;
	wstring TmpOut;
	Out* out=(Out*)new BYTE[sizeof(Out)];
	memset(out,0,sizeof(Out));

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
				ReadResFile(hOut,&out);
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
				MyHandle hIn = GetTempFile(TmpIn,0);
				MyHandle hOut = GetTempFile(TmpOut,0);
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
				out->ErrCode = Test->Test_Single(test->char_data+test->offBin,test->char_data+test->offCmp,(DWORD)test->Time,(DWORD)test->Memory,&out->rTime,&out->uMemory,hIn,hOut,hRes,TmpIn,TmpOut);
				ReadResFile(hRes,&out);
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
*/

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
	/*
	SYSTEMTIME t;
	GetLocalTime(&t);
	FILE *flog = fopen("C:\\log.txt","a");
	fprintf(flog,"%d-%d-%d %d:%d:%d Read byte(s) %d\n",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,sz);
	fwrite(buf,sz,1,flog);
	fputs("",flog);
	fclose(flog);
	*/
}

/*
MAIN::MESSAGE* MAIN::getmessage(){
	int DataHead[2];
	readdata((char*)DataHead,8);
	MESSAGE* buffer =(MESSAGE*) new BYTE[DataHead[1]+8];
	buffer->Datasize=DataHead[1];
	buffer->Mid=DataHead[0];
	readdata((char*)buffer->DATA,buffer->Datasize);
	return buffer;
}
*/
map<string,wstring>* MAIN::getmessage(){
	int len;
	readdata((char*)&len,4);
	//4 bytes for length and last one for \0
	RUNNER* buffer = (RUNNER*)new BYTE[len+5];
	memset(buffer,0,len+5);
	buffer->Datasize=len;
	readdata((char*)buffer->DATA,len);
	stringstream stm(buffer->DATA);
	string line;
	map<string,wstring> *ret=new map<string,wstring>();
	while(getline(stm,line)){
		string::size_type pos = line.find(':');
		if(pos==string::npos)
			throw runtime_error("Bad Config");
		WCHAR* buf=GetWideChar(line.substr(pos+1,line.length()-pos-1).c_str());
		(*ret)[line.substr(0,pos)]=buf;
		delete[] buf;
	}
	if((*ret)["standard-in"][0]=='.' && (*ret)["standard-in"][1]=='/')
		(*ret)["standard-in"]=(*ret)["working-directory"]+(*ret)["standard-in"].substr(2,(*ret)["standard-in"].length()-2);
	if((*ret)["standard-out"][0]=='.' && (*ret)["standard-out"][1]=='/')
		(*ret)["standard-out"]=(*ret)["working-directory"]+(*ret)["standard-out"].substr(2,(*ret)["standard-out"].length()-2);
	if((*ret)["standard-err"][0]=='.' && (*ret)["standard-err"][1]=='/')
		(*ret)["standard-err"]=(*ret)["working-directory"]+(*ret)["standard-err"].substr(2,(*ret)["standard-err"].length()-2);
	wstring cmd=(*ret)["command"];
	if(cmd[0]=='.'&&cmd[1]=='/')
		(*ret)["command"]=(*ret)["working-directory"]+cmd.substr(2,cmd.length()-2);
	return ret;
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
					MAIN* Work = new MAIN(Msg->Client);
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

void MAIN::JobMessage(PVOID){
	DWORD type;
	LPOVERLAPPED overlape;
	RUN* ptr;
	while(TRUE){
		if(GetQueuedCompletionStatus(InitSrv::jobIocp,&type,(PULONG_PTR)&ptr,&overlape,INFINITE)){
			if(type==JOB_OBJECT_MSG_PROCESS_MEMORY_LIMIT || type==JOB_OBJECT_MSG_JOB_MEMORY_LIMIT){
				ptr->isMLE=true;
			}
		}
	}
}

void MAIN::ReadResFile(HANDLE hFile,Out **out) {
	if(INVALID_SET_FILE_POINTER == SetFilePointer(hFile,0,NULL,FILE_BEGIN))
		throw runtime_error("SetFilePointer");
	DWORD fsize=GetFileSize(hFile,NULL);
	if(INVALID_FILE_SIZE==fsize)
		throw runtime_error("GetFileSize");
	Out *new_out=new Out[sizeof(Out)+fsize];
	memcpy(new_out,*out,sizeof(Out));
	new_out->MsgLen=fsize;
	if(!ReadFile(hFile,new_out->Msg,fsize,(PDWORD)&new_out->MsgLen,NULL))
		throw runtime_error("ReadFile");
	delete *out;
	*out=new_out;
}