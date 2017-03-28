#ifndef _MT4ConnectionDll_MT4ConnectionDll_h
#define _MT4ConnectionDll_MT4ConnectionDll_h

#include "Common.h"

DllExport int ConnectionInit(int port, const char* file_path_cstr);
DllExport void ConnectionDeinit();

DllExport char* GetLine();
DllExport void PutLine(char* c);

#endif


