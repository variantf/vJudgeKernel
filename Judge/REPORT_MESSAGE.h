#pragma once
#include "stdafx.h"
class REPORT_MESSAGE
{
private:
	HANDLE hReport;
public:
	REPORT_MESSAGE(void);
	~REPORT_MESSAGE(void);
	void Report(string msg,DWORD E = 0);
};

