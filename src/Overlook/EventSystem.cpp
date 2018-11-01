#include "Overlook.h"
#include <plugin/cctz/civil_time.h>
#include <plugin/cctz/time_zone.h>

namespace Overlook {

EventSystem::EventSystem() {
	ses_counted = Date(1970,1,1);
	
	
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
	
	
	
}

EventSystem::~EventSystem() {
	
}

void EventSystem::Init() {
	
	
}

void EventSystem::Start() {
	DataBridgeCommon& db = GetDataBridgeCommon();
	CalendarCommon& cal = GetCalendar();
	const Index<Time>& time = db.GetTimeIndex();
	ASSERT(!time.IsEmpty());
	
	pairs <<= GetSentiment().symbols;
	
	int resp_counted = responses.GetCount() / (RESP_COUNT * pairs.GetCount());
	responses.SetCount(time.GetCount() * RESP_COUNT * pairs.GetCount());
	
	TimeStop ts;
	
	
	if (ses_counted < time[0]) {
		ses_counted = time[0];
	}
	
	for(int i = cal_counted; i < cal.GetCount(); i++) {
		const CalEvent& e = cal.GetEvent(i);
		int j = time.Find(e.timestamp);
		if (j < 0) continue;
		int cur = currencies.Find(e.currency);
		if (cur < 0) continue;
		/*Vector<Event>& ev = events[j];
		Event& ese = ev.Add();
		ese.type = Event::TYPE_CALENDAR;
		ese.currency = cur;
		double act = ScanDouble(e.actual);
		double fc = ScanDouble(e.forecast);
		double prev = ScanDouble(e.previous);
		if (e.previous != "" && e.forecast != "" && prev != fc)
			ese.pre_effect = fc > prev ? +1 : -1;
		if (e.actual != "" && e.forecast != "" && act != fc)
			ese.post_effect = act > fc ? +1 : -1;
		else if (e.actual != "" && e.previous != "" && prev != act)
			ese.post_effect = act > prev ? +1 : -1;*/
	}
	cal_counted = cal.GetCount();
	
	Date now = GetUtcTime();
	for (Date d = ses_counted; d < now; d++) {
		int wday = DayOfWeek(d);
		if (wday == 0 || wday == 6) continue;
		DaySlots ds;
		GetDaySlots(d, ds);
		for(int i = 0; i < ds.time_points.GetCount(); i++) {
			TimeSlot& ts = ds.time_points[i];
			Vector<int> pair_list;
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
			Time begin = ds.time_points.GetKey(i);
			Time end;
			if (i < ds.time_points.GetCount() - 1) {
				end = ds.time_points.GetKey(i + 1);
			} else {
				end = ds.time_points.GetKey(0);
				end += 24 * 60 * 60;
			}
			ASSERT(begin.second == 0 && end.second == 0);
			for (Time t = begin; t < end; t += 60) {
				int j = time.Find(t);
				if (j < 0) continue;
				for(int k = 0; k < pair_list.GetCount(); k++) {
					int pair = pair_list[k];
					SentimentResponseProto& proto = responses[(j * pairs.GetCount() + pair) * RESP_COUNT + RESP_ISOPEN];
					proto.int_value0 = 1;
				}
			}
		}
	}
	ses_counted = now;
	
	
	for(int i = resp_counted; i < time.GetCount(); i++) {
		for (int j = 0; j < pairs.GetCount(); j++) {
			for(int k = 0; k < RESP_COUNT; k++) {
				switch (k) {
					case RESP_ISOPEN: break;
					case RESP_IDXUPDOWN: WriteIndexUpDown(i, j); break;
				}
			}
		}
	}
	
	LOG(ts.ToString());
	
	/*if (opt.GetRound() == 0) {
		opt.Init(RESP_COUNT *2, 100);
	}
	
	if (!running && !opt.IsEnd()) {
		running = true; stopped = false;
		Thread::Start(THISBACK(RunOptimizer));
	}*/
}

void EventSystem::WriteIndexUpDown(int i, int pair) {
	SentimentResponseProto& proto = responses[(i * pairs.GetCount() + pair) * RESP_COUNT + RESP_IDXUPDOWN];
	
	int cl_id = pair * RESP_COUNT + RESP_IDXUPDOWN;
	int cl_pos = corelists.Find(cl_id);
	if (cl_pos < 0) {
		cl_pos = corelists.GetCount();
		CoreList& cl = corelists.Add(cl_id);
		const String& pair_str = pairs[pair];
		String cur0 = pair_str.Left(3);
		String cur1 = pair_str.Right(3);
		cl.AddSymbol(cur0);
		cl.AddSymbol(cur1);
		cl.AddTf(0);
		cl.AddIndi(System::Find<MovingAverage>());
		cl.Init();
		cl.Refresh();
	}
	
	CoreList& cl = corelists[cl_pos];
	ConstBuffer& cur0 = cl.GetBuffer(0, 0, 0);
	ConstBuffer& cur1 = cl.GetBuffer(1, 0, 0);
	if (i >= cur0.GetCount()) cl.Refresh();
	
	double d0 = cur0.Get(i);
	double d1 = cur1.Get(i);
	proto.int_value0 = d0 * d1 < 0;
	proto.int_value1 = d0 > 0 ? +1 : -1;
}

void EventSystem::GetSentimentResponses(int i, int pair, Vector<SentimentResponse>& response, int signal) {
	response.SetCount(RESP_COUNT);
	
	for(int j = 0; j < RESP_COUNT; j++) {
		const SentimentResponseProto& proto = responses[(i * pairs.GetCount() + pair) * RESP_COUNT + j];
		SentimentResponse& resp = response[j];
		
		switch (j) {
		
		case RESP_IDXUPDOWN:
			// If other currency's MA up and other's down
			if (proto.int_value0) {
				if (signal == 0 || signal != proto.int_value1)
					resp.type = -1;
				else
					resp.type = +1;
			} else {
				resp.type = -1;
			}
			resp.is_goal = false;
			break;
			
		default:
			Panic("Unexpected proto");
		}
		
	}
}

void EventSystem::RunOptimizer() {
	const Index<Time>& time = GetDataBridgeCommon().GetTimeIndex();
	
	Vector<SentimentResponse> response;
	
	SimBroker sb;
	
	while (!opt.IsEnd()) {
		
		opt.Start();
		
		const Vector<double>& trial = opt.GetTrialSolution();
		
		
		for(int i = 0; i < time.GetCount(); i++) {
			
			SentimentSnapshot snap;
			
			for (int pair = 0; pair < pairs.GetCount(); pair++) {
				
				
				double max_score = -DBL_MAX;
				int max_signal;
				for (int signal = -1; signal <= +1; signal++) {
					GetSentimentResponses(i, pair, response, signal);
					
					double score = 0;
					for(int i = 0; i < response.GetCount(); i++) {
						SentimentResponse& r = response[i];
						
						if (!r.is_goal) {
							double factor = 0;
							if (r.type == 1)		factor = trial[i * 2 + 0];
							else if (r.type == 1)	factor = trial[i * 2 + 1];
							score += factor;
						} else {
							
						}
					}
					
					if (score > max_score) {
						max_score = score;
						max_signal = signal;
					}
				}
				
				snap.pair_pres[pair] = max_signal;
			}
			
			
		}
		
		double result = sb.AccountEquity();
		opt.Stop(result);
	}
	
}

void EventSystem::GetDaySlots(Date date, DaySlots& slot) {
	
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
	
	
	DUMPM(slot.time_points);
}




void TestEventSystem() {
	EventSystem& es = GetEventSystem();
	
	Date d(2018,3,29);
	DaySlots ds;
	es.GetDaySlots(d, ds);
	
	
}















EventSystemCtrl::EventSystemCtrl() {
	
}
	
void EventSystemCtrl::Data() {
	
}

}

