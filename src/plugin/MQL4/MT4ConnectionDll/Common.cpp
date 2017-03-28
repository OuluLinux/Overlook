#include "Common.h"



FileOut out;
Array<char*> allocated_strings;

void Log(String s) {
	if (!out.IsOpen())
		out.Open("C:\\dll.log");
	if (!out.IsOpen())
		return;
	out << s << "\n";
	out.Flush();
}

String LoadCString(const char* c) {
	String out;
	while (*c) {
		out.Cat(*c);
		c++;
		c++;
	}
	return out;
}

char* StoreCString(String txt) {
	SubClean();
	int c1 = (txt.GetCount() + 1) * 2;
	char* n = (char*)MemoryAlloc(c1);
	allocated_strings.Add(n);
	for(int i = 0; i < txt.GetCount(); i++) {
		n[i*2] = txt[i];
		n[i*2+1] = 0;
	}
	n[c1-1] = 0;
	n[c1-2] = 0;
	return n;
}

void Clean() {
	TLOG("Full cleaning");
	for(int i = 0; i < allocated_strings.GetCount(); i++)
		MemoryFree(allocated_strings[i]);
	allocated_strings.Clear();
}

void SubClean() {
	if (allocated_strings.GetCount() > 12000) {
		while (allocated_strings.GetCount() != 10000) {
			MemoryFree(allocated_strings[0]);
			allocated_strings.Remove(0);
		}
	}
}









