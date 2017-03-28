#ifndef _MT4ConnectionDll_Common_h_
#define _MT4ConnectionDll_Common_h_

#define DllImport extern "C" __declspec(dllimport)
#define DllExport extern "C" __declspec(dllexport)

#ifdef flagDLL
	#define DLLIMPORT __declspec(dllexport)
#else
	#define DLLIMPORT __declspec(dllimport)
#endif


#include <Core/Core.h>
using namespace Upp;


#if defined(flagDEBUG)
	#define TLOG(x) {String s; s << x; Log(Format("%", GetSysTime()) + ": " + s);}
#else
	#define TLOG(x)
#endif

void Log(String s);
	

void Clean();
void SubClean();
String LoadCString(const char* c);
char* StoreCString(String txt);


#ifdef CPU_LITTLE_ENDIAN
inline int StrPut(TcpSocket& out, void* data, int count) {
	void* mem = MemoryAlloc(count);
	byte* buf = (byte*)data;
	byte* tmp = (byte*)mem;
	buf += count-1;
	for(int i = 0; i < count; i++) {
		*tmp = *buf;
		buf--; tmp++;
	}
	int res = out.Put(mem, count);
	MemoryFree(mem);
	return res;
}
inline int StrGet(TcpSocket& in, void* data, int count) {
	void* mem = MemoryAlloc(count);
	byte* buf = (byte*)data;
	byte* tmp = (byte*)mem;
	int res = in.Get(mem, count);
	buf += res-1;
	for(int i = 0; i < res; i++) {
		*buf = *tmp;
		buf--; tmp++;
	}
	MemoryFree(mem);
	return res;
}
#else
inline int StrPut(TcpSocket& out, void* data, int count) {return out.Put(data, count);}
inline int StrGet(TcpSocket& in, void* data, int count) {return in.Get(data, count);}
#endif




#endif
