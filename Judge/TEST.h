#pragma once
#include "RUN.h"
class TEST{
private:
	//传说中的Error Code
	enum{
		ACCEPT,
		WRONG_ANSWER,
		Time_Limit_Exceeded,
		Runtime_Error,
		Memory_Limit_Exceeded,
		CMP_ERROR,
		Output_Limit_Execeeded,
		TERROR = -1
	};
	
	//根据PE文件头获得程序静态内存空间
	DWORD GetStaticMemory(const WCHAR* path);
	
public:
	//测试单一测试点 path是二进制文件路径 compare 是比较器dll路径，Time 时间限制 Memory 内存限制
	//rTime 实际时间 uMemory 实际内存 hIn 可读文件 用于 stdin  hOut 可写文件用于 stdout
	//hRes 可写文件，用于比较器返回文本信息以及stderr
	int 
	Test_Single(const char *path,const char *compare,DWORD Time,DWORD Memory,
			long long* rTime,long long* uMemory,HANDLE hIn,HANDLE hOut,HANDLE hRes,
			wstring inputFile,wstring outputFile);
};
