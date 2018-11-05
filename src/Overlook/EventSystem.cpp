#include "Overlook.h"
#include <plugin/cctz/civil_time.h>
#include <plugin/cctz/time_zone.h>

namespace Overlook {

EventSystem::EventSystem() {
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
	
	currencies.Add("NZD").Set("NZD").AddOpenLocal(1000, 1645).SetTZ("NZ");
	currencies.Add("AUD").Set("AUD").AddOpenLocal(1000, 1600).SetTZ("Australia/Sydney");
	currencies.Add("JPY").Set("JPY").AddOpenLocal( 900, 1500).SetTZ("Japan");
	currencies.Add("EUR").Set("EUR").SetTZ("Europe/Amsterdam")
		.AddOpenLocal( 900, 1730)
		.AddOpenLocal( 800, 2000)
		.AddOpenLocal( 800, 2200);
	currencies.Add("GBP").Set("GBP").SetTZ("Europe/London").AddOpenLocal( 800, 1630);
	currencies.Add("CHF").Set("CHF").SetTZ("Europe/Zurich").AddOpenLocal( 930, 1600);
	currencies.Add("USD").Set("USD").SetTZ("America/New_York").AddOpenLocal( 930, 1600);
	currencies.Add("CAD").Set("CAD").SetTZ("America/Toronto").AddOpenLocal( 930, 1600);
	
	
	LoadThis();
}

EventSystem::~EventSystem() {
	
}

void EventSystem::Init() {
	
	pairs <<= GetSentiment().symbols;
	
}

void EventSystem::Start() {
	DataBridgeCommon& db = GetDataBridgeCommon();
	const Index<Time>& time = db.GetTimeIndex();
	ASSERT(!time.IsEmpty());
	
	Sentiment& sent = GetSentiment();
	if (sent.GetSentimentCount()) {
		int sent_id = sent.GetSentimentCount() - 1;
		SentimentSnapshot& snap = sent.GetSentiment(sent_id);
		Index<EventError> errors;
		GetErrorList(snap, errors);
		
		bool changes = false;
		for(int i = 0; i < errors.GetCount(); i++) {
			const EventError& e = errors[i];
			if (prev_errors.Find(e) == -1) {
				if (prev_sent_id == sent_id)
					WhenError(e);
				history_errors.Add(e);
				changes = true;
			}
		}
		prev_sent_id = sent_id;
		
		prev_errors <<= errors;
		if (changes)
			StoreThis();
	}
}

void EventSystem::GetErrorList(const SentimentSnapshot& snap, Index<EventError>& errors) {
	CalendarCommon& cal = GetCalendar();
	
	Time now = GetUtcTime();
	Time begin = now - 24*60*60;
	Time pre_news_begin = now;
	Time pre_news_end = now + 60*60;
	Time post_news_begin = now - 3*60*60;
	Time post_news_end = now;
	Time h24_begin = now;
	Time h24_end = now + 24*60*60;
	
	bool high_usd_news_upcoming = false;
	
	for(int i = cal.GetCount() - 1; i >= 0; i--) {
		const CalEvent& e = cal.GetEvent(i);
		if (e.timestamp < begin)
			break;
		int cur = currencies.Find(e.currency);
		if (cur < 0) continue;
		
		if (e.timestamp >= pre_news_begin && e.timestamp <= pre_news_end) {
			AddInfo(errors, e.currency + " pre-news: " + e.title);
			PreNewsErrors(e, snap, errors);
		}
		if (e.timestamp >= post_news_begin && e.timestamp <= post_news_end) {
			AddInfo(errors, e.currency + " post-news: " + e.title);
			PostNewsErrors(e, snap, errors);
		}
		
		if (e.timestamp >= h24_begin && e.timestamp <= h24_end) {
			if (e.currency == "USD" && e.impact == 3) {
				high_usd_news_upcoming = true;
			}
		}
	}
	
	
	if (high_usd_news_upcoming) {
		if (IsSessionOpen("AUD")) {
			AddInfo(errors, "AUD Expecting high volatility");
			if (snap.cur_pres[GetSentiment().currencies.Find("AUD")] == 0)
				AddError(errors, "AUD not traded");
		}
		if (IsSessionOpen("JPY")) {
			AddInfo(errors, "JPY Expecting high volatility");
			if (snap.cur_pres[GetSentiment().currencies.Find("JPY")] == 0)
				AddError(errors, "JPY not traded");
		}
	}
	
	GetSessionErrors(snap, errors);
}

void EventSystem::PreNewsErrors(const CalEvent& e, const SentimentSnapshot& snap, Index<EventError>& errors) {
	
	if (e.impact < 2)
		return;
	
	double fc = ScanDouble(e.forecast);
	double prev = ScanDouble(e.previous);
	
	// Same with forecast vs previous change
	int pre_effect = 0;
	if (e.previous != "" && e.forecast != "" && prev != fc)
		pre_effect = fc > prev ? +1 : -1;
	if (e.IsNegative())
		pre_effect *= -1;
	for(int i = 0; i < pairs.GetCount(); i++) {
		int pres = snap.pair_pres[i];
		const String& pair = pairs[i];
		String a = pair.Left(3);
		String b = pair.Right(3);
		if (e.currency == a) {
			if (pres * pre_effect < 0)
				AddWarning(errors, pair + " against pre news " + e.title);
			if (!pres)
				AddError(errors, pair + " inactive");
		}
		if (e.currency == b) {
			if (pres * pre_effect > 0)
				AddWarning(errors, pair + " against pre news " + e.title);
			if (!pres)
				AddError(errors, pair + " inactive");
		}
	}
	
	
	// Against 12 hour change
	
}

void EventSystem::PostNewsErrors(const CalEvent& e, const SentimentSnapshot& snap, Index<EventError>& errors) {
	
	if (e.impact < 2)
		return;
	
	bool post_effect1_affects = e.currency == "AUD" || e.currency == "JPY";
	
	double act = ScanDouble(e.actual);
	double fc = ScanDouble(e.forecast);
	double prev = ScanDouble(e.previous);
	
	int post_effect0 = 0;
	if (e.actual != "" && e.forecast != "" && act != fc)
		post_effect0 = act > fc ? +1 : -1;
	
	int post_effect1 = 0;
	if (e.actual != "" && e.previous != "" && prev != act)
		post_effect1 = act > prev ? +1 : -1;
	
	if (e.IsNegative()) {
		post_effect0 *= -1;
		post_effect1 *= -1;
	}
	
	for(int i = 0; i < pairs.GetCount(); i++) {
		int pres = snap.pair_pres[i];
		const String& pair = pairs[i];
		String a = pair.Left(3);
		String b = pair.Right(3);
		if (e.currency == a) {
			if (pres * post_effect0 < 0)
				AddWarning(errors, pair + " against pre news " + e.title);
			if (pres * post_effect1 < 0 && post_effect1_affects)
				AddWarning(errors, pair + " against pre news " + e.title);
			if (!pres)
				AddError(errors, pair + " inactive");
		}
		if (e.currency == b) {
			if (pres * post_effect0 > 0)
				AddWarning(errors, pair + " against pre news " + e.title);
			if (pres * post_effect1 > 0 && post_effect1_affects)
				AddWarning(errors, pair + " against pre news " + e.title);
			if (!pres)
				AddError(errors, pair + " inactive");
		}
	}
}

bool EventSystem::IsSessionOpen(String currency) {
	Date today = GetUtcTime();
	return IsSessionOpen(currency, today-1) || IsSessionOpen(currency, today) || IsSessionOpen(currency, today+1);
}

bool EventSystem::IsSessionOpen(String currency, Date d) {
	Time now = GetUtcTime();
	DaySlots ds;
	GetDaySlots(d, ds);
	for(int i = 0; i < ds.time_points.GetCount(); i++) {
		TimeSlot& ts = ds.time_points[i];
		if (ts.is_open_cur.Find(currency) == -1)
			continue;
		Time begin = ds.time_points.GetKey(i);
		Time end;
		if (i < ds.time_points.GetCount() - 1) {
			end = ds.time_points.GetKey(i + 1);
		} else {
			end = ds.time_points.GetKey(0);
			end += 24 * 60 * 60;
		}
		if (now >= begin && now <= end) {
			return true;
		}
	}
	return false;
}

void EventSystem::GetSessionErrors(const SentimentSnapshot& snap, Index<EventError>& errors) {
	Time now = GetUtcTime();
	DaySlots ds;
	GetDaySlots(now, ds);
	for(int i = 0; i < ds.time_points.GetCount(); i++) {
		Time begin = ds.time_points.GetKey(i);
		Time end;
		if (i < ds.time_points.GetCount() - 1) {
			end = ds.time_points.GetKey(i + 1);
		} else {
			end = ds.time_points.GetKey(0);
			end += 24 * 60 * 60;
		}
		if (now >= begin && now <= end) {
			TimeSlot& ts = ds.time_points[i];
			Index<int> pair_list;
			for(int j0 = 0; j0 < ts.is_open_cur.GetCount(); j0++) {
				String a = ts.is_open_cur[j0];
				for(int j1 = 0; j1 < ts.is_open_cur.GetCount(); j1++) {
					if (j0 == j1) continue;
					String b = ts.is_open_cur[j1];
					int k = pairs.Find(a + b);
					if (k >= 0) pair_list.Add(k);
					k = pairs.Find(b + a);
					if (k >= 0) pair_list.Add(k);
				}
			}
			
			for(int i = 0; i < pairs.GetCount(); i++) {
				int pres = snap.pair_pres[i];
				if (pres != 0 && pair_list.Find(i) == -1) {
					String p = pairs[i];
					if (p.Find("USD") == -1)
						AddWarning(errors, "Pair " + p + " doesn't have open sessions");
				}
			}
		}
	}
}

void EventSystem::AddError(Index<EventError>& errors, String err) {
	EventError e;
	e.msg = err;
	e.level = 2;
	e.time = GetUtcTime();
	errors.Add(e);
}

void EventSystem::AddWarning(Index<EventError>& errors, String err) {
	EventError e;
	e.msg = err;
	e.level = 1;
	e.time = GetUtcTime();
	errors.Add(e);
}

void EventSystem::AddInfo(Index<EventError>& errors, String err) {
	EventError e;
	e.msg = err;
	e.level = 0;
	e.time = GetUtcTime();
	errors.Add(e);
}

void EventSystem::GetDaySlots(Date date, DaySlots& slot) {
	
	slot.Clear();
	
	cctz::time_zone utc;
	ASSERT(cctz::load_time_zone("UTC", &utc));
	
	for(int i = 0; i < currencies.GetCount(); i++) {
		Currency& cur = currencies[i];
		String tzpath = cur.GetTZ();
		//LOG(cur.GetName());
		//LOG(tzpath);
		
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
				//DUMP(open);
				slot.time_points.GetAdd(open).AddOpen(cur.GetName() + IntStr(j));
			}
			
			{
				cur.GetLocalClose(j, t);
				const auto utc_close = cctz::convert(cctz::civil_second(t.year, t.month, t.day, t.hour, t.minute, 0), tz);
				const cctz::time_zone::absolute_lookup al = utc.lookup(utc_close);
				Time close(al.cs.year(), al.cs.month(), al.cs.day(), al.cs.hour(), al.cs.minute());
				//DUMP(close);
				slot.time_points.GetAdd(close).AddClose(cur.GetName() + IntStr(j));
			}
			
		}
	}
	
	
	SortByKey(slot.time_points, StdLess<Time>());
	//DUMPM(slot.time_points);
	
	Index<String> is_open;
	for(int i = 0; i < slot.time_points.GetCount(); i++) {
		TimeSlot& ts = slot.time_points[i];
		
		for(int j = 0; j < is_open.GetCount(); j++) {
			ts.was_open.Add(is_open[j]);
		}
		
		for(int j = 0; j < ts.close.GetCount(); j++) {
			int k = is_open.Find(ts.close[j]);
			if (k == -1) Panic("EventSystem: Expected open exchange");
			is_open.Remove(k);
		}
		
		
		for(int j = 0; j < ts.open.GetCount(); j++) {
			is_open.Add(ts.open[j]);
		}
		SortIndex(is_open, StdLess<String>());
		
		for(int j = 0; j < is_open.GetCount(); j++) {
			ts.is_open.Add(is_open[j]);
			ts.is_open_cur.FindAdd(is_open[j].Left(3));
		}
		
		
	}
	
	
	//DUMPM(slot.time_points);
}




void TestEventSystem() {
	EventSystem& es = GetEventSystem();
	
	Date d(2018,3,29);
	DaySlots ds;
	es.GetDaySlots(d, ds);
	
	
}















EventSystemCtrl::EventSystemCtrl() {
	Add(arr.SizePos());
	
	arr.AddColumn("Time");
	arr.AddColumn("msg");
	arr.ColumnWidths("1 3");
}
	
void EventSystemCtrl::Data() {
	EventSystem& es = GetEventSystem();
	
	int prev_count = es.history_errors.GetCount();
	for(int i = 0; i < es.history_errors.GetCount(); i++) {
		const EventError& e = es.history_errors[i];
		arr.Set(i, 0, e.time);
		arr.Set(i, 1, e.msg);
	}
	
	if (prev_count < arr.GetCount())
		arr.ScrollEnd();
	
}

}

