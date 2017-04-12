#include "Earworm.h"


YouTubeDL::YouTubeDL() {
	downloading = 0;
}


void YouTubeDL::Download(String path, String url, Callback1<String> WhenReadyToPlay) {
	downloading++;
	thrds.Add().Run(THISBACK3(DoDownload, path, url, WhenReadyToPlay));
}

void YouTubeDL::DoDownload(String path, String url, Callback1<String> WhenReadyToPlay) {
	
	int list = url.Find("&list");
	if (list != -1) {
		url = url.Left(list);
	}
	
#ifdef flagWIN32
	String cmd = ConfigFile("youtube-dl.exe") + " --no-part --proxy \"http://192.168.0.169:3128\" -o " + file + " \"" + url + "\"";
#elif defined flagPOSIX
	//String cmd = "/home/sblo/bin/youtube-dl -f 171 --no-part --proxy \"http://127.0.0.1:3128\" -o \"" + file + "\" \"" + url + "\"";
	String cmd =
		AppendFileName(GetHomeDirFile("bin"), "youtube-dl") +
		" -f 171 --no-part -o \"" + path + "\" \"" + url + "\"";
#endif
	LOG(cmd);
	Cout() << cmd << "\n";
	
	LocalProcess lp;
	lp.Start(cmd);
	
	Sleep(100);
	
	if (FileExists(path)) {
		Cout() << "File exists already\n";
		WhenReadyToPlay(path);
		downloading--;
		return;
	}
	
	String out;
	bool started = false;
	TimeStop ts;
	while (lp.IsRunning()) {
		String s;
		if (lp.Read(s) && s.GetCount()) {
			out += s;
			LOG(s);
			Cout() << s;
			// Don't try to break earlier, because youtube-dl may download audio and video in separate, and join them at the end
		}
	}
	Cout() << "LocalProcess finished\n";
	
	if (!started && FileExists(path)) {
		started = true;
		WhenReadyToPlay(path);
		Cout() << "ready to play\n";
	} else {
		Cout() << "not starting\n";
		FindFile ff;
		String search_str = path + "*";
		search_str.Replace(".mp4", "");
		ff.Search(search_str);
		if(!FileExists(path))
			Cout() << "File does not exists! " << path << "\n";
		do {
			path = ff.GetPath();
			Cout() << "path: " << path << "\n";
			if (path == ".." || path == ".") continue;
			if (FileExists(path)) {
				Cout() << "Found file: " << path << "\n";
				WhenReadyToPlay(path);
				break;
			}
		} while (ff.Next());
	}
	downloading--;
}
