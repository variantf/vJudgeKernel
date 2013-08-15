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
		//�������ͣ�M_COMPILE/M_TEST
		unsigned int Mid;
		//DATA����ʵ�ʳ���
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

	//���յı�����Ϣ�ṹ
	struct COMPILE_IN{
		//��������ʱ��
		long long Time;
		//���������ڴ�
		long long Memory;
		
		unsigned int Codelen;
		//���������� {Source}{S.cpp} {Execute}{E.exe}
		char cmd[1];
	};

	//���ز����ṹ��Ϣ�ṹ
	struct Out{
		//�����ı���Ϣ����
		unsigned int MsgLen;
		//�������
		unsigned int ErrCode;
		//����ʵ������ʱ��
		long long rTime;
		//����ʵ��ʹ���ڴ�
		long long uMemory;
		//�ı���Ϣ
		char Msg[1];
	};

	//���ղ�����Ϣ�ṹ
	struct TEST_IN{
		//�������ļ���������char_data��ƫ��
		unsigned int offBin;
		//�����ļ���char_data��ƫ��
		unsigned int offIn;
		//���ƫ��
		unsigned int offOut;
		//�Ƚ�����ַ
		unsigned int offCmp;
		//����ʱ��
		long long Time;
		//�����ڴ�
		long long Memory;
		//�ı�����
		char char_data[1];
	};

	//������������
	RUNNER_OUT* work(map<string,wstring>* In);

	//���ָ��size������
	void readdata(void* buf,int sz);

	//ȡ�ò�����Ϣ
	map<string,wstring>* getmessage();

	//����ָ��size������
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

