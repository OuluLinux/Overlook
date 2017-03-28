#include "Common.h"





bool MT4Packet::Export(TcpSocket &s) {
	uint32 size;
	StringStream ss;
	void* mem;
	
	Store(*this, ss);
	ss.Seek(0);
	
	size = ss.GetSize();
	if (s.Put(&size, 4) != 4)
		return 1;
	
	mem = MemoryAlloc(size);
	ss.Get(mem, size);
	if (s.Put(mem, size) != size)  {
		MemoryFree(mem);
		return 1;
	}
	MemoryFree(mem);
	//ss.Seek(0); String sss = ss.Get(ss.GetSize()); LOG("Export: " << sss);
	return 0;
}

bool MT4Packet::Import(TcpSocket &s) {
	uint32 size, realsize;
	StringStream ss;
	void* mem;
	
	if (s.Get(&size, 4) != 4) 
		return 1;
	
	mem = MemoryAlloc(size);
	if (s.Get(mem, size) != size)  {
		MemoryFree(mem);
		return 1;
	}
	ss.Put(mem, size);
	MemoryFree(mem);
	ss.Seek(0);
	Load(*this, ss);
	//ss.Seek(0); String sss = ss.Get(ss.GetSize()); LOG("Import: " << sss);
	return 0;
}
