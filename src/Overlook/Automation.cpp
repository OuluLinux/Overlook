#include "Overlook.h"

namespace Overlook {
using namespace Forecast;

Automation::Automation() {
	last_update = Time(1970,1,1);
	
	symbols.Add("EURUSD");
	symbols.Add("GBPUSD");
	symbols.Add("USDCHF");
	symbols.Add("USDJPY");
	symbols.Add("USDCAD");
	//symbols.Add("AUDUSD");
	//symbols.Add("NZDUSD");
	//symbols.Add("EURCHF");
	symbols.Add("EURJPY");
	symbols.Add("EURGBP");
	
	
	Manager& mgr = GetManager();
	
	for(int i = 0; i < symbols.GetCount(); i++)
		mgr.GetAdd(symbols[i]);
	
}

void Automation::Init() {
	
}

void Automation::Start() {
	RLOG("Automation::Start");
	Manager& mgr = GetManager();
	
	bool all_finished = true;
	for(int i = 0; i < symbols.GetCount(); i++) {
		if (mgr.GetAdd(symbols[i]).IsFinished() == false) {
			all_finished = false;
			break;
		}
	}
	
	if (mode == NO_PENDING && all_finished) {
		for(int i = 0; i < symbols.GetCount(); i++) {
			CoreList cl;
			cl.AddSymbol(symbols[i]);
			cl.AddTf(0);
			cl.AddIndi(0);
			cl.Init();
			cl.Refresh();
			ConstBuffer& src = cl.GetBuffer(0, 0, 0);
			int size = 5*1440;
			Vector<double> data;
			int begin = src.GetCount() - size;
			data.SetCount(size);
			for(int j = 0; j < size; j++)
				data[j] = src.Get(begin + j);
			mgr.GetAdd(symbols[i]).AddForecastTask(data);
		}
		mode++;
	}
	else if (mode == PENDING && all_finished) {
		System& sys = GetSystem();
		Sentiment& sent = GetSentiment();
		
		last_update = GetUtcTime();
		
		SentimentSnapshot* prev = NULL;
		if (sent.GetSentimentCount())
			prev = &sent.GetSentiment(sent.GetSentimentCount()-1);
		SentimentSnapshot& snap = sent.AddSentiment();
		snap.added = GetUtcTime();
		snap.cur_pres.SetCount(sys.GetCurrencyCount(), 0);
		snap.pair_pres.SetCount(sent.symbols.GetCount(), 0);
		snap.comment = "Automation";
		snap.equity = GetMetaTrader().AccountEquity();
		
		for(int i = 0; i < symbols.GetCount(); i++) {
			int j = sent.symbols.Find(symbols[i]);
			if (j == -1)
				continue;
			Forecast::Session& ses = mgr.GetAdd(symbols[i]);
			Forecast::Task& t = ses.tasks.Top();
			if (t.forecast.IsEmpty())
				continue;
			
			double first_change = t.forecast[0];
			double peak_up = 0, peak_down = 0;
			int peak_up_pos = 0, peak_down_pos = 0;
			for(int k = 0; k < t.forecast.GetCount(); k++) {
				double d = t.forecast[k];
				if (d > peak_up) {peak_up = d; peak_up_pos = k;}
				if (d < peak_down) {peak_down = d; peak_down_pos = k;}
			}
			bool down_before_up = peak_down_pos < peak_up_pos;
			bool peak_up_stronger = peak_up > -peak_down;
			
			int prev_pres = prev ? prev->pair_pres[j] : 0;
			int pres = 0;
			if (prev_pres) {
				// Simple keep on going
				if ((prev_pres > 0 && first_change > 0) || (prev_pres < 0 && first_change < 0))
					pres = prev_pres;
				// Continue while seeing positive
				/*else if ((prev_pres > 0 && peak_up > 0) || (prev_pres < 0 && peak_down < 0))
					pres = prev_pres;*/
				// Change signal
				else
					pres = peak_up_stronger ? +1 : -1;
			}
			else {
				// Wait better opportunity
				if ((peak_up_stronger && down_before_up) || (!peak_up_stronger && !down_before_up))
					pres = 0;
				// Opportunity ongoing
				else
					pres = peak_up_stronger ? +1 : -1;
			}
			
			if (prev_pres != 0 && pres * prev_pres < 0) {
				double open_profit = GetMetaTrader().GetOpenProfit(symbols[i]);
				if (open_profit < 0) {
					pres = -prev_pres * 2;
				}
			}
			
			snap.pair_pres[j] = pres;
		}
		
		sent.StoreThis();
		mode++;
	}
	else if (mode == WAITING) {
		Time now = GetUtcTime();
		if (now.Get() - last_update.Get() > 10*60) {
			mode = NO_PENDING;
		}
	}
}










AutomationCtrl::AutomationCtrl() {
	
}

void AutomationCtrl::Data() {
	
}

}
