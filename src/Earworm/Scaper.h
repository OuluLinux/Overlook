#ifndef _Earworm_Scaper_h_
#define _Earworm_Scaper_h_

#include <Core/Core.h>
using namespace Upp;

struct ChartSong : public Moveable<ChartSong> {
	ChartSong() {}
	ChartSong(const ChartSong& cs) {title = cs.title; artist = cs.artist; year = cs.year; week = cs.week;}
	String title, artist;
	int year, week;
	void Serialize(Stream& s) {s % title % artist % year % week;}
};

class Scraper;

class SingleChart {
	VectorMap<int, VectorMap<String, ChartSong> > year_unique_songs;
	VectorMap<int, VectorMap<int, int> > year_week_songs;
	
	int begin_year;
	String code;
	String name;
	
	Scraper* db;
	
public:
	SingleChart();
	
	int GetBeginYear() const {return begin_year;}
	String GetName() const {return name;}
	
	SingleChart& SetBeginYear(int year) {begin_year = year; return *this;}
	SingleChart& SetName(String s) {name = s; return *this;}
	void SetCode(String s) {code = s;}
	void SetScraper(Scraper* cdb) {db = cdb;}
	
	void Serialize(Stream& s) {
		s % year_unique_songs % year_week_songs % begin_year % code % name;
	}
	
	void RefreshYear(int year);
	int GetUniqueCount(int year) {if (year_unique_songs.Find(year)==-1) return 0; return year_unique_songs.Get(year).GetCount();}
	
	ChartSong GetUniqueSong(int year, int i) {return year_unique_songs.Get(year)[i];}
	
	void ParseUSAlt();
	void ParseFinnish();
};


class Scraper {
	ArrayMap<String, SingleChart> singles;
	
public:
	typedef Scraper CLASSNAME;
	Scraper();
	
	void Init();
	void Reload();
	void StoreThis() {StoreToFile(*this, ConfigFile("Scraper.bin"));}
	void LoadThis() {LoadFromFile(*this, ConfigFile("Scraper.bin"));}
	void LoadPreScraped();
	SingleChart& GetChart(String code) {return singles.Get(code);}
	SingleChart& GetChart(int i) {return singles[i];}
	int GetChartCount() const {return singles.GetCount();}
	String GetChartName(int i) const {return singles.GetKey(i);}
	
	void Serialize(Stream& s) {s % singles;}
	
};

#endif
