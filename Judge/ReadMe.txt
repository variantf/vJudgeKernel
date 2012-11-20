本Judge可用于OI/ICPC等计算机竞赛测评
程序启动以后请安装service。之后程序以服务身份运行。shell可以退出。
安装服务时需要在exe相同目录下面有Judge.ini
Judge.ini共有5行

//Judge.ini Sample:
RunUnSafe
Password
6000
1
1000


第一行为运行不安全代码的用户名
第二行为运行不安全代码的帐户密码
第三行是服务程序与其他程序通信的端口(采用socket通信)
第四行是等待程序运行时间系数 ratio
第五行是等待程序运行时间偏移 offset (ms)

判断程序TLE方法为 Wait For Process (TimeLimit*ratio+offset) 然后判断 usertime 是否大于 TimeLimit

通信格式：

	static const int M_COMPILE = 1;
	static const int M_TEST = 2;

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


每组数据均由结构体Message开始。
Data 为边长数组，为TEST_IN 结构体或者 COMPILE_IN 结构体

数据个变量含义均有注释。


程序如有错误。在事件查看器中会有所表示。请查看

欢迎通过 QQ:506938499/Email:sunjiayu_2006@126.com 联系我。