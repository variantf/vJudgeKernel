#pragma once
class MAINLOOP{
public:
	static const int MAX_MSG_LEN = 10 * 1024;
	static const int M_COMPILE = 1;
	static const int M_TEST = 2;
	static const int M_INTERACTIVE = 4;
	static const int OUT_SIZE = 24;

	#define DESKTOP_ALL (DESKTOP_READOBJECTS | DESKTOP_CREATEWINDOW | \
	DESKTOP_CREATEMENU | DESKTOP_HOOKCONTROL | DESKTOP_JOURNALRECORD | \
	DESKTOP_JOURNALPLAYBACK | DESKTOP_ENUMERATE | DESKTOP_WRITEOBJECTS | \
	DESKTOP_SWITCHDESKTOP | STANDARD_RIGHTS_REQUIRED)

	#define WINSTA_ALL (WINSTA_ENUMDESKTOPS | WINSTA_READATTRIBUTES | \
	WINSTA_ACCESSCLIPBOARD | WINSTA_CREATEDESKTOP | \
	WINSTA_WRITEATTRIBUTES | WINSTA_ACCESSGLOBALATOMS | \
	WINSTA_EXITWINDOWS | WINSTA_ENUMERATE | WINSTA_READSCREEN | \
	STANDARD_RIGHTS_REQUIRED)

	#define GENERIC_ACCESS (GENERIC_READ | GENERIC_WRITE | \
	GENERIC_EXECUTE | GENERIC_ALL)

	//接收测评信息数据头
	struct MESSAGE{
		//测评类型：M_COMPILE/M_TEST
		unsigned int Mid;
		//DATA数组实际长度
		unsigned int Datasize;
		unsigned char DATA[1];
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
	static Out* work(MESSAGE* In);

	//获得指定size的数据
	static void readdata(void* buf,int sz);

	//取得测评信息
	static MESSAGE* getmessage();

	//发送指定size的数据
	static void sendback(void *Data,int sz);

	
	//初始化socket
	static void init();

	static int UnSafeInit(WCHAR *Username,WCHAR *Passwrod);

	static int RemoveAceBySid(HANDLE hObj,PSID pSid);

	static void ReportMessage(string msg,DWORD E = 0);

	static int LoadConfig();

	static HANDLE Report;

	static SOCKET sockSvr,sockConn;

	static DWORD port;

	static LONGLONG SecurityCode;

	static BOOL Authority();
	//主循环
	static void main();
};
