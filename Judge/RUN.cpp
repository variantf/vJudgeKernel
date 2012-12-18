#include "stdafx.h"
#include "RUN.h"


double RUN::tm_offset = 0,RUN::tm_ratio = 0;

Environment RUN::environment;

Token RUN::hToken;

DWORD RUN::GetAllProcessHandle(HANDLE hJob){
	PJOBOBJECT_BASIC_PROCESS_ID_LIST lst;
	DWORD len = (MAXIMUM_WAIT_OBJECTS-1)*sizeof(ULONG_PTR)+sizeof(JOBOBJECT_BASIC_PROCESS_ID_LIST);
	lst=(PJOBOBJECT_BASIC_PROCESS_ID_LIST)new BYTE[len];
	lst->NumberOfAssignedProcesses=MAXIMUM_WAIT_OBJECTS;
	if(!QueryInformationJobObject(hJob,JobObjectBasicProcessIdList,lst,len,&len)){
		throw runtime_error("QueryInfomationJobObject");
		assert(false);
	}
	if(!(lst->NumberOfAssignedProcesses==lst->NumberOfProcessIdsInList)){
		throw runtime_error("QueryInformationJobObject");
		assert(false);
	}
	//throw runtime_error("aiyouwocao");
	for(DWORD i=0;i<lst->NumberOfProcessIdsInList;i++){
		h[i]=OpenProcess(PROCESS_ALL_ACCESS,FALSE,lst->ProcessIdList[i]);
		if(!h[i]){
			throw runtime_error("OpenProcess");
			assert(false);
		}
	}
	DWORD Count=lst->NumberOfProcessIdsInList;
	delete[] lst;
	return Count;
}

VOID RUN::SetLimit(HANDLE hJob,DWORD Time,size_t Memory,int NumberOfProcess){
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION limit;
	memset(&limit,0,sizeof(limit));
	limit.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_JOB_MEMORY | /*JOB_OBJECT_LIMIT_JOB_TIME |JOB_OBJECT_LIMIT_PROCESS_TIME |*/
		JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION | JOB_OBJECT_LIMIT_ACTIVE_PROCESS |JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE|
		JOB_OBJECT_LIMIT_AFFINITY | JOB_OBJECT_LIMIT_PRIORITY_CLASS | JOB_OBJECT_LIMIT_PROCESS_MEMORY;
	limit.JobMemoryLimit = Memory;
	limit.ProcessMemoryLimit = Memory;
	limit.BasicLimitInformation.ActiveProcessLimit = NumberOfProcess;
	limit.BasicLimitInformation.PerJobUserTimeLimit.QuadPart = Time * 10000;
	limit.BasicLimitInformation.PerProcessUserTimeLimit.QuadPart = Time * 10000;
	limit.BasicLimitInformation.Affinity = CpuMask;
	limit.BasicLimitInformation.PriorityClass = NORMAL_PRIORITY_CLASS;
	if(!SetInformationJobObject(hJob,JobObjectExtendedLimitInformation,&limit,sizeof(limit))){
		throw runtime_error("SetInformationJobObject");
		assert(false);
	}

	JOBOBJECT_BASIC_UI_RESTRICTIONS UIlimit;
	UIlimit.UIRestrictionsClass = JOB_OBJECT_UILIMIT_EXITWINDOWS|JOB_OBJECT_UILIMIT_DESKTOP|
		JOB_OBJECT_UILIMIT_GLOBALATOMS|JOB_OBJECT_UILIMIT_DISPLAYSETTINGS|JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS|
		JOB_OBJECT_UILIMIT_WRITECLIPBOARD|JOB_OBJECT_UILIMIT_READCLIPBOARD;
	if(!SetInformationJobObject(hJob,JobObjectBasicUIRestrictions,&UIlimit,sizeof(UIlimit))){
		throw runtime_error("SetInformationJobObject");
		assert(false);
	}
}

VOID RUN::CreatePro(WCHAR* cmd,HANDLE hIn,HANDLE hOut,HANDLE hErr,BOOL AsUser,PPROCESS_INFORMATION pi){
	STARTUPINFOW si;
	memset(&si,0,sizeof(si));
	si.cb=sizeof(si);
	si.hStdError=hErr;
	si.hStdInput=hIn;
	si.hStdOutput=hOut;
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESTDHANDLES;	
	if(!CreateProcessAsUserW(
		hToken,            // client's access token
		NULL,              // file to execute
		cmd,               // command line
		NULL,              // pointer to process SECURITY_ATTRIBUTES
		NULL,              // pointer to thread SECURITY_ATTRIBUTES
		TRUE,              // handles are not inheritable
		CREATE_NO_WINDOW|
		CREATE_BREAKAWAY_FROM_JOB|
		CREATE_SUSPENDED|
		CREATE_UNICODE_ENVIRONMENT|
		DEBUG_ONLY_THIS_PROCESS, 
		// creation flags
		environment,       // pointer to new environment block 
		NULL,              // name of current directory 
		&si,               // pointer to STARTUPINFO structure
		pi                 // receives information about new process
		))
		throw runtime_error("CreateProcessAsUser");
}

BOOL RUN::Run_Binary(WCHAR* cmd,DWORD Time,size_t Memory,HANDLE hIn,HANDLE hOut,HANDLE hErr,DWORD NumberOfProcess,PLONGLONG rTime,PLONGLONG uMemory,BOOL AsUser,PDWORD ExitCode){

	//Create the Job
	MyHandle hJob=CreateJobObject(NULL,NULL);
	if(!hJob){
		throw runtime_error("CreateJobObject");
		assert(false);
	}

	SetLimit(hJob,Time,Memory,NumberOfProcess);

	PROCESS_INFORMATION pi;
	CreatePro(cmd,hIn,hOut,hErr,AsUser,&pi);

	MyHandle hProcess(pi.hProcess);
	MyHandle hThread(pi.hThread);
	if(!AssignProcessToJobObject(hJob,pi.hProcess)){
		throw runtime_error("AssignProcessToJobObject");
		assert(false);
	}

	if(-1==ResumeThread(pi.hThread)){
		throw runtime_error("ResumeThread");
		assert(false);
	}

	BOOL isTLE = DebugMainLoop(Time*tm_ratio+tm_offset,ExitCode,&pi,AsUser);
	DWORD nCount;
	while((nCount = GetAllProcessHandle(hJob))){
		TerminateJobObject(hJob,0);
		if(WAIT_FAILED == WaitForMultipleObjects(nCount,(HANDLE*)h,TRUE,INFINITE)){
			throw runtime_error("WaitForMultiObjects");
			assert(false);
		}
		for(DWORD i = 0;i < nCount ;i++)
			CloseHandle(h[i]);
	}
	if(0==*ExitCode){
		if(!GetExitCodeProcess(pi.hProcess,ExitCode))
			throw runtime_error("GetExitCodeProcess");
	}
	PROCESS_MEMORY_COUNTERS mInfo;
	mInfo.cb=sizeof(mInfo);
	if(!GetProcessMemoryInfo(pi.hProcess,&mInfo,sizeof(mInfo))){
		throw runtime_error("GetMemoryInfo");
		assert(false);
	}
	*uMemory = mInfo.PeakPagefileUsage;
	JOBOBJECT_BASIC_ACCOUNTING_INFORMATION info;
	if(!QueryInformationJobObject(hJob,JobObjectBasicAccountingInformation,&info,sizeof(info),0)){
		throw runtime_error("QueryInformation");
		assert(false);
	}
	*rTime = info.TotalUserTime.QuadPart;
	if(*rTime > (long long)Time * 10000)
		isTLE = TRUE;
	return isTLE;
}

RUN::RUN(){
	DWORD_PTR AffinityOwn,AffinityAll,Lowbit;
	if(!GetProcessAffinityMask(GetCurrentProcess(),&AffinityOwn,&AffinityAll))
		throw runtime_error("GetProcessAffinityMask");
	Lowbit = AffinityAll & (-(int)AffinityAll);
	if(Lowbit && AffinityAll - Lowbit){
		CpuMask = Lowbit;
		if(!SetProcessAffinityMask(GetCurrentProcess(),AffinityOwn & (~Lowbit)))
			throw runtime_error("SetProcessAffinityMask");
	}else{
		CpuMask = AffinityOwn;
	}
	memset(h,0,sizeof(h));
}

VOID RUN::dealException(EXCEPTION_RECORD *rec,LPDWORD Exitcode){
	switch(rec->ExceptionCode)
	{ 
	case EXCEPTION_ACCESS_VIOLATION: 
		OutputDebugString(L"EXCEPTION_ACCESS_VIOLATION");
		*Exitcode=0x10000001;
		break;
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		OutputDebugString(L"EXCEPTION_ARRAY_BOUNDS_EXCEEDED");
		*Exitcode=0x10000002;
		break;
	case EXCEPTION_FLT_DENORMAL_OPERAND:
		OutputDebugString(L"EXCEPTION_FLT_DENORMAL_OPERAND");
		*Exitcode=0x10000003;
		break;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		OutputDebugString(L"EXCEPTION_FLT_DIVIDE_BY_ZERO");
		*Exitcode=0x10000004;
		break;
	case EXCEPTION_FLT_INEXACT_RESULT:
		OutputDebugString(L"EXCEPTION_FLT_INEXACT_RESULT");
		*Exitcode=0x10000005;
		break;
	case EXCEPTION_FLT_INVALID_OPERATION:
		OutputDebugString(L"EXCEPTION_FLT_INVALID_OPERATION");
		*Exitcode=0x10000006;
		break;
	case EXCEPTION_FLT_OVERFLOW:
		OutputDebugString(L"EXCEPTION_FLT_OVERFLOW");
		*Exitcode=0x10000007;
		break;
	case EXCEPTION_FLT_STACK_CHECK:
		OutputDebugString(L"EXCEPTION_FLT_STACK_CHECK");
		*Exitcode=0x10000008;
		break;
	case EXCEPTION_FLT_UNDERFLOW:
		OutputDebugString(L"EXCEPTION_FLT_UNDERFLOW");
		*Exitcode=0x10000009;
		break;
	case EXCEPTION_ILLEGAL_INSTRUCTION:
		OutputDebugString(L"EXCEPTION_ILLEGAL_INSTRUCTION");
		*Exitcode=0x1000000A;
		break;
	case EXCEPTION_IN_PAGE_ERROR:
		OutputDebugString(L"EXCEPTION_IN_PAGE_ERROR");
		*Exitcode=0x1000000B;
		break;
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		OutputDebugString(L"EXCEPTION_INT_DIVIDE_BY_ZERO");
		*Exitcode=0x1000000C;
		break;
	case EXCEPTION_INT_OVERFLOW:
		OutputDebugString(L"EXCEPTION_INT_OVERFLOW");
		*Exitcode=0x1000000D;
		break;
	case EXCEPTION_INVALID_DISPOSITION:
		OutputDebugString(L"EXCEPTION_INVALID_DISPOSITION");
		*Exitcode=0x1000000E;
		break;
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		OutputDebugString(L"EXCEPTION_NONCONTINUABLE_EXCEPTION");
		*Exitcode=0x1000000F;
		break;
	case EXCEPTION_PRIV_INSTRUCTION:
		OutputDebugString(L"EXCEPTION_PRIV_INSTRUCTION");
		*Exitcode=0x10000010;
		break;
	case EXCEPTION_BREAKPOINT: 
		OutputDebugString(L"EXCEPTION_BREAKPOINT");
//		*Exitcode=0x10000011;
		break;
	case EXCEPTION_STACK_OVERFLOW:
		OutputDebugString(L"EXCEPTION_STACK_OVERFLOW");
		*Exitcode=0x10000012;
		break;
	case EXCEPTION_DATATYPE_MISALIGNMENT:  
		OutputDebugString(L"EXCEPTION_DATATYPE_MISALIGNMENT");
		*Exitcode=0x10000013;
		break;

	case EXCEPTION_SINGLE_STEP: 
		OutputDebugString(L"EXCEPTION_SINGLE_STEP");
		*Exitcode=0x10000014;
		break;

	case DBG_CONTROL_C: 
		OutputDebugString(L"DBG_CONTROL_C");
		*Exitcode=0x10000015;
		break;

	default:
		OutputDebugString(L"UNHANDLE");
//		*Exitcode=0x10000016;
		break;
	} 
	if(rec->ExceptionFlags==EXCEPTION_NONCONTINUABLE){
		OutputDebugString(L"   NonContinuable");
	}else{
		OutputDebugString(L"   Continuable");
	}
	for(unsigned i=0;i<rec->NumberParameters;i++)
		OutputDebugString((wstring(L"    ")+to_wstring(rec->ExceptionInformation[i])).c_str());
	OutputDebugString((wstring(L"    at address:")+to_wstring((LONGLONG)rec->ExceptionAddress)).c_str());
	OutputDebugString(L"\r\n");
	if(rec->ExceptionRecord && *Exitcode==0)
		dealException(rec->ExceptionRecord,Exitcode);

}

BOOL RUN::DebugMainLoop(DWORD Time,LPDWORD Exitcode,LPPROCESS_INFORMATION pi,BOOL AsUser){
	DWORD dwContinueStatus = DBG_CONTINUE;
	*Exitcode=0;
	DEBUG_EVENT DebugEv;
	BOOL ret=FALSE;
	while(1){
		FILETIME cre,t1,t2,t3,now;
		SYSTEMTIME snow;
		GetProcessTimes(pi->hProcess,&cre,&t1,&t2,&t3);
		GetSystemTime(&snow);
		SystemTimeToFileTime(&snow,&now);
		if((((LONGLONG)now.dwHighDateTime)<<32)+now.dwLowDateTime>(((LONGLONG)cre.dwHighDateTime)<<32)+cre.dwLowDateTime+Time*10000){
			TerminateProcess(pi->hProcess,0);
			ret=TRUE;
		}
		if(!WaitForDebugEvent(&DebugEv, 100)){
			if(121!=GetLastError())
				throw runtime_error("WaitForDebugEvent");
			continue;
		}
		switch (DebugEv.dwDebugEventCode) 
		{ 
		case EXCEPTION_DEBUG_EVENT: 
			dealException(&DebugEv.u.Exception.ExceptionRecord,Exitcode);
			if(*Exitcode)
				TerminateProcess(pi->hProcess,0);
			break;
		case CREATE_THREAD_DEBUG_EVENT:
			if(AsUser){
				if(pi->dwThreadId!=GetThreadId(DebugEv.u.CreateProcessInfo.hThread)){
					TerminateProcess(pi->hProcess,0);
					*Exitcode=0x20000001;
				}
			}
			break;
		case CREATE_PROCESS_DEBUG_EVENT: 
			CloseHandle(DebugEv.u.CreateProcessInfo.hFile);
			if(AsUser){
				if(pi->dwProcessId!=GetProcessId(DebugEv.u.CreateProcessInfo.hProcess)){
					TerminateProcess(pi->hProcess,0);
					*Exitcode=0x20000002;
				}
			}
			break;
		case EXIT_THREAD_DEBUG_EVENT: 
			break;
		case EXIT_PROCESS_DEBUG_EVENT: 
			break;
		case LOAD_DLL_DEBUG_EVENT: 
			CloseHandle(DebugEv.u.LoadDll.hFile);
			break;
		case UNLOAD_DLL_DEBUG_EVENT: 
			break;
		case OUTPUT_DEBUG_STRING_EVENT: 
			break;
		case RIP_EVENT:
			*Exitcode=0x20000004;
			break;
		}
		ContinueDebugEvent(DebugEv.dwProcessId, 
			DebugEv.dwThreadId, 
			dwContinueStatus);
		if(DebugEv.dwDebugEventCode==EXIT_PROCESS_DEBUG_EVENT){
			return ret;
		}
	}
}

Environment::Environment(){
	data.push_back(0);
	DWORD sz=GetEnvironmentVariable(L"PATH",NULL,0);
	WCHAR* path=new WCHAR[sz];
	if(0==GetEnvironmentVariable(L"PATH",path,sz))
		throw runtime_error("GetEnvironment");
	Add(L"PATH",path);
}

Environment::~Environment(){}

void Environment::Add(wstring name,wstring value){
	data.pop_back();
	wstring item=name+L"="+value;
	data.insert(data.end(),item.begin(),item.end());
	data.push_back(0);
	data.push_back(0);
}

Environment::operator LPVOID(){
	return &data[0];
}

Token::Token():_token(NULL){}

Token::~Token(){}

Token::operator HANDLE(){
	return _token;
}

BOOL Token::InitWithAuth(WCHAR* username,WCHAR* password){
	if (!LogonUserW(username,NULL,password,LOGON32_LOGON_INTERACTIVE,LOGON32_PROVIDER_DEFAULT,&_token))
		return false;
	return true;
}
