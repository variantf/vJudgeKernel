#include "StdAfx.h"
#include "NETWORK.h"

DWORD NETWORK::port = 0;

NETWORK::NETWORK(void)
{
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
	addrSvr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrSvr.sin_family = AF_INET;
	addrSvr.sin_port = htons((u_short)port);
	//绑定端口监听
	if(SOCKET_ERROR == bind(sockSvr,(SOCKADDR*)&addrSvr,sizeof(SOCKADDR)))
		throw runtime_error("Network Error bind");
	if(SOCKET_ERROR == listen(sockSvr,SOMAXCONN))
		throw runtime_error("Network Error listen");
	if ((hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL) 
		throw runtime_error("CreateIoCompletionPort");
    SYSTEM_INFO SystemInfo; 
    GetSystemInfo(&SystemInfo); 
    for(unsigned int i=0; i<SystemInfo.dwNumberOfProcessors*2; i++) 
    { 
            HANDLE ThreadHandle; 
			DWORD ThreadID;
			if ((ThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MAIN::WaitThreadpool, hIOCP, 0, &ThreadID)) == NULL) 
				throw runtime_error("Create WaitThreadpool");
            CloseHandle(ThreadHandle); 
    } 
}


NETWORK::~NETWORK(void)
{
}

void NETWORK::Start(){
	try{
		REPORT_MESSAGE *Log = new REPORT_MESSAGE();
		Log->Report("Network Started!");
		try{
			sockaddr_in addrClient;
			int len = sizeof(sockaddr);
			while(TRUE){
				SOCKET sockConn = accept(sockSvr,(sockaddr*)&addrClient,&len);
				if(INVALID_SOCKET == sockConn)
					throw runtime_error("accept");
				WSABUF *buf = new WSABUF;
				buf->len=sizeof(LONGLONG);
				buf->buf = new char[sizeof(LONGLONG)];
				NETWORK_IO *net_io = new NETWORK_IO;
				net_io->Client=sockConn;
				net_io->pBuf=buf;
				WSAOVERLAPPED *Overlapped = new WSAOVERLAPPED;
				memset(Overlapped,0,sizeof(WSAOVERLAPPED));
				RecvFlag = 0;
				if (CreateIoCompletionPort((HANDLE)sockConn, hIOCP, (ULONG_PTR)net_io, 0) == NULL)
					throw runtime_error("AssignDeviceWithIOCP");
                if(WSARecv(sockConn, buf, 1, NULL, &RecvFlag, Overlapped, NULL) == SOCKET_ERROR) 
                    if(WSAGetLastError() != WSA_IO_PENDING) 
						throw runtime_error("WSARecv");
			}
		}
		catch(runtime_error e){
			Log->Report(e.what(),WSAGetLastError());
		}
	}
	catch(...){
		return;
	}
}
