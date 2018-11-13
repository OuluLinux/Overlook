#include "Overlook.h"
#include <plugin/tidy/tidy.h>

namespace Overlook {
	
	
void StrTime(Time& t, String time) {
	t.month = StrInt(time.Mid(0, 2));
	t.day = StrInt(time.Mid(3, 2));
	t.year = StrInt(time.Mid(6, 4));
	t.hour = StrInt(time.Mid(11, 2));
	t.minute = StrInt(time.Mid(14, 2));
	t.second = 0;
}

MyfxbookCommon::MyfxbookCommon() {
	System& sys = GetSystem();
	
	
	
	LoadThis();
	
	if (symbols.IsEmpty()) {
		symbols.Add("EURUSD");
		symbols.Add("GBPUSD");
		symbols.Add("EURJPY");
		symbols.Add("USDJPY");
		
		symbols.Add("EURGBP");
		symbols.Add("GBPJPY");
		symbols.Add("AUDUSD");
		symbols.Add("USDCAD");
		
		symbols.Add("EURAUD");
		symbols.Add("AUDCAD");
		symbols.Add("EURCHF");
		symbols.Add("CADJPY");
		
		symbols.Add("GBPCHF");
		symbols.Add("USDCHF");
		symbols.Add("NZDUSD");
		symbols.Add("AUDJPY");
		
		
		for(int i = 0; i < symbols.GetCount(); i++) {
			SymbolStats& s = symbols[i];
			s.symbol = symbols.GetKey(i);
			s.id = sys.FindSymbol(s.symbol);
			if (s.id == -1)
				Panic("Symbol " + s.symbol + " wasn't found from system");
		}
		
		Add("https://www.myfxbook.com/members/ITRADEGOLD/itg-romania/2471064", 0, 0);
		Add("https://www.myfxbook.com/members/ITRADEGOLD/itg-spain/2475012", 0, 0);
		Add("https://www.myfxbook.com/members/ITRADEGOLD/itg-uk/2475021", 0, 0);
		Add("https://www.myfxbook.com/members/globalFXteam/inclusivefx-4/2438903", 0, 0);
		Add("https://www.myfxbook.com/members/ITRADEGOLD/itg-dubai/2505157", 0, 0);
		
	}
}
	

MyfxbookCommon::~MyfxbookCommon() {
	running = false;
	while (!stopped) Sleep(100);
}

void MyfxbookCommon::Init() {
	Thread::Start(THISBACK(Updater));
}

void MyfxbookCommon::Start() {
	
}

void MyfxbookCommon::Updater() {
	running = true;
	stopped = false;
	
	
	if (accounts.IsEmpty()) {
		for(int i = 0; i < urls.GetCount(); i++) {
			for(int j = i+1; j < urls.GetCount(); j++) {
				if (urls[i] == urls[j]) {
					urls.Remove(j);
					j--;
				}
			}
		}
		
		accounts.SetCount(urls.GetCount());
		for(int i = 0; i < accounts.GetCount(); i++) {
			Account& a = accounts[i];
			a.url = urls[i];
			a.id = a.url.Mid(a.url.ReverseFind("/") + 1);
		}
		
		RefreshHistory();
		FixOrders();
		//AddDelay(); // to simulate lagging updating
		SolveSources();
		
		if (running)
			StoreThis();
		
		ReleaseLog("Myfxbook::Updater init ready");
	}
	
	while (running) {
		TimeStop ts;
		
		Time now = GetUtcTime();
		int wday = DayOfWeek(now);
		
		if (wday >= 1 && wday <= 5)
			RefreshOpen();
		else
			for(int i = 0; i < symbols.GetCount(); i++)
				symbols[i].wait = false;
		
		int sec = ts.Elapsed() / 1000;
		for(int i = sec; i < 60 && running; i++) {
			Sleep(1000);
		}
	}
	
	stopped = true;
}

void MyfxbookCommon::SolveSources() {
	System& sys = GetSystem();
	Time now = GetUtcTime();
	
	typedef Tuple<DataBridge*, ConstBuffer*, ConstBuffer*> Ptrs;
	VectorMap<String, Ptrs> dbs;
	
	// Refresh DataBridges for allowed symbols
	for(int i = 0; i < symbols.GetCount() && running; i++) {
		SymbolStats& s = symbols[i];
		
		Index<int> tf_ids, sym_ids;
		Vector<FactoryDeclaration> indi_ids;
		Vector<Ptr<CoreItem> > work_queue;
		
		ReleaseLog("Myfxbook::SolveSources " + s.symbol + "\t" + IntStr(s.id) + " " + IntStr(i));
		ASSERT(s.id >= 0 && s.id < sys.GetSymbolCount());
		
		FactoryDeclaration decl;
		decl.factory = System::Find<DataBridge>();
		indi_ids.Add(decl);
		tf_ids.Add(0);
		sym_ids.Add(s.id);
		work_queue.Clear();
		sys.GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
		for (int j = 0; j < work_queue.GetCount(); j++)
			sys.Process(*work_queue[j], true);
		
		
		DataBridge& db = *dynamic_cast<DataBridge*>(&*work_queue[0]->core);
		ConstBuffer& openbuf = db.GetBuffer(0);
		ConstBuffer& timebuf = db.GetBuffer(4);
		
		Ptrs& ptrs = dbs.GetAdd(s.symbol);
		ptrs.a = &db;
		ptrs.b = &openbuf;
		ptrs.c = &timebuf;
	}
	
	
	VectorMap<int, double> account_results;
	
	for(int j = 0; j < accounts.GetCount() && running; j++) {
		Account& a = accounts[j];
		
		int read_pos = 0;
		
		double total_pips = 0.0;
		double total_gain = 1.0;
		double pos_pips = 0.0, neg_pips = 0.0;
		int trade_count = 0;
		
		if (a.history_orders.IsEmpty())
			continue;
		
		for(int k = 0; k < a.history_orders.GetCount() && running; k++) {
			Order& o = a.history_orders[k];
			
			int l = dbs.Find(o.symbol);
			if (l == -1)
				continue;
			
			Ptrs& ptrs = dbs[l];
			
			double point = ptrs.a->GetPoint();
			ASSERT(point > 0.0);
			ConstBuffer& openbuf = *ptrs.b;
			ConstBuffer& timebuf = *ptrs.c;
			
			while (read_pos < openbuf.GetCount()) {
				Time t = Time(1970,1,1) + timebuf.Get(read_pos);
				if (t >= o.begin)
					break;
				read_pos++;
			}
			double open_price = openbuf.Get(read_pos);
			
			while (read_pos < openbuf.GetCount()) {
				Time t = Time(1970,1,1) + timebuf.Get(read_pos);
				if (t >= o.end)
					break;
				read_pos++;
			}
			double close_price = openbuf.Get(read_pos);
			
			if (open_price <= 0.0 || close_price <= 0.0)
				continue;
			
			double diff = close_price - open_price;
			double gain = close_price / open_price - 1;
			double pips = gain;// point is unreliable diff / point;
			if (gain < -0.05 || gain > 0.05)
				continue;
			
			if (o.action) {
				diff *= -1;
				gain *= -1;
			}
			
			total_pips += fabs(pips);
			total_gain *= gain + 1;
			
			if (pips > 0) pos_pips += pips;
			else          neg_pips -= pips;
			trade_count++;
		}
		
		total_gain -= 1.0;
		
		double profitability = total_pips > 0.0 ? pos_pips / total_pips : 0.0;
		DUMP(profitability);
		DUMP(total_pips);
		DUMP(total_gain);
		
		a.profitability = profitability;
		a.pips = pos_pips - neg_pips;
		a.gain = total_gain;
		
		Order& oldest_order = a.history_orders[0];
		int64 diff = now.Get() - oldest_order.begin.Get();
		a.av_gain = a.gain / diff * 60*60*24*5;
	}
	
	Sort(accounts, Account());
	
	
}

void MyfxbookCommon::AddDelay() {
	for(int i = 0; i < accounts.GetCount() && running; i++) {
		Account& a = accounts[i];
		
		for(int j = 0; j < a.history_orders.GetCount(); j++) {
			Order& o = a.history_orders[j];
			
			o.begin += 5*60;
			o.end += 5*60;
		}
	}
}

void MyfxbookCommon::FixOrders() {
	
	
	for(int i = 0; i < accounts.GetCount() && running; i++) {
		Account& a = accounts[i];
		
		Sort(a.history_orders, Order());
		
		for(int j = 0; j < a.history_orders.GetCount(); j++) {
			Order& o = a.history_orders[j];
			
			if (o.begin == o.end) {
				a.history_orders.Remove(j);
				j--;
				continue;
			}
			else {
				ASSERT(o.begin < o.end);
			}
		}
		
		for(int j = 0; j < a.history_orders.GetCount(); j++) {
			Order& o = a.history_orders[j];
			
			ASSERT(o.begin < o.end);
			
			bool sort = false;
			bool dec = false;
			
			for(int k = j+1; k < a.history_orders.GetCount(); k++) {
				Order& next = a.history_orders[k];
				ASSERT(o.begin <= next.begin);
				
				bool rem = false;
				
				if (next.begin >= o.end)
					break;
				
				if (o.symbol == next.symbol) {
					
					if (next.begin == o.begin) {
						if (next.end == o.end) {
							int mult = o.action == next.action ? +1 : -1;
							o.lots += mult * next.lots;
							rem = true;
						}
						else {
							if (next.end < o.end) {
								Swap(next, o);
							}
							next.begin = o.end;
							ASSERT(o.begin < o.end);
							ASSERT(next.begin < next.end);
							int mult = o.action == next.action ? +1 : -1;
							o.lots += mult * next.lots;
							sort = true;
							if (o.lots < 0) {
								o.lots *= -1;
								o.action = !o.action;
							}
						}
					}
					else {
						if (next.end == o.end) {
							o.end = next.begin;
							ASSERT(o.begin < o.end);
							ASSERT(next.begin < next.end);
							int mult = o.action == next.action ? +1 : -1;
							next.lots += mult * o.lots;
							if (next.lots < 0) {
								next.lots *= -1;
								next.action = !next.action;
							}
						}
						else {
							if (o.end < next.end) {
								Order& intersect = a.history_orders.Insert(k);
								ASSERT(o.begin < o.end);
								ASSERT(next.begin < next.end);
								ASSERT(next.begin < o.end);
								ASSERT(o.begin < next.begin);
								intersect.begin = next.begin;
								intersect.end = o.end;
								next.begin = intersect.end;
								o.end = intersect.begin;
								ASSERT(o.begin < o.end);
								ASSERT(intersect.begin < intersect.end);
								ASSERT(next.begin < next.end);
								
								intersect.symbol = o.symbol;
								intersect.lots = o.lots;
								intersect.action = o.action;
								int mult = o.action == next.action ? +1 : -1;
								intersect.lots += mult * next.lots;
								
								if (intersect.lots < 0) {
									intersect.lots *= -1;
									intersect.action = !intersect.action;
								}
								sort = true;
								dec = true;
								break;
							} else {
								Order& trail = a.history_orders.Insert(k+1);
								trail.begin = next.end;
								trail.end = o.end;
								o.end = next.begin;
								
								trail.symbol = o.symbol;
								trail.lots = o.lots;
								trail.action = o.action;
								
								int mult = o.action == next.action ? +1 : -1;
								next.lots += mult * o.lots;
								if (next.lots < 0) {
									next.lots *= -1;
									next.action = !next.action;
								}
								sort = true;
							}
						}
					}
				}
				
				
				if (rem) {
					a.history_orders.Remove(k);
					k--;
				}
			}
			
			if (sort) {
				Sort(a.history_orders, Order());
			}
			
			if (o.lots == 0) {
				a.history_orders.Remove(j);
				j--;
			}
			else if (dec) {
				j--;
			}
		}
	}
}

void MyfxbookCommon::RefreshHistory() {
	String cache_dir = ConfigFile("cache");
	RealizeDirectory(cache_dir);
	
	VectorMap<String, int> symbol_list;
	
	for(int i = 0; i < accounts.GetCount(); i++) {
		Account& a = accounts[i];
		LOG("Account " << i << ": " << a.id);
		ReleaseLog("Myfxbook::RefreshHistory Account " + IntStr(i) + ": " + a.id);
		
		a.history_orders.Clear();
		
		for (int page = 1; page < 1000; page++) {
			
			//String url = "https://www.myfxbook.com/paging.html?pt=4&p=" + IntStr(page) + "&ts=29&&l=x&id=" + a.id + "&invitation=&start=2015-05-18%2000:00&end=&sb=27&st=1&symbols=&magicNumbers=&types=0,1,2,4,19,5&orderTagList=&daysList=&hoursList=&buySellList=&yieldStart=&yieldEnd=&netProfitStart=&netProfitEnd=&durationStart=&durationEnd=&takeProfitStart=&takeProfitEnd=&stopLoss=&stopLossEnd=&sizingStart=&sizingEnd=&selectedTime=&pipsStart=&pipsEnd=";
			String url = "https://www.myfxbook.com/paging.html?pt=4&p=" + IntStr(page) + "&ts=105&&id=" + a.id + "&l=a&invitation=&start=2015-05-18%2000:00&end=&sb=28&st=2&magicNumbers=&symbols=&types=0,1,2,4,19,5&orderTagList=&daysList=&hoursList=&buySellList=&yieldStart=&yieldEnd=&netProfitStart=&netProfitEnd=&durationStart=&durationEnd=&takeProfitStart=&takeProfitEnd=&stopLoss=&stopLossEnd=&sizingStart=&sizingEnd=&selectedTime=&pipsStart=&pipsEnd=&ts=105&z=0.6986965124862867";
			
			String filename = IntStr(url.GetHashValue()) + ".html";
			String filepath = AppendFileName(cache_dir, filename);
			String xmlname = IntStr(url.GetHashValue()) + ".xml";
			String xmlpath = AppendFileName(cache_dir, xmlname);
			
			String xml;
			for (int i = 0; i < 2; i++) {
				if (!FileExists(filepath)) {
					LOG(url);
					HttpRequest h;
					BasicHeaders(h);
					h.Url(url);
					String content = h.Execute();
					content.Replace("data='", "data=''>");
					
					FileOut fout(filepath);
					fout << content;
					fout.Close();
					
					Sleep(500);
				}
				
				if (!FileExists(xmlpath)) {
					Tidy(xmlpath, filepath);
				}
				
				xml = LoadFile(xmlpath);
				if (xml.IsEmpty()) {
					ReleaseLog("Myfxbook::RefreshHistory retrying " + url);
					DeleteFile(filepath);
					DeleteFile(xmlpath);
				}
				else break;
			}
			if (xml.Find("No data to") != -1)
				break;
			
			XmlFix(xml);
			//LOG(xml);
			
			XmlNode xn = ParseXML(xml);
			//LOG(XmlTreeString(xn));
			
			
			int errorcode = 0;
			const XmlNode& rows = TryOpenLocation("0 1 0 1", xn, errorcode);
			
			for(int j = 1; j < rows.GetCount(); j++) {
				const XmlNode& row = rows[j];
				//LOG(XmlTreeString(row));
				
				for (int s = 0; s < 2; s++) {
					String action = row[6-s][0].GetText();
					
					if (action == "Sell" || action == "Buy") {
						Order& o = a.history_orders.Add();
						
						StrTime(o.begin, row[2-s][0].GetText());
						StrTime(o.end, row[4-s][0].GetText());
						o.symbol = row[5-s][0][0].GetText();
						o.action = action == "Sell";
						o.open   = row[9-s][0][0].GetText();
						o.profit = row[9-s][2][0].GetText();
						o.lots   = StrDbl(row[7-s][0].GetText());
						
						symbol_list.GetAdd(o.symbol, 0)++;
						break;
					}
				}
			}
		}
		
		LOG("   trades " << a.history_orders.GetCount());
	}
	
	SortByValue(symbol_list, StdGreater<int>());
	DUMPM(symbol_list);
}

void MyfxbookCommon::RefreshOpen() {
	System& sys = GetSystem();
	ReleaseLog("Myfxbook::RefreshOpen");
	
	
	for(int i = 0; i < symbols.GetCount(); i++)
		symbols[i].lots_mult = 0.0;
	
	#if 0
	for(int i = 0; i < accounts.GetCount() && running; i++) {
		Account& a = accounts[i];
		if (a.id == "2475021") {
			RefreshAccountOpen(i);
			active_account = i;
			break;
		}
	}
	#else
	if (active_account == -1) {
		for(int i = 0; i < accounts.GetCount() && running; i++) {
			if (RefreshAccountOpen(i)) {
				active_account = i;
				break;
			}
		}
	} else {
		RefreshAccountOpen(active_account);
		Account& a = accounts[active_account];
		if (a.orders.IsEmpty()) {
			active_account = -1;
		}
	}
	#endif
	
	double max_lots = 0.0;
	for(int i = 0; i < symbols.GetCount(); i++)
		max_lots = max(max_lots, fabs(symbols[i].lots_mult));
	if (max_lots > 0.0)
		for(int i = 0; i < symbols.GetCount(); i++)
			symbols[i].lots_mult /= max_lots;
	
	double lots_max = 1.0 / MAX_SYMOPEN;
	
	SortByValue(symbols, SymbolStats());
	DUMPM(symbols);
	
	for (int i = 0; i < symbols.GetCount(); i++) {
		SymbolStats& s = symbols[i];
		
		if (s.lots_mult > lots_max)
			s.lots_mult = lots_max;
		
		int new_signal = s.lots_mult * SIGNALSCALE;
		
		// Don't switch to opposite signal without zeroing first
		//if (s.signal != 0 && s.signal != new_signal)
		//	s.wait = true;
		
		s.signal = new_signal;
		
		if (s.signal == 0)
			s.wait = false;
		//if (s.wait)
		//	s.signal = 0;
		
		int id = sys.FindSymbol(s.symbol);
		ReleaseLog("Set real signal " + IntStr(id) + " to " + IntStr(s.signal));
		sys.SetSignal(id, s.signal);
	}
	
	
	latest_update = GetSysTime();
}

bool MyfxbookCommon::RefreshAccountOpen(int i) {
	Account& a = accounts[i];
	VectorMap<String, double> account_symlots;
	
	if (!(a.profitability > 0.50 && a.gain > 0.0 && a.pips > 0.0))
		return false;
	
	bool new_orders = a.orders.IsEmpty();
	bool is_active = i == active_account;
	
	a.orders.Clear();
	
	String cache_dir = ConfigFile("cache");
	
	for (int page = 1; page < 1000 && running; page++) {
		
		String url = "https://www.myfxbook.com/paging.html?pt=15&p=" + IntStr(page) + "&ts=29&&l=x&id=" + a.id + "&invitation=&start=2015-05-18%2000:00&end=&sb=27&st=1&symbols=&magicNumbers=&types=0,1,2,4,19,5&orderTagList=&daysList=&hoursList=&buySellList=&yieldStart=&yieldEnd=&netProfitStart=&netProfitEnd=&durationStart=&durationEnd=&takeProfitStart=&takeProfitEnd=&stopLoss=&stopLossEnd=&sizingStart=&sizingEnd=&selectedTime=&pipsStart=&pipsEnd=";
		//String url = "https://www.myfxbook.com/paging.html?pt=15&p=" + IntStr(page) + "&ts=29&&l=x&id=2419864&invitation=&start=2015-05-18%2000:00&end=&sb=27&st=1&symbols=&magicNumbers=&types=0,1,2,4,19,5&orderTagList=&daysList=&hoursList=&buySellList=&yieldStart=&yieldEnd=&netProfitStart=&netProfitEnd=&durationStart=&durationEnd=&takeProfitStart=&takeProfitEnd=&stopLoss=&stopLossEnd=&sizingStart=&sizingEnd=&selectedTime=&pipsStart=&pipsEnd=";
		
		LOG(url);
		String filename = IntStr(url.GetHashValue()) + ".html";
		String filepath = AppendFileName(cache_dir, filename);
		String xmlname = IntStr(url.GetHashValue()) + ".xml";
		String xmlpath = AppendFileName(cache_dir, xmlname);
		
		HttpRequest h;
		BasicHeaders(h);
		h.Url(url);
		String content = h.Execute();
		FileOut fout(filepath);
		fout << content;
		fout.Close();
		
		Tidy(xmlpath, filepath);
		
		Sleep(500);
		
		
		String xml = LoadFile(xmlpath);
		
		
		if (xml.Find("No data to") != -1)
			break;
		
		XmlFix(xml);
		//LOG(xml);
		
		XmlNode xn;
		try {
			xn = ParseXML(xml);
		}
		catch (XmlError e) {
			ReleaseLog("Myfxbook::RefreshOpen XML PARSE ERROR");
		}
		//LOG(XmlTreeString(xn));
		
		int errorcode = 0;
		const XmlNode& rows = TryOpenLocation("0 1 0 0", xn, errorcode);
		
		int items = rows.GetCount() - 2;
		for(int j = 1; j < rows.GetCount(); j++) {
			const XmlNode& row = rows[j];
			
			for (int s = 0; s < 2; s++) {
				String action = row[4-s][0].GetText();
				
				if (action == "Sell" || action == "Buy") {
					Order& o = a.orders.Add();
					
					StrTime(o.begin, row[1-s][0].GetText());
					o.symbol = row[3-s][0][0].GetText();
					o.action = action == "Sell";
					o.open   = row[6-s][0].GetText();
					o.profit = row[9-s][0][0].GetText();
					o.lots = StrDbl(row[5-s][0].GetText());
					
					double mult = action == "Sell" ? -1 : +1;
					
					account_symlots.GetAdd(o.symbol, 0.0) += mult * o.lots;
					
					//LOG(XmlTreeString(row));
					break;
				}
			}
		}
		
		if (items < 20) {
			LOG("page " << page << " items " << items);
			break;
		}
	}
	
	DUMPM(account_symlots);
	
	if (is_active || new_orders) {
		double max_lots = 0.0;
		for(int j = 0; j < account_symlots.GetCount(); j++) {
			double lots = fabs(account_symlots[j]);
			if (lots > max_lots)
				max_lots = lots;
		}
		
		for(int j = 0; j < account_symlots.GetCount(); j++) {
			String symbol = account_symlots.GetKey(j);
			double lots = account_symlots[j];
			int l = symbols.Find(symbol);
			if (l != -1) {
				symbols[l].lots_mult += lots / max_lots;
			}
		}
		
		
		// To use only one account
		return max_lots > 0.0;
	}
	else return false;
}

























MyfxbookCtrl::MyfxbookCtrl() {
	Ctrl::Add(splitter.SizePos());
	
	splitter.Horz();
	splitter << valuelist << accountlist << orderlist << historylist;
	
	valuelist <<= THISBACK(Data);
	accountlist <<= THISBACK(Data);
	
	valuelist.AddColumn("Key");
	valuelist.AddColumn("Value");
	
	accountlist.AddColumn("Id");
	accountlist.AddColumn("Profitability");
	accountlist.AddColumn("Pips");
	accountlist.AddColumn("Gain");
	accountlist.AddColumn("Av Gain");
	
	orderlist.AddColumn("Open");
	orderlist.AddColumn("Symbol");
	orderlist.AddColumn("Action");
	orderlist.AddColumn("Units");
	orderlist.AddColumn("Open");
	orderlist.AddColumn("Profit");
	
	historylist.AddColumn("Open");
	historylist.AddColumn("Close");
	historylist.AddColumn("Symbol");
	historylist.AddColumn("Action");
	historylist.AddColumn("Units");
	historylist.AddColumn("Open");
	historylist.AddColumn("Profit");
	
}

void MyfxbookCtrl::Data() {
	MyfxbookCommon& m = GetMyfxbook();
	
	int valuecursor = valuelist.GetCursor();
	
	valuelist.Set(0, 0, "Latest update");
	valuelist.Set(0, 1, m.latest_update);
	for(int i = 0; i < m.symbols.GetCount(); i++) {
		MyfxbookCommon::SymbolStats& s = m.symbols[i];
		valuelist.Set(1+i, 0, s.symbol);
		valuelist.Set(1+i, 1, s.signal);
	}
	
	int a_id = -1;
	
	for(int i = 0; i < m.accounts.GetCount(); i++) {
		MyfxbookCommon::Account& a = m.accounts[i];
		accountlist.Set(i, 0, a.id);
		accountlist.Set(i, 1, a.profitability);
		accountlist.Set(i, 2, a.pips);
		accountlist.Set(i, 3, a.gain);
		accountlist.Set(i, 4, a.av_gain);
	}
	
	a_id = accountlist.GetCursor();
	
	
	if (a_id >= 0 && a_id < m.accounts.GetCount()) {
		MyfxbookCommon::Account& a = m.accounts[a_id];
		
		for(int i = 0; i < a.orders.GetCount(); i++) {
			MyfxbookCommon::Order& o = a.orders[i];
			orderlist.Set(i, 0, o.begin);
			orderlist.Set(i, 1, o.symbol);
			orderlist.Set(i, 2, o.action);
			orderlist.Set(i, 3, o.lots);
			orderlist.Set(i, 4, o.open);
			orderlist.Set(i, 5, o.profit);
		}
		orderlist.SetCount(a.orders.GetCount());
		
		for(int i = 0; i < a.history_orders.GetCount(); i++) {
			MyfxbookCommon::Order& o = a.history_orders[i];
			historylist.Set(i, 0, o.begin);
			historylist.Set(i, 1, o.end);
			historylist.Set(i, 2, o.symbol);
			historylist.Set(i, 3, o.action);
			historylist.Set(i, 4, o.lots);
			historylist.Set(i, 5, o.open);
			historylist.Set(i, 6, o.profit);
		}
		historylist.SetCount(a.history_orders.GetCount());
		
	}

}

}
