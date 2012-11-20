#include "stdafx.h"
#include "Install.h"

BOOL Install::Install(){
	string usr,pwd;
	LONGLONG Security;
	unsigned int port;
	double tm_ratio,tm_offset;
	ifstream cfg("Judge.ini");
	if(!(cfg>>usr>>pwd>>port>>tm_ratio>>tm_offset>>Security)){
		MessageBoxW(0,L"Can not read Judge.ini",L"Error.",MB_OK);
		return FALSE;
	}
	DWORD size  = GetCurrentDirectory(0,NULL);
	WCHAR *buf = new WCHAR[size+10];
	memset(buf,0,sizeof(WCHAR)*(size+10));
	if(0 == GetCurrentDirectoryW(size,buf))
		return FALSE;
	if(buf[wcslen(buf)-1]=='\\')
		wcscat_s(buf,size+10,L"Temp");
	else
		wcscat_s(buf,size+10,L"\\Temp");
	HKEY hKey;
	if(ERROR_SUCCESS != RegOpenKeyW(HKEY_LOCAL_MACHINE,L"SYSTEM\\CurrentControlSet\\Services\\v-Judge_Kernel",&hKey))
		return FALSE;
	if(ERROR_SUCCESS != RegSetValueExA(hKey,"Port",0,REG_DWORD,(BYTE*)&port,sizeof(port)))
		return FALSE;
	if(ERROR_SUCCESS != RegSetValueExA(hKey,"Username",0,REG_SZ,(BYTE*)usr.c_str(),usr.length()+1))
		return FALSE;
	if(ERROR_SUCCESS != RegSetValueExA(hKey,"Password",0,REG_SZ,(BYTE*)pwd.c_str(),pwd.length()+1))
		return FALSE;
	if(ERROR_SUCCESS != RegSetValueExW(hKey,L"Temp",0,REG_SZ,(BYTE*)buf,(wcslen(buf)+1)*sizeof(WCHAR)))
		return FALSE;
	if(ERROR_SUCCESS != RegSetValueExW(hKey,L"tm_Ratio",0,REG_BINARY,(BYTE*)&tm_ratio,sizeof(tm_ratio)))
		return FALSE;
	if(ERROR_SUCCESS != RegSetValueExW(hKey,L"tm_Offset",0,REG_BINARY,(BYTE*)&tm_offset,sizeof(tm_offset)))
		return FALSE;
	if(ERROR_SUCCESS != RegSetValueExW(hKey,L"SecurityCode",0,REG_BINARY,(BYTE*)&Security,sizeof(Security)))
		return FALSE;
	delete[] buf;
	if(!SetDenied(usr))
		return FALSE;
	RegCloseKey(hKey);
	return TRUE;
}

BOOL Install::SetDenied(string UsernameA){
DWORD dwRet; 
    LPWSTR SamName = L"MACHINE\\SYSTEM\\CurrentControlSet\\services\\v-Judge_Kernel";
    PSECURITY_DESCRIPTOR pSD = NULL; 
    PACL pOldDacl = NULL; 
    PACL pNewDacl = NULL; 
    EXPLICIT_ACCESSW ea; 
    HKEY hKey = NULL; 

	WCHAR* Username = GetWideChar(UsernameA.c_str());

    // 获取SAM主键的DACL 
    dwRet = GetNamedSecurityInfoW(SamName, SE_REGISTRY_KEY, DACL_SECURITY_INFORMATION, 
                NULL, NULL, &pOldDacl, NULL, &pSD); 
    if (dwRet != ERROR_SUCCESS) 
    { 
		return FALSE;
    } 

    // 创建一个ACE，允许Everyone完全控制对象，并允许子对象继承此权限 
    ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS)); 
    BuildExplicitAccessWithNameW(&ea, Username, KEY_ALL_ACCESS , DENY_ACCESS, 
        SUB_CONTAINERS_AND_OBJECTS_INHERIT); 

    // 将新的ACE加入DACL 
    dwRet = SetEntriesInAclW(1, &ea, pOldDacl, &pNewDacl); 
    if (dwRet != ERROR_SUCCESS) 
    { 
		return FALSE;
    } 

    // 更新SAM主键的DACL 
    dwRet = SetNamedSecurityInfoW(SamName, SE_REGISTRY_KEY, DACL_SECURITY_INFORMATION, 
                NULL, NULL, pNewDacl, NULL); 
    if (dwRet != ERROR_SUCCESS) 
    { 
		return FALSE;
    } 
	return TRUE;
}