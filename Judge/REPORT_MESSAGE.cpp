#include "StdAfx.h"
#include "REPORT_MESSAGE.h"


REPORT_MESSAGE::REPORT_MESSAGE(void)
{
	hReport = RegisterEventSourceW(NULL,L"v-Judge_Kernel");
	if(NULL == hReport)	throw runtime_error("RegisterEventSource");
}


REPORT_MESSAGE::~REPORT_MESSAGE(void)
{
	DeregisterEventSource(hReport);
}

void REPORT_MESSAGE::Report(string msg,DWORD E){
		SYSTEMTIME t;
		GetLocalTime(&t);
		stringstream s;
		s<<t.wYear<<"-"<<t.wMonth<<"-"<<t.wDay<<" "<<t.wHour<<":"<<t.wMinute<<":"<<t.wSecond<<" "<<msg<<" "<<E<<endl;
		cout<<s;
		char *buf=new char[s.str().length()+1];
		memcpy_s(buf,s.str().length(),s.str().c_str(),s.str().length());
		buf[s.str().length()]=0;
		ReportEventA(hReport,E?EVENTLOG_ERROR_TYPE:EVENTLOG_SUCCESS,0,0,NULL,1,0,(LPCSTR*)&buf,NULL);
}