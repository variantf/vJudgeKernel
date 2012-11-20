#pragma once
#include "REPORT_MESSAGE.h"
#include "MAIN.h"
class NETWORK
{
private:
	void init();
	SOCKET sockSvr;
	HANDLE hIOCP;
	DWORD RecvFlag;
public:
	static DWORD port;
	void Start();
	NETWORK(void);
	~NETWORK(void);
};

