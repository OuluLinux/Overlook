#ifndef _CommandBridge_CommandPrompt_h_
#define _CommandBridge_CommandPrompt_h_

#include <CodeEditor/CodeEditor.h>
#include <Esc/Esc.h>
using namespace Upp;

class Console;

class CommandPrompt : public CodeEditor {
	
	ArrayMap<String, EscValue> vars;
	Console* cons;

public:
	CommandPrompt(Console* cons);
	void    Execute();

	virtual bool Key(dword key, int count);
	virtual void LeftDouble(Point p, dword flags);

};

ArrayMap<String, EscValue>& UscGlobal();

#endif
