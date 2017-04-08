#include "CommandPrompt.h"
#include "CommandBridge.h"

ArrayMap<String, EscValue>& UscGlobal() {
	static ArrayMap<String, EscValue> global;
	return global;
}

CommandPrompt::CommandPrompt(Console* cons) : cons(cons)
{
	Highlight("calc");
	NoHorzScrollbar();
	HideBar();
}

int LfToSpaceFilter(int c)
{
	return c == '\n' ? ' ' : c;
}

void CommandPrompt::Execute()
{
	int li = GetLineCount() - 1;
	if(IsSelection()) {
		String s = GetSelection();
		if(s.GetLength() < 80) {
			SetCursor(GetLength());
			Paste(Filter(s, LfToSpaceFilter).ToWString());
		}
		return;
	}
	if(GetLine(GetCursor()) != li) {
		WString s = GetWLine(GetLine(GetCursor()));
		if(s[0] == '$') s = s.Mid(1);
		SetCursor(GetLength());
		Paste(s);
		return;
	}
	String txt;
	try {
		String s = TrimBoth(GetUtf8Line(li));
		if (s.IsEmpty()) return;
		
		// Do math if line starts with numbers
		char first = s[0];
		if (IsDigit(first) || first == '-' || first == '+') {
			ArrayMap<String, EscValue>& g = UscGlobal();
			for(int i = 0; i < g.GetCount(); i++)
				vars.GetAdd(g.GetKey(i)) = g[i];
			if(IsNull(s))
				return;
			EscValue v = Evaluatex(s, vars);
			txt = v.ToString(GetSize().cx / max(1, GetFont().Info()['x']), 4, true);
			vars.GetAdd("_") = v;
		}
		// Otherwise send command to command bridge
		else {
			txt = cons->Command(s);
		}
	}
	catch(CParser::Error e) {
		const char *x = strchr(e, ':');
		txt << "ERROR: " << (x ? x + 1 : ~e);
	}
	SetCursor(GetPos(li));
	Paste("$");
	SetCursor(GetLength());
	Paste("\n");
	if (!txt.IsEmpty()) {
		Paste(FromUtf8(txt));
		Paste("\n");
	}
}

void CommandPrompt::LeftDouble(Point p, dword flags)
{
	CodeEditor::LeftDouble(p, flags);
	if(IsSelection())
		Execute();
}

bool CommandPrompt::Key(dword key, int count)
{
	switch(key) {
	case K_ENTER:
		Execute();
		break;
	default:
		return CodeEditor::Key(key, count);
	}
	return true;
}

