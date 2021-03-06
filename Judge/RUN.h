#pragma once
#include "stdafx.h"
#include "InitSrv.h"

class Environment{
private:
	vector<WCHAR> data;
	size_t noPath;
public:
	Environment();
	~Environment();
	void Add(wstring name,wstring value);
	operator LPVOID();
};

class Token{
private:
	HANDLE _token;
public:
	Token();
	~Token();
	BOOL InitWithAuth(WCHAR* username,WCHAR* password);
	operator HANDLE();
};

class RUN{
private:
	//获得Job中全部进程句柄
	DWORD GetAllProcessHandle(HANDLE hJob);

	VOID SetLimit(HANDLE hJob,DWORD Time,size_t Memory,int NumberOfProcess);

	VOID CreatePro(WCHAR* cmd,HANDLE hIn,HANDLE hOut,HANDLE hErr,BOOL AsUser,WCHAR* Directory,PPROCESS_INFORMATION pi);

	LONGLONG GetStaticMemory(wstring File);

	BOOL DebugMainLoop(DWORD Time,LPDWORD Exitcode,LPPROCESS_INFORMATION pi,BOOL AsUser);

	VOID dealException(EXCEPTION_RECORD *rec,LPDWORD Exitcode);

	//用于存放Job中所有进程句柄
	HANDLE h[MAXIMUM_WAIT_OBJECTS];

	DWORD CpuMask;

public:
	RUN();
	//执行一个二进制代码。cmd:执行二进制代码的命令行 Time:执行时间限制
	//Memory：执行程序内存限制 hIn 一个具有可读权限的文件句柄，为程序的stdin
	//hOut：一个具有可写权限的文件句柄，为程序stdout
	//hErr:一个具有可写权限的文件句柄，为程序stderr
	//NumberOfProcess:一个进程树最多的进程数量（用于限制程序按创建子进程）
	//rTime:指向long long的指针，用于接收程序实际运行时间
	//uMemory:指向long long 指针，用于接收程序实际运行内存
	//User:当AsUser为TRUE时，以User用户运行
	//Password:当以其他用户身份运行时，此处为User的密码
	//AsUser:是否以其他用户身份运行
	BOOL Run_Binary(WCHAR* cmd,DWORD Time,size_t Memory,HANDLE hIn,HANDLE hOut,HANDLE hErr,
		DWORD NumberOfProcess,PLONGLONG rTime,PLONGLONG uMemory,BOOL AsUser,WCHAR* Directory, PDWORD ExitCode);

	static double tm_offset,tm_ratio;

	static Token hToken;

	static Environment environment;

	atomic<bool> isMLE;

};
