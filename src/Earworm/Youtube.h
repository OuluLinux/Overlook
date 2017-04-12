#ifndef _Earworm_YoutubeDL_h_
#define _Earworm_YoutubeDL_h_

#include <Core/Core.h>
using namespace Upp;

class SearchResult : public Moveable<SearchResult> {
	String title, addr;
public:
	SearchResult() {}
	SearchResult(const SearchResult& c) {title = c.title; addr = c.addr;}
	
	void SetTitle(String s) {title = s;}
	void SetAddress(String s) {addr = s;}
	
	String GetTitle() {return title;}
	String GetAddress() {return addr;}
	
};

class YouTubeSearch  {
	Vector<SearchResult> list;
	bool check_avail;
	bool max_avail_checks;
	
protected:
	bool IsVideoAvailable(String url);
	
public:
	YouTubeSearch();
	
	void SetAvailabilityCheck(bool b=true, int max_checks=-1) {check_avail = b; max_avail_checks = max_checks;}
	void Search(String string);
	
	int GetCount() {return list.GetCount();}
	SearchResult operator[] (int i) {return list[i];}
	
};

class YouTubeDL {
	Array<Thread> thrds;
	int downloading;
public:
	typedef YouTubeDL CLASSNAME;
	YouTubeDL();
	
	int GetDownloading() const {return downloading;}
	
	void Download(String path, String url, Callback1<String> WhenReadyToPlay);
	void DoDownload(String path, String url, Callback1<String> WhenReadyToPlay);
	
};

#endif
