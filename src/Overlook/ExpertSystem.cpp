#include "Overlook.h"
#include <plugin/cctz/civil_time.h>
#include <plugin/cctz/time_zone.h>

namespace Overlook {

ExpertSystem::ExpertSystem() {
	
	
	
	/*
	
	Approximation in UTC
	
	NZD 22:00	05:00
	AUD 00:00	06:00
	JPY 00:00	06:00		02:30–03:30
	EUR 08:00	16:30
	    07:00	19:00
	    07:00	21:00
	CHF 08:00	16:30
	USD 14:30	21:00
	CAD 14:30	21:00
	
	00:00 - 02:00	NZD,AUD,JPY
	02:00 - 04:00	NZD,AUD,JPY		Japan lunch
	04:00 - 05:00	NZD,AUD,JPY
	05:00 - 06:00	AUD,JPY
	06:00 - 07:00	
	07:00 - 08:00	EUR
	08:00 - 10:00	EUR,CHF
	10:00 - 12:00	EUR,CHF
	12:00 - 14:30	EUR,CHF
	14:30 - 16:30	EUR,CHF,USD,CAD
	16:30 - 19:00	EUR,CHF,USD,CAD
	19:00 - 21:00	EUR,CHF,USD,CAD
	21:00 - 22:00	
	22:00 - 20:00	NZD
	
	However, exchanges are open in local time, which fluctuate with daylight savings time.
	
	NZD	      10:00	16:45	NZST
	AUD	      10:00	16:00	AEST
	JPY	      09:00	15:00	JST
	EUR	Paris 09:00	17:30	CET
	EUR FSX   08:00	20:00	CET
	EUR EUREX 08:00	22:00	CET
	GBP       08:00	16:30	GMT
	USD NYSE  09:30	16:00	EST
	USD NASQ  09:30	16:00	EST
	CAD       09:30	16:00	EST
	*/
	
	currencies.Add().Set("NZD").AddOpenLocal(1000, 1645).SetTZ("NZ");
	currencies.Add().Set("AUD").AddOpenLocal(1000, 1600).SetTZ("Australia/Sydney");
	currencies.Add().Set("JPY").AddOpenLocal( 900, 1500).SetTZ("Japan");
	currencies.Add().Set("EUR").SetTZ("Europe/Amsterdam")
		.AddOpenLocal( 900, 1730)
		.AddOpenLocal( 800, 2000)
		.AddOpenLocal( 800, 2200);
	currencies.Add().Set("GBP").SetTZ("Europe/London").AddOpenLocal( 800, 1630);
	currencies.Add().Set("CHF").SetTZ("Europe/Zurich").AddOpenLocal( 930, 1600);
	currencies.Add().Set("USD").SetTZ("America/New_York").AddOpenLocal( 930, 1600);
	currencies.Add().Set("CAD").SetTZ("America/Toronto").AddOpenLocal( 930, 1600);
	
	
	
	
	
	
	
	
}

ExpertSystem::~ExpertSystem() {
	
}

void ExpertSystem::GetDaySlots(Date date, DaySlots& slot) {
	
	slot.Clear();
	
	cctz::time_zone utc;
	ASSERT(cctz::load_time_zone("UTC", &utc));
	
	for(int i = 0; i < currencies.GetCount(); i++) {
		Currency& cur = currencies[i];
		String tzpath = cur.GetTZ();
		LOG(cur.GetName());
		LOG(tzpath);
		
		cctz::time_zone tz;
		if(!cctz::load_time_zone(tzpath.Begin(), &tz))
			Panic("Include timezone files with executable");
	
		for(int j = 0; j < cur.GetLocalCount(); j++) {
			Time t(date.year, date.month, date.day);
			
			{
				cur.GetLocalOpen(j, t);
				const auto utc_open = cctz::convert(cctz::civil_second(t.year, t.month, t.day, t.hour, t.minute, 0), tz);
				const cctz::time_zone::absolute_lookup al = utc.lookup(utc_open);
				Time open(al.cs.year(), al.cs.month(), al.cs.day(), al.cs.hour(), al.cs.minute());
				DUMP(open);
				slot.time_points.GetAdd(open).AddOpen(cur.GetName() + IntStr(j));
			}
			
			{
				cur.GetLocalClose(j, t);
				const auto utc_close = cctz::convert(cctz::civil_second(t.year, t.month, t.day, t.hour, t.minute, 0), tz);
				const cctz::time_zone::absolute_lookup al = utc.lookup(utc_close);
				Time close(al.cs.year(), al.cs.month(), al.cs.day(), al.cs.hour(), al.cs.minute());
				DUMP(close);
				slot.time_points.GetAdd(close).AddClose(cur.GetName() + IntStr(j));
			}
			
		}
	}
	
	
	SortByKey(slot.time_points, StdLess<Time>());
	DUMPM(slot.time_points);
	
	Index<String> is_open;
	for(int i = 0; i < slot.time_points.GetCount(); i++) {
		TimeSlot& ts = slot.time_points[i];
		
		for(int j = 0; j < is_open.GetCount(); j++) {
			ts.was_open.Add(is_open[j]);
		}
		
		for(int j = 0; j < ts.close.GetCount(); j++) {
			int k = is_open.Find(ts.close[j]);
			if (k == -1) Panic("ExpertSystem: Expected open exchange");
			is_open.Remove(k);
		}
		
		
		for(int j = 0; j < ts.open.GetCount(); j++) {
			is_open.Add(ts.open[j]);
		}
		SortIndex(is_open, StdLess<String>());
		
		for(int j = 0; j < is_open.GetCount(); j++) {
			ts.is_open.Add(is_open[j]);
		}
		
		
	}
	
	
	DUMPM(slot.time_points);
}



void TestExpertSystem() {
	ExpertSystem& es = GetExpertSystem();
	
	Date d(2018,3,29);
	DaySlots ds;
	es.GetDaySlots(d, ds);
	
	
}

}
