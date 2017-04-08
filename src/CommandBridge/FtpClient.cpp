#include "FtpClient.h"

/*GUI_APP_MAIN
{
	FtpBrowser browser;
	LoadFromFile(browser);
	browser.Run();
	StoreToFile(browser);
}*/

void FtpBrowser::Connect()
{
	Ftp::Trace(server.log);
	server.active ? browser.Active() : browser.Passive();
	browser.SSL(server.ftps);
	browser.User(~account.user, ~account.password);
	if(!browser.Connect(~server.host)) {
		Exclamation(browser.GetReply());
		return;
	}
	rootdir = browser.GetDir();
	if(rootdir.IsVoid()) {
		Exclamation(browser.GetReply());
		return;
	}
	Browse();
}

void FtpBrowser::Disconnect()
{
	browser.Disconnect();
	rootdir.Clear();
	workdir.Clear();
	list.Clear();
}

void FtpBrowser::Browse()
{
	Ftp::DirList dirlist;
	if(!browser.ListDir(Null, dirlist)) {
		Exclamation(browser.GetReply());
		return;
	}
	workdir = browser.GetDir();
	if(workdir.IsEmpty()) {
		Exclamation(browser.GetReply());
		return;
	}
	String host = server.host.GetData();
	String user = account.user.GetData();
	if(user.IsEmpty()) user = "anonymous";
	Title(Format(t_("Ftp Browser - %s@%s [%s]"),
					user,
					host, 
					workdir));
	list.Clear();
	if(workdir != rootdir) {
		list.Add(t_("Go to Parent Directory"),
		         CtrlImg::DirUp(),
		         StdFont(),
		         SColorText(),
		         true,
		         0,
		         Null,
		         Null,
		         Null,
		         Null,
		         0);
	}
	for(int i = 0; i < dirlist.GetCount(); i++) {
		Ftp::DirEntry& e = dirlist[i];
		if(e.IsSymLink())
			continue;  // Ignore symbolic links.
		list.Add(e.GetName(), 
		         e.IsFile() ? CtrlImg::File() : CtrlImg::Dir(),
		         StdFont(),
		         SColorText(),
		         e.IsDirectory(),
		         e.GetSize(),
		         e.GetLastModified(),
		         Null,
		         Null,
				 Null, 
				 RawPickToValue(pick(e)));
	}
}

void FtpBrowser::Action()
{
	int cursor = list.GetCursor();
	if(cursor == -1)
		return;
	const FileList::File& entry = list.Get(cursor);
	if(entry.isdir) {
		if(entry.data == 0) {
			if(!browser.DirUp()) {
				Exclamation(browser.GetReply());
				return;
			}
		}
		else {
			const Ftp::DirEntry& e = entry.data.To<Ftp::DirEntry>();
			if(!browser.SetDir(e.GetName())) {
				Exclamation(browser.GetReply());
				return;
			}
		}
		Browse();
	}
	else {
		const Ftp::DirEntry& e = entry.data.To<Ftp::DirEntry>();
		if(e.IsFile()) {
			String path = SelectDirectory();
			if(path.IsEmpty())
				return;
			path.Cat(path.EndsWith("/") ? "" : "/");
			Download(entry.name, path);
		}
	}
}

void FtpBrowser::Information(const FileList::File& entry)
{
	WithInformationLayout<TopWindow> infowin;
	CtrlLayout(infowin, t_("Information"));
	const Ftp::DirEntry& e = entry.data.To<Ftp::DirEntry>();
	String info;
	info << t_("Name:\t\t") << e.GetName() << "\r\n";
	info << t_("Type:\t\t");
	info << (e.IsDirectory() ? t_("Directory") : t_("File"));
	info << "\r\n";
	info << t_("Size:\t\t") << e.GetSize() << "\r\n";
	info << t_("Modified:\t\t") << e.GetLastModified() << "\r\n";
	if(e.GetStyle() == Ftp::DirEntry::UNIX) {
		info << t_("Owner:\t\t") << e.GetOwner() << "\r\n";
		info << t_("Group:\t\t") << e.GetGroup() << "\r\n";
		info << t_("Executable:\t\t") << e.IsExecutable() << "\r\n";
		info << t_("Readable:\t\t") << e.IsReadable() << "\r\n";
		info << t_("Writeable:\t\t") << e.IsWriteable() << "\r\n";		
	}
	infowin.info.SetData(info);
	infowin.Execute();
}

void FtpBrowser::Download(const String& file, const String& path)
{
	if(workdir.IsEmpty()) {
		Exclamation(t_("Path is not set"));
		return;
	}
	String remotefile;
	remotefile << workdir << (workdir.EndsWith("/") ? "" : "/") << file;
	FileOut localfile(path + file);
	progress.SetOwner(this);
	progress.Reset();
	progress.Create();
	int rc = FtpGet(remotefile, localfile, 
					~server.host, 21,
					~account.user, ~account.password,
					THISBACK(DownloadProgress), THISBACK(UpdateGui),
					Ftp::BINARY, server.active ? Ftp::ACTIVE : Ftp::PASSIVE,
					server.ftps);
	if(rc != 0) 
		Exclamation(t_("File download failed."));
	progress.Close();
}

bool FtpBrowser::DownloadProgress(int64 total, int64 done)
{
	progress.Set(done, total);
	if(progress.Canceled()) {
		if(PromptOKCancel(t_(t_("Do you really want to abort download?"))) == IDOK)
			return true;
		else
			progress.Reset();
	}
	else
		progress.SetText(Format(t_("Downloading %ld of %ld (bytes)"), done, total));
	return false;
}

void FtpBrowser::Settings()
{
	settings.Execute();
}

void FtpBrowser::About()
{
	PromptOK(t_("[A3 Simple Ftp Browser Example]&Using [*^www://upp.sf.net^ Ultimate`+`+] technology."));
}

void FtpBrowser::MainMenu(Bar& bar)
{
	bar.Add(t_("File"), THISBACK(FileMenu));
	bar.Add(t_("Help"), THISBACK(HelpMenu));
}

void FtpBrowser::FileMenu(Bar& bar)
{
	bar.Add(t_("Connect"), THISBACK(Connect));
	bar.Add(t_("Disconnect"), THISBACK(Disconnect));
	bar.Separator();
	bar.Add(t_("Settings"), THISBACK(Settings));
	bar.Separator();
	bar.Add(t_("Exit"), THISBACK(Close));
}

void FtpBrowser::HelpMenu(Bar& bar)
{
	bar.Add(t_("About..."), THISBACK(About));
}

void FtpBrowser::ContextMenu(Bar& bar)
{
	int cursor = list.GetCursor();
	if(cursor == -1)
		return;
	const FileList::File& entry = list.Get(cursor);
	if(entry.data == 0)
		return;
	bar.Add(t_("Information"), THISBACK1(Information, entry));
	bar.Separator();
	bar.Add(entry.isdir ? t_("Browse...") : t_("Download..."), THISBACK(Action));
}

void FtpBrowser::Serialize(Stream& s)
{
	int version = 1;
	s / version;
	s % server.host % server.active % server.log % server.ftps;
	s % account.user % account.password;
	SerializePlacement(s);
}

FtpBrowser::FtpBrowser()
{
	CtrlLayout(*this, t_("Ftp Browser"));
	Sizeable();
		
	list.Columns(2);
	list.WhenBar = THISBACK(ContextMenu);
	list.WhenLeftDouble = THISBACK(Action);

	AddFrame(mainmenu);	
	mainmenu.Set(THISBACK(MainMenu));

	browser.WhenWait = THISBACK(UpdateGui);

	settings.Add(account, t_("Account"));
	settings.Add(server, t_("Server"));
	settings.OKCancel();
	settings.Title(t_("Ftp Browser Settings"));
	account.password.Password();
}


FtpBrowser::~FtpBrowser()
{
}


