#pragma once
#include "stdafx.h"
#include "RUN.h"
#include "MAIN.h"
#include "DELETE_TEMP.h"
#include "NETWORK.h"

namespace InitSrv{
	int UnSafeInit(WCHAR* Username,WCHAR* Password);
	int LoadConfig();
	int InitSrv();
};