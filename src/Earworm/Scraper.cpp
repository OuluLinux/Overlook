#include "Earworm.h"
#include <plugin/bz2/bz2.h>
#include "Scraper.brc"

extern "C" {
int Tidy(const char* file, const char* html);
}

void BasicHeaders(HttpRequest& h);
const XmlNode& TryOpenLocation(String pos, const XmlNode& root, int& errorcode);
//String XmlTreeString(const XmlNode& node, int indent=0, String prev_addr="");




Scraper::Scraper() {
	LoadThis();
	
	if (singles.IsEmpty()) {
		LoadPreScraped();
	}

	if (singles.IsEmpty()) {
		Init();
		Reload();
	}
}

void Scraper::Init() {
	if (singles.GetCount() == 0) {
		singles.Add("des").SetBeginYear(1956).SetName("Deutsche Single-Charts");
		singles.Add("bys").SetBeginYear(1981).SetName("Bayerische Single-Charts");
		singles.Add("uks").SetBeginYear(1950).SetName("UK Single-Charts");
		singles.Add("frs").SetBeginYear(1984).SetName("Französische Single-Charts");
		singles.Add("eus").SetBeginYear(1991).SetName("MTV Europe Single-Charts");
		singles.Add("uss").SetBeginYear(1940).SetName("USA Single-Charts");
		singles.Add("ussalt").SetBeginYear(1988).SetName("Billboard #1 alternative hits").ParseUSAlt();
		singles.Add("finnish").SetBeginYear(1951).SetName("Finnish Single-Charts").ParseFinnish();
		
		for(int i = 0; i < singles.GetCount(); i++)
			singles[i].SetCode(singles.GetKey(i));
	}
	
	for(int i = 0; i < singles.GetCount(); i++) {
		SingleChart& s = singles[i];
		s.SetScraper(this);
	}
}

void Scraper::Reload() {
	int end_year = GetSysTime().year - 1;
	
	for(int i = 0; i < singles.GetCount(); i++) {
		if (singles.GetKey(i) == "ussalt") continue;
		if (singles.GetKey(i) == "finnish") continue;
		SingleChart& chart = singles[i];
		
		for(int j = chart.GetBeginYear(); j < end_year; j++) {
			chart.RefreshYear(j);
		}
	}
	
	StoreThis();
}

void Scraper::LoadPreScraped() {
	MemReadStream in(prescraped_bin, prescraped_bin_length);
	StringStream out;
	BZ2Decompress(out, in);
	out.Seek(0);
	out.SetLoading();
	Load(*this, out);
}







//www.charts-surfer.de/musikhits.php?Spr=de&Land=des&Jahr=1973&x=184&y=19
SingleChart::SingleChart() {
	db = 0;
}

void SingleChart::RefreshYear(int year) {
	Time now = GetSysTime();
	if (now.year != year && year_week_songs.Find(year) != -1)
		return;
	
	if (year > now.year || year < 1940)
		return;
	
	String code = this->code + IntStr(year);
	String filehtm	= ConfigFile("singlechart_result_" + code + ".htm");
	String file		= ConfigFile("singlechart_result_" + code + ".xml");
	
	if (!FileExists(file)) {
		HttpRequest h;
		HttpRequest::Trace();
		
		BasicHeaders(h);
		
		String url = "http://www.charts-surfer.de/musikhits.php";
		
		h.POST();
		h.Post("Spr", "de");
		h.Post("Land", this->code);
		h.Post("Jahr", IntStr(year));
		h.Post("x", IntStr(174));
		h.Post("y", IntStr(119));
		
		
		h.SSL();
		h.Url(url);
		h.KeepAlive(false);
		
		String c = h.Execute();
		
		FileOut out(filehtm);
		out << c;
		out.Close();
		
		Tidy(file, filehtm);
		
		Sleep(500); // don't be rude client
	}
	
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
		return;
	}
	//LOG(XmlTreeString(xn));
	
	// 0 1 1 0 1 6 2 1 0
	// 0 1 1 0 1 6 3 1 0
	// 0 1 1 0 1 6 3 2 0
	// 0 1 1 0 1 6 3 4 0
	
	VectorMap<String, ChartSong>& unique_songs = year_unique_songs.Find(year) == -1 ?
		year_unique_songs.Add(year) : year_unique_songs.Get(year);
	
	VectorMap<int, int>& week_songs = year_week_songs.Find(year) == -1 ?
		year_week_songs.Add(year) : year_week_songs.Get(year);
	
	unique_songs.Clear();
	week_songs.Clear();
	
	int errorcode = 0;
	const XmlNode& list = TryOpenLocation("0 1 1 0 1 6", xn, errorcode);
	
	if (!errorcode) {
		int count = list.GetCount();
		for(int i = 2; i < count; i++) {
			const XmlNode& date = TryOpenLocation(IntStr(i) + " 1 0", list, errorcode);
			const XmlNode& title = TryOpenLocation(IntStr(i) + " 2 0", list, errorcode);
			const XmlNode& artist = TryOpenLocation(IntStr(i) + " 4 0", list, errorcode);
			String date_str = date.GetText();
			String title_str = title.GetText();
			String artist_str = artist.GetText();
			
			String keystr = title_str + " - " + artist_str;
			Vector<String> date_vec = Split(date_str, "/");
			
			int week = StrInt(date_vec[1]);
			
			int j = unique_songs.Find(keystr);
			if (j == -1) {
				j = unique_songs.GetCount();
				ChartSong& cs = unique_songs.Add(keystr);
				cs.title = title_str;
				cs.artist = artist_str;
				cs.year = StrInt(date_vec[0]);
				cs.week = week;
			}
			
			if (week_songs.Find(week) == -1)
				week_songs.Add(week, j);
			
			/*SearchResult& sr = this->list.Add();
			sr.SetTitle(title);
			sr.SetAddress("https://www.youtube.com" + href);
			*/
			LOG(i << ": " << date_str << ": " << title_str << " - " << artist_str);
		}
	}
	
	LOG("end");
}


void SingleChart::ParseUSAlt() {
	//String file = GetDataFile("usalt.txt");
	String file = GetDataFile("usalt.txt");
	ASSERT(FileExists(file));
	String content = LoadFile(file);
	LOG(content);
	
	
	content.Replace("January","1,");
	content.Replace("February","2,");
	content.Replace("March","3,");
	content.Replace("April","4,");
	content.Replace("May","5,");
	content.Replace("June","6,");
	content.Replace("July","7,");
	content.Replace("August","8,");
	content.Replace("September","9,");
	content.Replace("October","10,");
	content.Replace("November","11,");
	content.Replace("December","12,");
	
	Vector<String> lines = Split(content, "\n");
	
	for(int i = 0; i < lines.GetCount(); i++) {
		String& line = lines[i];
		line.Replace("\t", ",");
		line.Replace("\r", "");
		line.Replace(" ,",",");
		line.Replace("dagger",",dagger");
		
		LOG(line);
		
		Vector<String> sub = Split(line, ",");
		
		for(int j = 0; j < sub.GetCount(); j++) {
			if (j == 0) {
				sub[j] = TrimBoth(sub[j]);
				if (sub[j].Right(1) == "\"") break;
				else continue;
			}
			bool do_break = false;
			if (sub[j].Right(1) == "\""){
				do_break = true;
			}
			sub[0] += ", " + sub[j];
			sub.Remove(j);
			j--;
			if (do_break) break;
		}
		
		while (sub[1].Left(6) == "dagger")
			sub.Remove(1);
		
		
		DUMP(sub);
		// sub = ["Ride", Twenty One Pilots, April 9,  2016, 1]
		
		int count = sub.GetCount();
		ASSERT(count == 6);
		
		int year = StrInt(sub[4]);
		int month = StrInt(sub[2]);
		int day = StrInt(sub[3]);
		Time time(year, month, day);
		
		int week = GetWeek(time, year);
		
		sub[0].Replace("\"", "");
		String keystr = sub[0] + " - " + sub[1];
		
		
		VectorMap<String, ChartSong>& unique_songs = year_unique_songs.GetAdd(year);
		VectorMap<int, int>& week_songs = year_week_songs.GetAdd(year);
		
		int j = unique_songs.Find(keystr);
		if (j == -1) {
			j = unique_songs.GetCount();
			ChartSong& cs = unique_songs.Add(keystr);
			cs.title = sub[0];
			cs.artist = sub[1];
			cs.year = year;
			cs.week = week;
		}
		
		if (week_songs.Find(week) == -1)
			week_songs.Add(week, j);
		
	}
	
}

void SingleChart::ParseFinnish() {
	String content = LoadFile(GetDataFile("finnish.txt"));
	Vector<String> lines = Split(content, "\n");
	for(int i = 0; i < lines.GetCount(); i++) {
		const String& line = lines[i];
		Vector<String> data = Split(line, ",");
		if (!data.GetCount()) continue;
		ASSERT(data.GetCount() == 3);
		
		String keystr = data[2] + " - " + data[1];
		int year = StrInt(data[0]);
		
		VectorMap<String, ChartSong>& unique_songs = year_unique_songs.GetAdd(year);
		VectorMap<int, int>& week_songs = year_week_songs.GetAdd(year);
		
		int j = unique_songs.Find(keystr);
		if (j == -1) {
			j = unique_songs.GetCount();
			ChartSong& cs = unique_songs.Add(keystr);
			cs.title = data[2];
			cs.artist = data[1];
			cs.year = year;
			cs.week = unique_songs.GetCount();
		}
		
		week_songs.GetAdd(j, j);
	}
}
