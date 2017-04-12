#include "Earworm.h"

#include <plugin/tidy/tidy.h>

extern "C" {
int Tidy(const char* file, const char* html);
}

void BasicHeaders(HttpRequest& h)
{
	h.ClearHeaders();
	h.KeepAlive(1);
	h.UserAgent("Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko");
	h.Header("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
	h.Header("Accept-Language", "en-US,en;q=0.5");
	h.Header("Accept-Encoding", "gzip, deflate");
	h.Header("DNT", "1");
}

const XmlNode& TryOpenLocation(String pos, const XmlNode& root, int& errorcode) {
	Vector<String> p = Split(pos, " ");
	const XmlNode* xn = &root;
	for(int i = 0; i < p.GetCount(); i++ ) {
		int j = StrInt(p[i]);
		if (j >= xn->GetCount()) {
			errorcode = 1;
			LOG("ERROR!!! xml TryOpenLocation fails! Maybe major change in web page");
			return root;
		}
		xn = &(*xn)[j];
	}
	return *xn;
}


String XmlTreeString(const XmlNode& node, int indent=0, String prev_addr="") {
	String out;
	for(int i = 0; i < indent; i++) {
		out.Cat(' ');
	}
	out << String(node.GetTag()) << " : " << String(node.GetText()) << " : " << prev_addr << " : ";
	for(int i = 0; i < node.GetAttrCount(); i++) {
		out += node.AttrId(i) + "=" + node.Attr(i) + ", ";
	}
	
	out += "\n";
	for(int i = 0; i < node.GetCount(); i++) {
		out += XmlTreeString(node[i], indent+1, prev_addr + " " + IntStr(i));
	}
	return out;
}


YouTubeSearch::YouTubeSearch() {
	check_avail = false;
	max_avail_checks = -1;
	
}

void YouTubeSearch::Search(String string) {
	list.Clear();
	
	HttpRequest h;
	HttpRequest::Trace();
	
	//h.Proxy("192.168.0.169:3128");
	//h.SSLProxy("192.168.0.169:3128");
	
	BasicHeaders(h);
	
	String url = "https://www.youtube.com/results?search_query=" + UrlEncode(string);
	
	h.SSL();
	h.Url(url);
	h.KeepAlive(false);
	
	String c = h.Execute();
	
	
	String filehtm 	= ConfigFile("yt_search_result.htm");
	String file 	= ConfigFile("yt_search_result.xml");
	FileOut out(filehtm);
	out << c;
	out.Close();
	
	Tidy(file, filehtm);
	
	FileIn in(file);
	String xml = in.Get(in.GetSize());
	in.Close();
	
	//LOG(xml);
	
	XmlNode xn;
	
	
	try {
		XmlParser p(xml);
		p.Relaxed();
		xn = ParseXML(p);
	}
	catch (XmlError e) {
		LOG(e);
	}
	LOG(XmlTreeString(xn));
	
	int errorcode = 0;
	
	const XmlNode& list = TryOpenLocation("0 1 1 4 0 4 0 0 0 0 0 1 1 0 1 0", xn, errorcode);
	
	if (!errorcode) {
		int count = list.GetCount();
		int check_count = 0;
		for(int i = 0; i < count; i++) {
			const XmlNode& sub = TryOpenLocation(IntStr(i) + " 0 0 1 0 0", list, errorcode);
			String href = sub.Attr("href");
			String title = sub.Attr("title");
			String url = "https://www.youtube.com" + href;
			
			if (href.GetCount() == 0)
				continue;
			
			if (check_avail && check_count != max_avail_checks) {
				check_count++;
				if (!IsVideoAvailable(url))
					continue;
			}
			
			SearchResult& sr = this->list.Add();
			sr.SetTitle(title);
			sr.SetAddress(url);
			LOG(i << ": " << title << ": " << href);
		}
	}
	
	LOG("end");
	//0 1 1 4 0 4 0 0 0 0 0 1 1 1 1 0 0 0 0 1 0 0
	//0 1 1 4 0 4 0 0 0 0 0 1 1 1 1 0 1 0 0 1 0 0
	//const XmlNode& x = xn[0][1][0][0][2][1][8][0][0][0][1][0][1];
	
	
}


bool YouTubeSearch::IsVideoAvailable(String url) {
	
	HttpRequest h;
	HttpRequest::Trace();
	
	//h.Proxy("192.168.0.169:3128");
	//h.SSLProxy("192.168.0.169:3128");
	
	BasicHeaders(h);
	
	h.SSL();
	h.Url(url);
	h.KeepAlive(false);
	
	String c = h.Execute();
	
	
	String filehtm 	= ConfigFile("yt_avail_check.htm");
	String file 	= ConfigFile("yt_avail_check.xml");
	FileOut out(filehtm);
	out << c;
	out.Close();
	
	Tidy(file, filehtm);
	
	FileIn in(file);
	String xml = in.Get(in.GetSize());
	in.Close();
	
	//LOG(xml);
	
	XmlNode xn;
	
	
	try {
		XmlParser p(xml);
		p.Relaxed();
		xn = ParseXML(p);
	}
	catch (XmlError e) {
		LOG(e);
	}
	LOG(XmlTreeString(xn));
	
	int errorcode = 0;
	const XmlNode& avail_node = TryOpenLocation("0 1 1 4 0 3 1 0 1 0", xn, errorcode);
	if (!errorcode) {
		if (avail_node.Attr("id") == "unavailable-message")
			return false;
	}
	
	return true;
}
