#pragma once
#include "REPORT_MESSAGE.h"
#include "COMPILE.h"
#include "TEST.h"
#include "RUN.h"
class MAIN
{
private:
	static const int MAX_MSG_LEN = 10 * 1024;
	static const int M_COMPILE = 1;
	static const int M_TEST = 2;
	static const int M_INTERACTIVE = 4;
	static const int OUT_SIZE = 24;

	SOCKET sockConn;
	struct MESSAGE{
		//测评类型：M_COMPILE/M_TEST
		unsigned int Mid;
		//DATA数组实际长度
		unsigned int Datasize;
		unsigned char DATA[1];
	};

	struct RUNNER{
		unsigned int Datasize;
		char DATA[1];
	};

	struct RUNNER_OUT{
		unsigned int sz;
		char DATA[1];
	};

	//接收的编译信息结构
	struct COMPILE_IN{
		//编译限制时间
		long long Time;
		//编译限制内存
		long long Memory;
		
		unsigned int Codelen;
		//编译命令行 {Source}{S.cpp} {Execute}{E.exe}
		char cmd[1];
	};

	//返回测评结构信息结构
	struct Out{
		//返回文本信息长度
		unsigned int MsgLen;
		//错误代码
		unsigned int ErrCode;
		//测评实际运行时间
		long long rTime;
		//测评实际使用内存
		long long uMemory;
		//文本信息
		char Msg[1];
	};

	//接收测评信息结构
	struct TEST_IN{
		//二进制文件命令行在char_data中偏移
		unsigned int offBin;
		//输入文件在char_data中偏移
		unsigned int offIn;
		//输出偏移
		unsigned int offOut;
		//比较器地址
		unsigned int offCmp;
		//限制时间
		long long Time;
		//限制内存
		long long Memory;
		//文本数据
		char char_data[1];
	};

	//测评工作核心
	RUNNER_OUT* work(map<string,wstring>* In);

	//获得指定size的数据
	void readdata(void* buf,int sz);

	//取得测评信息
	map<string,wstring>* getmessage();

	//发送指定size的数据
	void sendback(void *Data,int sz);

	REPORT_MESSAGE *Log;

	void ReadResFile(HANDLE ,Out **);

public:
	static long long SecurityCode;
	static void WaitThreadpool(PVOID);
	static void JobMessage(PVOID);
	MAIN(SOCKET _Client);
	~MAIN(void);
	void Start();
};

