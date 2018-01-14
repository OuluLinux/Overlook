#include "Overlook.h"
#include <plugin/tidy/tidy.h>

#undef DLOG
#define DLOG(x)

namespace Overlook {

void BasicHeaders(HttpRequest& h) {
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
	for(int i = 0; i < p.GetCount(); i++) {
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
	for(int i = 0; i < n.GetCount(); i++) {
		if (n[i].Attr("class") == value) {
			if (n[i].GetCount() == 0) return 0;
			const XmlNode* t = &n[i];
			subnode = (XmlNode*)t;
			return 1;
		}
	}
	return 0;
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












#define COPY(x) x = e. x
CalEvent& CalEvent::operator = (const CalEvent& e) {
	
	COPY(id);
	COPY(title);
	COPY(timestamp);
	COPY(unit);
	COPY(currency);
	COPY(forecast);
	COPY(previous);
	COPY(actual);
	COPY(impact);
	COPY(direction);
	
	return *this;
}
#undef COPY



#undef PRINT
#define PRINT(x) out << " " #x "=" << x
String CalEvent::ToInlineString() const {
	String out;
	
	PRINT(id);
	PRINT(timestamp);
	PRINT(title);
	PRINT(unit);
	PRINT(currency);
	PRINT(forecast);
	PRINT(previous);
	PRINT(actual);
	PRINT(impact);
	PRINT(direction);
	
	return out;
}
#undef PRINT

String CalEvent::GetImpactString() const {
	switch (impact) {
		case 1:  return "Low";
		case 2:  return "Medium";
		case 3:  return "High";
		default: return "";
	}
}

void CalEvent::Serialize(Stream& s) {
	s % id % timestamp % impact % direction % title % unit % currency % forecast % previous % actual;
}

int CalEvent::GetSignalDirection() {
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








Calendar::Calendar() {
	
	
}

void Calendar::Init() {
	LoadThis();
	
	//if (events.GetCount() == 0) UpdateHistory();
}


void Calendar::StoreThis() {
	StoreToFile(*this, ConfigFile("events.bin"));
}

void Calendar::LoadThis() {
	LoadFromFile(*this, ConfigFile("events.bin"));
}

void Calendar::Data() {
	Time now = GetUtcTime();
	
	if (last_week_update.hour != now.hour) {
		last_week_update = now;
		UpdateLastWeek();
		UpdateNextWeek();
		StoreThis();
	}
}

void Calendar::Refresh() {
	SetTimeNow();
	Update("", true);
}

int Calendar::GetCount() {
	return events.GetCount();
}

const CalEvent& Calendar::GetEvent(int i) {
	return events[i];
}

void Calendar::SetEventActualDiff(int i, double diff) {
	CalEvent& ce = events[i];
	
	String strdbl = DblStr(100+diff);
	ce.previous = "100";
	ce.forecast = "100";
	ce.actual = strdbl;
	ce.direction = 1;
	
	StoreThis();
}

void Calendar::ParseEvent(const XmlNode& n, CalEvent& event) {
	
	event.id = StrInt(n.Attr("data-eventid"));
	
	#define WRAP(x) "calendar__cell calendar__" x " " x
	
	XmlNode* sn;
	if (FindNode(WRAP("date"), n, sn)) {
		String d = (*sn)[0][1][0].GetText();
		DLOG("date: " << d);
		
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
		DLOG("	" << Format("%", prevdate));
	}
	if (FindNode(WRAP("time"), n, sn)) {
		String t;
		if (sn->GetCount() == 1)
			t = (*sn)[0].GetText();
		else
			t = (*sn)[1][0].GetText();
		DLOG("time: " << t);
		if (t != "Tentative" && t != "All Day") {
			Vector<String> tv = Split(t, ":");
			if (tv.GetCount() == 2) {
				prevtime = Time(prevdate.year, prevdate.month, prevdate.day, StrInt(tv[0]), StrInt(tv[1]));
				DLOG("	" << Format("%", prevtime));
			}
		}
	}
	event.timestamp = prevtime;
	
	if (FindNode(WRAP("currency"), n, sn)) {
		event.currency = TrimBoth((*sn)[0].GetText());
		DLOG("currency: " << event.currency);
	}
	if (FindNode(WRAP("event"), n, sn)) {
		event.title = (*sn)[0][0][0].GetText();
		event.title.Replace("\t", " ");
		event.title.Replace("\r", "");
		event.title.Replace("\n", " ");
		DLOG("event: " << event.title);
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
	if (FindNode(WRAP("actual"), n, sn)) {
		event.actual = (*sn)[0].GetText();
		if (event.actual == "span") event.actual = (*sn)[0][0].GetText();
		String& s = event.actual;
		for(int i = 0; i < s.GetCount(); i++) {
			char c = s[i];
			if (!IsDigit(c) && c != '-' && c != '.') {
				event.unit = s.Mid(i);
				s = s.Left(i);
				break;
			}
		}
		DLOG("actual: " << v);
	}
	if (FindNode(WRAP("forecast"), n, sn)) {
		event.forecast = (*sn)[0].GetText();
		if (event.forecast == "span") event.forecast = (*sn)[0][0].GetText();
		String& s = event.forecast;
		for(int i = 0; i < s.GetCount(); i++) {
			char c = s[i];
			if (!IsDigit(c) && c != '-' && c != '.') {
				s = s.Left(i);
				break;
			}
		}
		DLOG("forecast: " << v);
	}
	if (FindNode(WRAP("previous"), n, sn)) {
		event.previous = (*sn)[0].GetText();
		if (event.previous == "span") event.previous = (*sn)[0][0].GetText();
		String& s = event.previous;
		for(int i = 0; i < s.GetCount(); i++) {
			char c = s[i];
			if (!IsDigit(c) && c != '-' && c != '.') {
				s = s.Left(i);
				break;
			}
		}
		DLOG("previous: " << v);
	}
	
}



void Calendar::UpdateLastWeek() {
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
	DLOG(postfix);
	SetTimeNow();
	Update(postfix, true);
}

void Calendar::UpdateNextWeek() {
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

void Calendar::Dump() {
	LOG("event-count: " << events.GetCount());
	for(int i = 0; i < events.GetCount(); i++) {
		LOG(i << ": " << events[i].ToInlineString());
	}
}

void Calendar::UpdateHistory() {
	Date now = GetSysDate();
	Date begin(2007,1,1);
	if (events.GetCount())
		begin = events[events.GetCount()-1].timestamp;
	
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

void Calendar::SetTimeNow() {
	Time now = GetUtcTime();
	thisyear = now.year;
	thismonth = now.month;
}

#define LEAVE {lock.Leave(); return downloaded;}
int Calendar::Update(String postfix, bool force_update) {
	int downloaded = 0;
	
	prevtime = Null;
	
	LOG("Calendar::Update " << postfix);
	lock.Enter();
	errorcode = 0;
	
	if (!postfix.GetCount()) {
		Time now = GetUtcTime();
		if (now >= last_update + 5*60)
			last_update = now;
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
		
		//String proxy_addr = GetGlobalConfigData("proxy_addr");
		//String proxy_port = GetGlobalConfigData("proxy_port");
		//if (!proxy_addr.IsEmpty() && !proxy_port.IsEmpty())
		//	h.Proxy(proxy_addr, StrInt(proxy_port));
		
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
		DLOG(XmlTreeString(xn));
		LEAVE;
	}
	
	Vector<CalEvent> events;
	
	int count = x.GetCount();
	for(int i = 0; i < x.GetCount(); i++) {
		String eventid = x[i].Attr("data-eventid");
		if (!eventid.GetCount()) continue;
		DLOG(eventid);
		ParseEvent(x[i], events.Add());
		if (errorcode) {
			events.Remove(events.GetCount()-1);
			errorcode = 0;
		}
	}
	
	for (auto& e : events) SetEvent(e);
	
	Sort(this->events, CalEvent());
	
	LEAVE
}
#undef LEAVE


















void Calendar::SetEvent(CalEvent& ce) {
	
	if (IsNull(ce.timestamp) || ce.title.IsEmpty()) return;
	
	Cout() << "SetEvent " << ce.title << "\n";
	
	
	for(int i = events.GetCount()-1; i >=  0; i-- ) {
		CalEvent& e = events[i];
		
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

bool Calendar::IsCalendarNeedingUpdate() {
	Time now = GetUtcTime();
	
	if (!events.GetCount())
		return 0;
	
	bool r = false;
	for(int i = events.GetCount()-1; i >=  0; i-- ) {
		CalEvent& e = events[i];
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
