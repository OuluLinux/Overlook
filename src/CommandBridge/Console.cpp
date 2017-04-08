#include "CommandBridge.h"

Console::Console() : cmd(this) {
	Add(cmd.SizePos());
	Add(wordapp.SizePos());
	
	SetView(VIEW_CMD);
	
	wordapp.WhenTitle << Proxy(WhenTitle);
	
	AddProgram("ls",	THISBACK(ListFiles));
	AddProgram("cd",	THISBACK(ChangeDirectory));
	AddProgram("mkdir",	THISBACK(CreateDirectory));
	AddProgram("rm",	THISBACK(RemoveFile));
	AddProgram("cat",	THISBACK(ShowFile));
	AddProgram("edit",	THISBACK(EditFile));
	AddProgram("get",	THISBACK(DownloadFile));
	AddProgram("ftpd",	THISBACK(StartFtpServer));
	
	cwd = GetHomeDirectory();
}

void Console::SetView(int i) {
	cmd.Hide();
	wordapp.Hide();
	switch(i) {
		case VIEW_CMD:		cmd.Show();			break;
		case VIEW_WORD:		wordapp.Show();		break;
	}
	WhenViewChange();
	WhenTitle();
}

void Console::AddProgram(String cmd, Callback1<String> cb) {
	commands.Add(cmd, cb);
}

void Console::ConsoleMenu(Bar& bar) {
	if (VIEW_WORD)
		wordapp.FileBar(bar);
	
}

String Console::GetMenuTitle() {
	if (VIEW_WORD) return "Word";
	return "";
}

String Console::GetTitle() {
	if (VIEW_WORD) return wordapp.GetTitle();
	return "Console";
}

String Console::Command(const String& cmd) {
	LOG("Console::Command: " << cmd);
	
	String ret;
	
	out.Clear();
	err.Clear();
	
	for(int i = 0; i < commands.GetCount(); i++) {
		const String& c = commands.GetKey(i);
		int c_count = c.GetCount();
		if (cmd.Left(c_count) == c &&
			(cmd == c || cmd.Mid(c_count, 1) == " ")) {
			commands[i](TrimBoth(cmd.Mid(c_count+1)));
			
			// Leave out last \n
			if (!out.IsEmpty() && *(out.End()-1) == '\n')
				return out.Left(out.GetCount()-1);
			else
				return out;
		}
	}
	
	return ret;
}

void Console::ListFiles(String arg) {
	
	// TODO: use arg
	
	// Calculate max line length
	Font fnt = cmd.GetFont();
	int width = cmd.GetSize().cx;
	int len4 = GetTextSize("abcd", fnt).cx;
	int max_chrs = width * 4 / len4;
	int max_fname = 30; // Max chars to show from filename
	int cols = max(1, max_chrs / max_fname);
	
	PutLine("Listing all files in " + cwd);
	FindFile ff(AppendFileName(cwd, "*"));
	bool first = true;
	String line;
	int col = 0;
	while(ff) {
		String fname = ff.GetName();
		if (fname.GetCharCount() > max_fname - 1)
			fname = fname.Left(max_fname - 1) + " ";
		else
			fname.Cat(' ', max_fname - fname.GetCharCount());
		line += fname;
		col++;
		if (col == cols) {
			PutLine(line);
			col = 0;
			line = "";
		}
		ff.Next();
	}
	if (line != "")
		PutLine(line);
}

void Console::ChangeDirectory(String arg) {
	
}

void Console::CreateDirectory(String arg) {
	
}

void Console::RemoveFile(String arg) {
	
}

void Console::ShowFile(String arg) {
	
}

void Console::EditFile(String arg) {
	
	
	SetView(VIEW_WORD);
}

void Console::DownloadFile(String arg) {
	
}

void Console::StartFtpServer(String arg) {
	ftpd.Clear();
	ftpd.Create();
	ftpd->Start();
}
