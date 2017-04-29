#include "DataCore.h"
#include <plugin/tidy/tidy.h>

namespace DataCore {

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




#define COPY(x) x = e. x
Event& Event::operator = (const Event& e) {
	
	COPY(id);
	COPY(title);
	COPY(timestamp);
	COPY(unit);
	COPY(currency);
	COPY(forecast);
	COPY(previous);
	COPY(actual);
	COPY(market);
	COPY(impact);
	COPY(direction);
	
	return *this;
}
#undef COPY



#undef PRINT
#define PRINT(x) out << " " #x "=" << x
String Event::ToInlineString() const {
	String out;
	
	PRINT(id);
	PRINT(timestamp);
	PRINT(title);
	PRINT(unit);
	PRINT(currency);
	PRINT(forecast);
	PRINT(previous);
	PRINT(actual);
	PRINT(market);
	PRINT(impact);
	PRINT(direction);
	
	return out;
}
#undef PRINT

void Event::Serialize(Stream& s) {
	s % id % timestamp % impact % direction % title % unit % currency % forecast % previous % actual % market;
}

int Event::GetSignalDirection() {
	if (direction != 3 && previous.GetCount() && actual.GetCount()) {
		double prev = StrDbl(previous);
		double act = StrDbl(actual);
		double diff = act - prev;
		if (diff > 0) {
			if (direction == 1)		return 1;
			else					return -1;
		}
		else {
			if (direction == 1)		return -1;
			else					return 1;
		}
	} else {
		return 0;
	}
}








void BasicHeaders(HttpRequest& h) {
	h.ClearHeaders();
	h.KeepAlive(1);
	h.UserAgent("Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko");
	h.Header("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
	h.Header("Accept-Language", "en-US,en;q=0.5");
	h.Header("Accept-Encoding", "gzip, deflate");
	//h.Header("DNT", "1");
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

bool FindNode(String value, const XmlNode& n, XmlNode*& subnode) {
	for(int i = 0; i < n.GetCount(); i++ ) {
		if (n[i].Attr("class") == value) {
			if (n[i].GetCount() == 0) return 0;
			const XmlNode* t = &n[i];
			subnode = (XmlNode*)t;
			return 1;
		}
	}
	return 0;
}









EventManager::EventManager() {
	lastupdate = 0;
	
	
}
/*
void EventManager::SetArguments(const VectorMap<String, Value>& args) {
	
}


void EventManager::Start() {
	
	UpdateHistory();
}
*/
void EventManager::Init() {
	ASSERTREF(!thrd.IsOpen());
	
	LoadThis();
	if (events.GetCount() == 0) {
		UpdateHistory();
	}
	
	thrd.Run(THISBACK(Updater));
}


void EventManager::StoreThis() {
	StoreToFile(*this, ConfigFile("events.bin"));
}

void EventManager::LoadThis() {
	LoadFromFile(*this, ConfigFile("events.bin"));
}

void EventManager::Deinit() {
	thrd.Wait();
}

void EventManager::Updater() {
	int interval = 60*10;
	Time now = GetUtcTime();
	int last_week_update = now.hour;
	UpdateLastWeek();
	UpdateNextWeek();
	
	while (!thrd.IsShutdownThreads()) {
		now = GetUtcTime();
		Time next = now + interval;
		
		if (last_week_update != now.hour) {
			last_week_update = now.hour;
			UpdateLastWeek();
			UpdateNextWeek();
			StoreThis();
		} else {
			Refresh();
		}
		
		while (next > GetUtcTime() && !thrd.IsShutdownThreads()) {
			Sleep(1000);
		}
	}
}

void EventManager::Refresh() {
	SetTimeNow();
	Update("", true);
}


	





int EventManager::GetCount() {
	return events.GetCount();
}

Event& EventManager::GetEvent(int i) {
	return events[i];
}

void EventManager::SetEventActualDiff(int i, double diff) {
	Event& ce = GetEvent(i);
	
	String strdbl = DblStr(100+diff);
	ce.previous = "100";
	ce.forecast = "100";
	ce.actual = strdbl;
	ce.direction = 1;
	
	StoreThis();
}

void EventManager::ParseEvent(const XmlNode& n, Event& event) {
	event.id = StrInt(n.Attr("data-eventid"));
	
	#define WRAP(x) "calendar__cell calendar__" x " " x
	
	XmlNode* sn;
	if (FindNode(WRAP("date"), n, sn)) {
		String d = (*sn)[0][1][0].GetText();
		//LOG("date: " << d);
		
		int month;
		if (d.Left(3) == "Jan") month = 1;
		else if (d.Left(3) == "Feb") month = 2;
		else if (d.Left(3) == "Mar") month = 3;
		else if (d.Left(3) == "Apr") month = 4;
		else if (d.Left(3) == "May") month = 5;
		else if (d.Left(3) == "Jun") month = 6;
		else if (d.Left(3) == "Jul") month = 7;
		else if (d.Left(3) == "Aug") month = 8;
		else if (d.Left(3) == "Sep") month = 9;
		else if (d.Left(3) == "Oct") month = 10;
		else if (d.Left(3) == "Nov") month = 11;
		else if (d.Left(3) == "Dec") month = 12;
		else {
			errorcode = 3;
			LOG("ForexFactory::ParseEvent month parse error: " << d);
			return;
		}
		
		int day = StrInt(d.Mid(4));
		
		int y;
		if (month <= thismonth) y = thisyear;
		else if (month > thismonth) y = thisyear - 1;
		prevdate = Date(y, month, day);
		//LOG("	" << Format("%", prevdate));
	}
	if (FindNode(WRAP("time"), n, sn)) {
		String t;
		if (sn->GetCount() == 1)
			t = (*sn)[0].GetText();
		else
			t = (*sn)[1][0].GetText();
		//LOG("time: " << t);
		if (t != "Tentative" && t != "All Day") {
			Vector<String> tv = Split(t, ":");
			if (tv.GetCount() == 2) {
				prevtime = Timestamp( Time(prevdate.year, prevdate.month, prevdate.day, StrInt(tv[0]), StrInt(tv[1])) );
				//LOG("	" << Format("%", prevtime));
			}
		}
	}
	event.timestamp = prevtime;
	
	if (FindNode(WRAP("currency"), n, sn)) {
		event.currency = (*sn)[0].GetText();
		//LOG("currency: " << event.currency);
	}
	if (FindNode(WRAP("event"), n, sn)) {
		event.title = (*sn)[0][0][0].GetText();
		event.title.Replace("\t", " ");
		event.title.Replace("\r", "");
		event.title.Replace("\n", " ");
		//LOG("event: " << event.title);
	}
	if (FindNode("calendar__cell calendar__impact impact calendar__impact calendar__impact--low", n, sn)) {
		event.impact = 1;
	}
	else if (FindNode("calendar__cell calendar__impact impact calendar__impact calendar__impact--medium", n, sn)) {
		event.impact = 2;
	}
	else if (FindNode("calendar__cell calendar__impact impact calendar__impact calendar__impact--high", n, sn)) {
		event.impact = 3;
	}
	/*if (FindNode(WRAP("impact"), n, sn)) {
		String c = (*sn)[0].Attr("class");
		if (c == "low") event.impact = 1;
		else if (c == "medium") event.impact = 2;
		else if (c == "high") event.impact = 3;
		else event.impact = 0;
		//if (c.GetCount() && IsDigit(c[0]))
		//	event.impact = c[0] - '0';
		//else
		//	event.impact = -1;
		//event.impact = StrInt(c);
		//LOG("impact: " << c);
	}*/
	if (FindNode(WRAP("actual"), n, sn)) {
		event.actual = (*sn)[0].GetText();
		if (event.actual == "span") event.actual = (*sn)[0][0].GetText();
		String& s = event.actual;
		for(int i = 0; i < s.GetCount(); i++ ) {
			char c = s[i];
			if (!IsDigit(c) && c != '-' && c != '.') {
				event.unit = s.Mid(i);
				s = s.Left(i);
				break;
			}
		}
		//LOG("actual: " << v);
	}
	if (FindNode(WRAP("forecast"), n, sn)) {
		event.forecast = (*sn)[0].GetText();
		if (event.forecast == "span") event.forecast = (*sn)[0][0].GetText();
		String& s = event.forecast;
		for(int i = 0; i < s.GetCount(); i++ ) {
			char c = s[i];
			if (!IsDigit(c) && c != '-' && c != '.') {
				s = s.Left(i);
				break;
			}
		}
		//LOG("forecast: " << v);
	}
	if (FindNode(WRAP("previous"), n, sn)) {
		event.previous = (*sn)[0].GetText();
		if (event.previous == "span") event.previous = (*sn)[0][0].GetText();
		String& s = event.previous;
		for(int i = 0; i < s.GetCount(); i++ ) {
			char c = s[i];
			if (!IsDigit(c) && c != '-' && c != '.') {
				s = s.Left(i);
				break;
			}
		}
		//LOG("previous: " << v);
	}
	
}



void EventManager::UpdateLastWeek() {
	Date now = GetUtcTime();
	now -= 1;
	while (DayOfWeek(now) != 0) now -= 1;
	String postfix = "?week=";
	switch (now.month) {
		case 1: postfix += "jan"; break;
		case 2: postfix += "feb"; break;
		case 3: postfix += "mar"; break;
		case 4: postfix += "apr"; break;
		case 5: postfix += "may"; break;
		case 6: postfix += "jun"; break;
		case 7: postfix += "jul"; break;
		case 8: postfix += "aug"; break;
		case 9: postfix += "sep"; break;
		case 10: postfix += "oct"; break;
		case 11: postfix += "nov"; break;
		case 12: postfix += "dec"; break;
	}
	postfix << (int)now.day << "." << (int)now.year;
	//LOG(postfix);
	SetTimeNow();
	Update(postfix, true);
}

void EventManager::UpdateNextWeek() {
	Date now = GetUtcTime();
	now += 1;
	while (DayOfWeek(now) != 0) now += 1;
	String postfix = "?week=";
	switch (now.month) {
		case 1: postfix += "jan"; break;
		case 2: postfix += "feb"; break;
		case 3: postfix += "mar"; break;
		case 4: postfix += "apr"; break;
		case 5: postfix += "may"; break;
		case 6: postfix += "jun"; break;
		case 7: postfix += "jul"; break;
		case 8: postfix += "aug"; break;
		case 9: postfix += "sep"; break;
		case 10: postfix += "oct"; break;
		case 11: postfix += "nov"; break;
		case 12: postfix += "dec"; break;
	}
	postfix << (int)now.day << "." << (int)now.year;
	SetTimeNow();
	Update(postfix, true);
}

void EventManager::Dump() {
	LOG("event-count: " << events.GetCount());
	for(int i = 0; i < events.GetCount(); i++) {
		LOG(i << ": " << events[i].ToInlineString());
	}
}

void EventManager::UpdateHistory() {
	TimeVector& tv = GetTimeVector();
	Date now = GetSysDate();
	Date begin(2007,1,1);
	if (events.GetCount())
		begin = TimeFromTimestamp(events[events.GetCount()-1].timestamp);
	
	// Dates are correct only about two weeks beforehand, or not always even then.
	now += 14;
	
	Date t(begin.year, begin.month, 1);
	int downloaded = 0;
	while (t < now) {
		
		String postfix = "?month=";
		switch (t.month) {
			case 1: postfix += "jan"; break;
			case 2: postfix += "feb"; break;
			case 3: postfix += "mar"; break;
			case 4: postfix += "apr"; break;
			case 5: postfix += "may"; break;
			case 6: postfix += "jun"; break;
			case 7: postfix += "jul"; break;
			case 8: postfix += "aug"; break;
			case 9: postfix += "sep"; break;
			case 10: postfix += "oct"; break;
			case 11: postfix += "nov"; break;
			case 12: postfix += "dec"; break;
		}
		postfix << "." << (int)t.year;
		
		thismonth = t.month;
		thisyear = t.year;
		
		if (Update(postfix)) {
			downloaded++;
			Sleep((10 + Random(3)) * 1000);
		}
		
		LOG("event-count: " << events.GetCount());
		
		int y = t.year;
		int m = t.month;
		m++;
		if (m == 13) {m = 1; y++;}
		t = Date(y, m, 1);
	}
	
	SetTimeNow();
	StoreThis();
}

void EventManager::SetTimeNow() {
	Time now = GetUtcTime();
	thisyear = now.year;
	thismonth = now.month;
}

#define LEAVE {lock.Leave(); return downloaded;}
int EventManager::Update(String postfix, bool force_update) {
	int downloaded = 0;
	
	addevents_count = 0;
	prevtime = 0;
	
	LOG("Calendar::Update " << postfix);
	lock.Enter();
	errorcode = 0;
	
	if (!postfix.GetCount()) {
		Time now = GetUtcTime();
		dword now_ts = Timestamp(now);
		if (now_ts >= lastupdate + 5*60)
			lastupdate = now_ts;
		else LEAVE;
	}
	
	// Realize cache directory
	String cache_dir = ConfigFile("cache");
	RealizeDirectory(cache_dir);
	String file_name = postfix;
	if (file_name.IsEmpty()) file_name = "default";
	file_name.Replace("?", "");
	file_name.Replace("=", "_");
	file_name.Replace(".", "_");
	String file_path = AppendFileName(cache_dir, file_name + ".html");
	
	String c;
	if (FileExists(file_path) && !force_update) {
		c = LoadFile(file_path);
	}
	else {
		HttpRequest h;
		h.Trace();
		BasicHeaders(h);
		
		/*String proxy_addr = GetGlobalConfigData("proxy_addr");
		String proxy_port = GetGlobalConfigData("proxy_port");
		if (!proxy_addr.IsEmpty() && !proxy_port.IsEmpty())
			h.Proxy(proxy_addr, StrInt(proxy_port));*/
		
		h.Url("http://www.forexfactory.com/calendar.php" + postfix);
		
		h.Cookie("fftimezoneoffset", "0");
		h.Cookie("ffdstonoff", "0");
		h.Cookie("fftimeformat", "1");
		h.Cookie("ffverifytimes", "1");
		h.Cookie("notice_bwa_nojs", "1");
		h.Cookie("ffadon", "0");
		h.Cookie("notice_bwa_mobile", "1");
		h.Cookie("notice_bwa_oldbrowser", "1");
		h.Cookie("fflastactivity", "0");
		h.Execute();
		
		c = h.GetContent();
		
		// Store cache file
		FileOut out(file_path);
		out << c;
		
		downloaded++;
	}
	
	c.Replace("/header", "/div");
	c.Replace("header", "div");
	c.Replace("/section", "/div");
	c.Replace("section", "div");
	c.Replace("/footer", "/div");
	c.Replace("footer", "div");
	c.Replace("&nbsp;","");
	c.Replace("></span> </td>", ">_</span> </td>");
	
	String filehtm	= ConfigFile("cal_temp.htm");
	String file		= ConfigFile("cal_temp.xml");
	FileOut out(filehtm);
	out << c;
	out.Close();
	
	Tidy(file, filehtm);
	
	
	FileIn in(file);
	String xml = in.Get(in.GetSize());
	in.Close();
	
	
	XmlNode xn = ParseXML(xml);
	
	const XmlNode& x = TryOpenLocation("0 1 3 0 3 1 10 0 0 0 1 0 1", xn, errorcode);
	if (errorcode) {
		LOG(XmlTreeString(xn));
		LEAVE;
	}
	
	Vector<Event> e;
	
	int count = x.GetCount();
	for(int i = 0; i < x.GetCount(); i++ ) {
		String eventid = x[i].Attr("data-eventid");
		if (!eventid.GetCount()) continue;
		//TLOG(eventid);
		ParseEvent(x[i], e.Add());
		if (errorcode) {
			e.Remove(e.GetCount()-1);
			errorcode = 0;//LEAVE;
		}
	}
	
	
	for(int j = 0; j < e.GetCount(); j++ ) {
		//Event& src = ce[j];
		SetEvent(e[j]);
	}
	
	Sort(events, Event());
	
	LEAVE
}
#undef LEAVE


















void EventManager::SetEvent(Event& ce) {
	
	if (!ce.timestamp || ce.title.IsEmpty()) return;
	
	Cout() << "SetEvent " << ce.title << "\n";
	
	
	for(int i = events.GetCount()-1; i >=  0; i-- ) {
		Event& e = events[i];
		
		if (e.timestamp + 60*60*24 < ce.timestamp)
			break;
		
		if (e.id == ce.id ) {
			if (e.actual != ce.actual) {
				e = ce;
			}
			return;
		}
	}

	events.Add(ce);
}

int EventManager::GetDirection(Event& ce) {
	Panic("Don't use this");
	
	LOG("EventManager::GetDirection " << ce.title);
	ASSERT(ce.id);
	String url = "http://www.forexfactory.com/flex.php?do=ajax&contentType=Content&flex=calendar_mainCal&details=" + IntStr(ce.id);
	HttpRequest http;
	http.UserAgent("Mozilla/5.0 (X11; Linux amd64; rv:41.0) Gecko/20100101 Firefox/41.0");
	http.Header("X-Requested-With", "XMLHttpRequest");
	String content = http.Url(url).Execute();
	
	int sigdir = 3;
	if (content.Find("Actual > Forecast = Good for currency") != -1)
		sigdir = 1;
	else if (content.Find("Actual < Forecast = Good for currency") != -1)
		sigdir = 2;
	ce.direction = sigdir;
	return sigdir;
}

bool EventManager::IsCalendarNeedingUpdate() {
	int now = TimestampNow();
	
	if (!events.GetCount())
		return 0;
	
	bool r = false;
	for(int i = events.GetCount()-1; i >=  0; i-- ) {
		Event& e = events[i];
		if (e.timestamp <= now &&
			e.actual.GetCount() == 0 &&
			e.previous.GetCount()) {
			r = true;
			break;
		}
	}
	
	return r;
}


}
