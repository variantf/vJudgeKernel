#pragma once
#include "RUN.h"
class COMPILE{
private:
	static const int Time_Limit_Exceeded = 2;
	static const int CERROR = 3;
	static const int SUCCESS = 0;
	static const int UNDEFINE_ERROR = -1;

public:
	//编译源代码 Compile：编译命令行 hOut 具有写入权限的文件句柄，用于接收编译信息
	//Time: 编译时间限制 Memory：编译器内存限制
	int Compile(const char *Compile,HANDLE hOut,DWORD Time,DWORD Memory,unsigned int CodeLen);
};
