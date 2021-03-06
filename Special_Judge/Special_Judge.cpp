// Special_Judge.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <io.h>
#include <fcntl.h>
#include <cassert>
//一些定义
const int ACCEPT = 0;
const int WRONG_ANSWER = 1;
const int Output_Limit_Execeeded = 6;
//fstd 标准输出 fout 选手输出 fin 标准输入
FILE *fstd,*fout,*fin;


int LastCharStd = -2,LastCharOut=-2;

//初始化文件指针。请完整复制
void InitFiles(TCHAR* argv[]){
	/*
#ifdef _WIN64
#define FORMAT L"%llu"
#else
#define FORMAT L"%u"
#endif
	HANDLE hOut,hTmp,hIn;
	swscanf_s(argv[1],FORMAT,&hOut);
	swscanf_s(argv[2],FORMAT,&hTmp);
	swscanf_s(argv[3],FORMAT,&hIn);
	int fdstd,fdout,fdin;
	if(-1==(fdstd = _open_osfhandle((intptr_t)hOut,_O_TEXT|_O_RDONLY))) 
		exit(-1);
	if(-1==(fdout = _open_osfhandle((intptr_t)hTmp,_O_TEXT|_O_RDONLY)))
		exit(-1);
	if(-1==(fdin = _open_osfhandle((intptr_t)hIn,_O_TEXT|_O_RDONLY)))
		exit(-1);
	fstd = _fdopen( fdstd,"r");
	fin = _fdopen(fdin,"r");
	fout = _fdopen(fdout ,"r");
	fseek(fstd,0,SEEK_SET);
	fseek(fout,0,SEEK_SET);
	fseek(fin,0,SEEK_SET);
	*/
}

//检查下一个字符
inline int Peek(FILE* f){
	if(f==fstd){
		if(LastCharStd == -2)
			LastCharStd=fgetc(f);
		return LastCharStd;
	}else{
		if(LastCharOut == -2)
			LastCharOut=fgetc(f);
		return LastCharOut;
	}
}

//取出下一个字符
inline void Pop(FILE* f){
	if(f==fstd){
		if(LastCharStd == -2)
			fgetc(f);
		else
			LastCharStd = -2;
	}else{
		if(LastCharOut == -2)
			fgetc(f);
		else
			LastCharOut = -2;
	}
}

//判断字符是否为空白
inline bool IsSpace(int ch){
	return ch>=0 && (ch<=32 || ch>=127);
}

//执行比较操作。请Spj编写者自行定义
//最终需要在标准输出中包含 {Score:xxx}
//表示本测试点获得得分为xxx
bool DoCompare(){
	int stdPosition=0,outPosition=0;
	bool stdInSpace=true,outInSpace=true;
	while(true){
		int stdC=Peek(fstd),outC=Peek(fout);
		if(stdC==EOF && outC==EOF){
			return true;
		}else if(stdC==EOF && IsSpace(outC)){
			outPosition++;
			Pop(fout);
		}else if(outC==EOF && IsSpace(stdC)){
			stdPosition++;
			Pop(fstd);
		}else if(IsSpace(stdC) && IsSpace(outC)){
			stdPosition++;
			outPosition++;
			stdInSpace=true;
			outInSpace=true;
			Pop(fstd);
			Pop(fout);
		}else if(IsSpace(stdC) && outInSpace){
			stdPosition++;
			Pop(fstd);
		}else if(IsSpace(outC) && stdInSpace){
			outPosition++;
			Pop(fout);
		}else if(stdC==outC){
			stdPosition++;
			outPosition++;
			stdInSpace=false;
			outInSpace=false;
			Pop(fstd);
			Pop(fout);
		}else{
			printf("答案文件的第%d字节",stdPosition+1);
			if(stdC==EOF){
				printf("<EOF>");
			}else{
				printf("0x%x",stdC);
			}
			printf("不能匹配输出文件的第%d字节",outPosition+1);
			if(outC==EOF){
				printf("<EOF>");
			}else{
				printf("0x%x",outC);
			}
			puts("");
			return false;
		}
	}
}

//照抄即可
int _tmain(int argc, char* argv[])
{
	if(argc!=4){
		printf("参数不足");
		return -1;
	}

	if(NULL==(fstd=fopen(argv[1],"r")))
		return -1;
	if(NULL==(fout=fopen(argv[2],"r")))
		return -1;
	if(NULL==(fin=fopen(argv[3],"r")))
		return -1;

	if(_filelength(_fileno(fout)) > max(10*1024,2* _filelength(_fileno(fstd))))
		return Output_Limit_Execeeded;
	if(DoCompare()){
		return ACCEPT;
	}else{
		return WRONG_ANSWER;
	}
}

