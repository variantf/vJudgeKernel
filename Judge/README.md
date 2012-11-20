v-Judge_Kernel
==============

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

自定义测评的测试数据需要编写一个测评程序作为附加文件。此测评程序应能根据标准输入、标准输出和选手输出评判选手的得分，并给出相应意见。

评测机将会把标准输出、选手输出和标准输入三个文件的句柄通过命令行参数传递给测评程序。测评程序应通过标准输出与程序返回值给出评测结果。

测评程序的C++模版如下：
//必须的头文件
include <io.h>

include <fcntl.h>


//主函数返回值常量
const int ACCEPT = 0;

const int WRONG_ANSWER = 1;

//fstd 标准输出 fout 选手输出 fin 标准输入
FILE *fstd,*fout,*fin;

//初始化文件指针。


void InitFiles(wchar_t* argv[]){
  HANDLE hOut,hTmp,hIn;
  
  swscanf_s(argv[1],L"%u",&hOut);
	
  swscanf_s(argv[2],L"%u",&hTmp);
	
  swscanf_s(argv[3],L"%u",&hIn);
	
  fstd = _fdopen( _open_osfhandle((intptr_t)hOut,_O_TEXT),"r");
	
  fout = _fdopen( _open_osfhandle((intptr_t)(HANDLE)hTmp,_O_TEXT),"r");
	
  fin = _fdopen(_open_osfhandle((intptr_t)hIn,_O_TEXT),"r");
	
  fseek(fstd,0,SEEK_SET);
	
  fseek(fout,0,SEEK_SET);
	
  fseek(fin,0,SEEK_SET);
}

int _tmain(int argc, _TCHAR* argv[])

{
	
  if(argc!=4){
	
    throw "参数不足";
	
  }
	
  InitFiles(argv);
	
  /*
	
  * 现在已经做好了全部的初始化工作。

  * 其中，fstd是标准输出，fout是选手输出，fin是标准输入。

  * 下面应该进行测评工作。主函数返回ACCEPT代表完全正确。返回WRONG_ANSER代表有错误

  * 程序必须向标准输出中输出类似"{Score:xxx}"的字符串，代表本测试点得分为xxx。

  */
}


交互式的测试数据需要编写一个调用程序作为附加文件。此调用程序应能根据选手函数与测评资料评判选手的得分，并给出相应意见。

评测机将会把测评资料通过标准输入传递给调用程序。调用程序应通过标准输出与程序返回值给出评测结果。

注意：调用程序应为目标文件格式，而不是可执行文件格式。

调用程序的C++模版如下：

//主函数返回值常量

const int ACCEPT = 0;

const int WRONG_ANSWER = 1;


//此处声明选手提交程序所导出的函数

// extern "C" int APlusB(int,int);


int main(){

  /*

  * 现在标准输入中为测评资料。

  * 下面应该进行测评工作。主函数返回ACCEPT代表完全正确。返回WRONG_ANSER代表有错误

  * 程序必须向标准输出中输出类似"{Score:xxx}"的字符串，代表本测试点得分为xxx。

  */
}

提交答案的测试数据需要编写一个测评程序作为附加文件。此测评程序应能根据选手答案与测评资料评判选手的得分，并给出相应意见。

评测机将会把选手答案与测评资料通过标准输入传递给测评程序。测评程序应通过标准输出与程序返回值给出评测结果。

其中标准输入的数据结构如下：
struct Input{
  
  //选手提交答案长度
	
  uint32_t AnswerLen;
	
  //选手提交答案内容,长度为AnswerLen
	
  char Answer[];
	
  //测试资料。直到 EOF
	
  char TestData[];
};

测评程序的C++模版如下：

include <cstdio>

include <cstdint>

include <vector>

//主函数返回值常量

const int ACCEPT = 0;

const int WRONG_ANSWER = 1;


std::vector<char> answer;

  void ReadAnswer(){

  uint32_t answerSize;

  fread(&answerSize,sizeof(answerSize),1,stdin);



  answer.resize(answerSize);

  fread(&answer[0],answerSize,1,stdin);
}

int main(){

  ReadAnswer();
	
  /*
	
  * 此时已完成对标准输入的分离，现在answer中存储选手答案。

  * 而标准输入中剩余部分为测评资料。

  * 下面应该进行测评工作。主函数返回ACCEPT代表完全正确。返回WRONG_ANSER代表有错误

  * 程序必须向标准输出中输出类似"{Score:xxx}"的字符串，代表本测试点得分为xxx。

  */
}



程序如有错误。在事件查看器中会有所表示。请查看

欢迎通过 QQ:506938499/Email:sunjiayu_2006@126.com 联系我。