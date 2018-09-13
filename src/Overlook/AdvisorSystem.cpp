#include "Overlook.h"

namespace Overlook {

AdvisorSystem::AdvisorSystem() {
	
}

void AdvisorSystem::Init() {
	System& sys = GetSystem();
	
	int sym_count = sys.GetNetCount();
	int tf_count = 5;
	
	cores.AddIndi(sys.Find<DataBridge>());
	for(int i = 0; i < sym_count; i++)
		cores.AddSymbol(sys.GetSymbol(sys.GetNormalSymbolCount() + sys.GetCurrencyCount() + i));
	cores.AddTf(0);
	cores.Init();
	
	pricecores.AddIndi(sys.Find<DataBridge>());
	System::NetSetting& net = sys.GetNet(0);
	for(int i = 0; i < net.symbols.GetCount(); i++)
		pricecores.AddSymbol(net.symbols.GetKey(i));
	pricecores.AddTf(0);
	pricecores.Init();
	
	
	for(int i = 0; i < AdvisorFactories().GetCount(); i++) {
		AdvisorFactoryPtr new_fn = AdvisorFactories()[i].b;
		One<Advisor> adv = new_fn();
		
		if (adv->GetArgCount() == 0) {
			advisors.Add(new_fn());
		}
		else {
			Panic("TODO");
		}
	}
	
	LoadThis();
	
	if (inputs.IsEmpty()) {
		inputs.SetCount(sym_count);
		for(int i = 0; i < sym_count; i++) {
			inputs[i].SetCount(tf_count);
			for(int j = 0; j < tf_count; j++) {
				int tf_min = sys.GetPeriod(j);
				for (int period = 20; period <= 100; period += 10)
					for (int dev = 5; dev <= 20; dev+=1)
						inputs[i][j].Add().Set(0, tf_min, period * tf_min, dev);
				inputs[i][j].Add().Set(1, tf_min);
			}
		}
		
		
		StoreThis();
	}
	
}

void AdvisorSystem::Start() {
	while (Tick())
		;
}

bool AdvisorSystem::Tick() {
	bool all_usable = true;
	for(int i = 0; i < cores.GetSymbolCount() && all_usable; i++) {
		ConstBuffer& open_buf = cores.GetBuffer(i, 0, 0);
		if (cursor >= open_buf.GetCount())
			all_usable = false;
	}
	
	if (!all_usable)
		return false;
	
	ConstBuffer& time = cores.GetBuffer(0, 0, 4);
	Time t = Time(1970,1,1) + time.Get(cursor);
	int wday = DayOfWeek(t);
	
	for(int i = 0; i < inputs.GetCount(); i++) {
		ConstBuffer& open = cores.GetBuffer(i, 0, 0);
		double o0 = open.Get(cursor);
		auto& sym = inputs[i];
		for(int j = 0; j < sym.GetCount(); j++) {
			auto& tf = sym[j];
			for(int k = 0; k < tf.GetCount(); k++) {
				tf[k].Tick(cursor, o0, t, wday);
			}
		}
	}
	
	//if (t.minute == 0)
	{
		for(int i = 0; i < advisors.GetCount(); i++) {
			Advisor& a = advisors[i];
			a.Tick();
			a.CycleBroker();
			LOG(cursor << " " << a.broker.AccountEquity());
		}
	}
	
	cursor++;
	return true;
}





void AdvisorInput::Init() {
	if (type == 0) {
		stdav.SetPeriod(arg0);
	}
	else if (type == 1) {
		anomaly_var.SetCount(24*7);
	}
	else Panic("Invalid type");
	
	av.SetCount(24 * 7);
	for(int i = 0; i < av.GetCount(); i++)
		av[i].SetPeriod(event_count);
}

void AdvisorInput::Tick(int cursor, double open, const Time& t, int wday) {
	
	bool is_enabled = false;
	
	if (type == 0) {
		stdav.Add(open);
		double stddev = stdav.Get();
		double change = stddev * (arg1 * 0.1);
		double ma = stdav.GetMean();
		double tl = ma + change;
		double bl = ma - change;
		if (cursor >= arg0) {
			if (open >= tl) {
				is_enabled = true;
				signal = 0;
			}
			else if (open <= bl) {
				is_enabled = true;
				signal = 1;
			}
		}
	}
	else if (type == 1) {
		if (prev_var >= 0) {
			/*OnlineVariance& var = anomaly_var[prev_var];
			double change = open / prev - 1.0;
			double fchange = fabs(change);
			var.Add(fchange);
			
			double mean = var.GetMean();
			double value;
			if (fchange >= mean) {
				value = var.GetCDF(fchange, false);
			} else {
				value = var.GetCDF(fchange, true);
			}
			value = (value - 0.5) * 2.0;
			
			if (value >= 0.75) {
				is_enabled = true;
				signal = change < 0.0;
			}*/
		}
		prev_var = wday * 24 + t.hour;
		prev = open;
	}
	else Panic("Invalid type");
	
	//if (t.minute == 0)
	{
		bool is_trigger = is_enabled && !prev_enabled;
		prev_enabled = is_enabled;
		
		
		for(int i = 0; i < temp.GetCount(); i++) {
			StatCollectTemp& t = temp[i];
			if (cursor >= t.end) {
				double change = open / t.open - 1.0;
				if (t.signal)
					change *= -1;
				OnlineVarianceWindow& var = av[t.var];
				var.Add(change);
				temp.Remove(i);
				i--;
			}
		}
		
		if (is_trigger) {
			StatCollectTemp& tmp = temp.Add();
			tmp.open = open;
			tmp.end = cursor + tf_min;
			tmp.var = wday * 24 + t.hour;
			tmp.signal = signal;
		}
		
		
		const OnlineVarianceWindow& var = av[wday * 24 + t.hour];
		int event_count = var.Get().GetEventCount();
		double cdf = var.Get().GetCDF(0.0, true);
		int grade = (1.0 - cdf) / grade_div;
		int inv_grade = cdf / grade_div;
		is_straight_trigger = grade < grade_count && event_count >= AdvisorInput::event_count;
		is_inverse_trigger = inv_grade < grade_count && event_count >= AdvisorInput::event_count;
		mean = var.Get().GetMean();
	}
}





Advisor::Advisor() {
	((Brokerage&)broker).operator=((Brokerage&)GetMetaTrader());
	broker.SetInitialBalance(1000);
	broker.Clear();
}

void Advisor::CycleBroker() {
	System& sys = GetSystem();
	AdvisorSystem& as = GetAdvisorSystem();
	
	System::NetSetting& net = sys.GetNet(0);
	
	for(int i = 0; i < net.symbol_ids.GetCount(); i++) {
		int id = net.symbol_ids.GetKey(i);
		double open = as.pricecores.GetBuffer(i, 0, 0).Get(as.cursor);
		broker.SetPrice(id, open, open);
	}


	if (active_net >= 0) {
		System::NetSetting& net = sys.GetNet(active_net);
		for(int i = 0; i < net.symbol_ids.GetCount(); i++) {
			int id = net.symbol_ids.GetKey(i);
			int sig = net.symbol_ids[i];
			if (active_signal) sig *= -1;
			int prev_sig = broker.GetSignal(id);
			broker.SetSignal(id, sig);
			broker.SetSignalFreeze(id, prev_sig == sig);
		}
	}
	else {
		for(int i = 0; i < net.symbol_ids.GetCount(); i++) {
			int id = net.symbol_ids.GetKey(i);
			broker.SetSignal(id, 0);
			broker.SetSignalFreeze(id, false);
		}
	}
	
	broker.SetFreeMarginLevel(0.95);
	broker.SetFreeMarginScale(net.symbol_ids.GetCount());
	broker.RefreshOrders();
	broker.Cycle();
}




AdvisorSystemCtrl::AdvisorSystemCtrl() {
	
}

void AdvisorSystemCtrl::Data() {
	
}
	
}
