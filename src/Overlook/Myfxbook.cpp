#include "Overlook.h"
#include <plugin/tidy/tidy.h>

namespace Overlook {

Myfxbook::Myfxbook() {
	allowed_symbols.Add("EURUSD");
	allowed_symbols.Add("GBPUSD");
	allowed_symbols.Add("GBPJPY");
	allowed_symbols.Add("USDJPY");
	allowed_symbols.Add("AUDUSD");
	allowed_symbols.Add("USDCAD");
	allowed_symbols.Add("EURJPY");
	allowed_symbols.Add("USDCHF");
	allowed_symbols.Add("EURGBP");
	allowed_symbols.Add("NZDUSD");
	allowed_symbols.Add("EURAUD");
	allowed_symbols.Add("AUDNZD");
	allowed_symbols.Add("AUDCAD");
	allowed_symbols.Add("EURCAD");
	allowed_symbols.Add("GBPCHF");
	allowed_symbols.Add("CADJPY");
	
	
	Ctrl::Add(splitter.SizePos());
	
	splitter.Horz();
	splitter << accountlist << orderlist << historylist;
	
	accountlist <<= THISBACK(Data);
	
	accountlist.AddColumn("Id");
	
	orderlist.AddColumn("Time");
	orderlist.AddColumn("Symbol");
	orderlist.AddColumn("Action");
	orderlist.AddColumn("Units");
	orderlist.AddColumn("Open");
	orderlist.AddColumn("Profit");
	
	historylist.AddColumn("Time");
	historylist.AddColumn("Symbol");
	historylist.AddColumn("Action");
	historylist.AddColumn("Units");
	historylist.AddColumn("Open");
	historylist.AddColumn("Profit");
	
	/*
		What to look at in accounts:
			- is verified "Track Record Verified" & "Trading Privileges Verified"
			- abs gain enough, profitability enough
			- no significant drops in growth chart, or at least lately
				- growth graph equity shouldn't diverge much from balance
			- green week, month, year, history list overall
			- pips
			- Avg. Trade Length around 4-12 hours
				- lengthy orders (M5 update interval should keep close enough)
				- NO arbitrage systems
			- no long open trades (postponing closing negative)
			- long history (OR real account with high equity)
			- no too expensive or weird symbols
			- no weird short orders with some very long orders
			
	*/
																					// profitability, abs gain
	Add("https://www.myfxbook.com/members/katamike/bravepointgettereurusd/2529001",					100, 184);
	Add("https://www.myfxbook.com/members/chainniji/c-project/2515913",								100, 160);
	Add("https://www.myfxbook.com/members/strawbellytiger/gps-forex-robot/2315367",					100,  79);
	Add("https://www.myfxbook.com/members/a40datsusara/%E4%B8%8A%E3%81%8C%E3%82%8A3%E3%83%8F%E3%83%AD%E3%83%B3/2437366", 100, 49);
	Add("https://www.myfxbook.com/members/SPAROBOT/said-taihou/2402325",							98,  95);
	Add("https://www.myfxbook.com/members/ForexMark/gps-forex-robot-fxchoice/301639",				96, 178);
	Add("https://www.myfxbook.com/members/ForexMark/gps-robot-fxchoice-1mio/1213851",				96,  39);
	Add("https://www.myfxbook.com/members/frugalist/ea-cutter-eurusd/2514101",						95,  39);
	Add("https://www.myfxbook.com/members/ForexMark/gps-robot-fxchoice-1mio-tr/1355807",			94, 176);
	Add("https://www.myfxbook.com/members/ForexMark/gps-robot-fxchoice-100k/396026",				93, 443);
	Add("https://www.myfxbook.com/members/EDROID/the-retirement-plan-2sangevt-/2494726",			93,  73);
	Add("https://www.myfxbook.com/members/DenVP/den-hamst4/2452457",								93,  51);
	Add("https://www.myfxbook.com/members/LuckForex/luckforex/2536294",								92,  18);
	Add("https://www.myfxbook.com/members/C7strategies/c7trolls/2508637",							91,   7);
	Add("https://www.myfxbook.com/members/SmartForexTrader/profit-miner/2360409",					89, 251);
	Add("https://www.myfxbook.com/members/skgchshnjmfb/khs-synpa-titan/2437409",					88,  41);
	Add("https://www.myfxbook.com/members/volk3888/volk3888/2499590",								87, 152);
	Add("https://www.myfxbook.com/members/SmartForexTrader/smart-pip/2266006",						85, 156);
	Add("https://www.myfxbook.com/members/katamike/katamike/2447680",								84,  91);
	Add("https://www.myfxbook.com/members/chainniji/dt-reborn/2489118",								82,  52);
	Add("https://www.myfxbook.com/members/goldenstrike/ninja-turtle/2186338",						82,  22);
	Add("https://www.myfxbook.com/members/hayes/t1-chuinhooi-9909922/2338673",						81, 136);
	Add("https://www.myfxbook.com/members/forexwallstreet/wallstreet-20-evolution-real/2498506",	81,  63);
	Add("https://www.myfxbook.com/members/JackDaniil/safety-low-risk/2199095",						81,  41);
	Add("https://www.myfxbook.com/members/GeorgeDow/gdow/1549874",									74, 488);
	Add("https://www.myfxbook.com/members/Bosko/my-way/2406456",									71, 275);
	// Continue from https://www.myfxbook.com/systems#?pt=6&p=5&ts=346&profitType=0&profitValue=0.0&drawType=1&drawValue=50.0&profitabilityType=0&profitabilityValue=80.0&ageType=0&ageValue=30&tradingType=0&systemType=0&symbols=&accountType=2&size=40&sb=19&st=2&lastTraded=90&tradesType=1&pipsType=1&pipsValue=30&equityType=1&equityValue=30&serverOid=0&regulationType=0
	
	Thread::Start(THISBACK(Updater));
}

Myfxbook::~Myfxbook() {
	running = false;
	while (!stopped) Sleep(100);
}

void Myfxbook::Updater() {
	running = true;
	stopped = false;
	
	accounts.SetCount(urls.GetCount());
	for(int i = 0; i < accounts.GetCount(); i++) {
		Account& a = accounts[i];
		a.url = urls[i];
		a.id = a.url.Mid(a.url.ReverseFind("/") + 1);
	}
	
	RefreshHistory();
	
	while (running) {
		TimeStop ts;
		
		Time now = GetUtcTime();
		int wday = DayOfWeek(now);
		
		if (wday >= 1 && wday <= 5)
			RefreshOpen();
		
		int sec = ts.Elapsed() / 1000;
		for(int i = sec; i < 60 && running; i++) {
			Sleep(1000);
		}
	}
	
	stopped = true;
}

void Myfxbook::RefreshHistory() {
	String cache_dir = ConfigFile("cache");
	
	VectorMap<String, int> symbol_list;
	
	for(int i = 0; i < accounts.GetCount(); i++) {
		Account& a = accounts[i];
		LOG("Account " << i << ": " << a.id);
		
		a.history_orders.Clear();
		
		for (int page = 1; page < 100; page++) {
			
			String url = "https://www.myfxbook.com/paging.html?pt=4&p=" + IntStr(page) + "&ts=29&&l=x&id=" + a.id + "&invitation=&start=2015-05-18%2000:00&end=&sb=27&st=1&symbols=&magicNumbers=&types=0,1,2,4,19,5&orderTagList=&daysList=&hoursList=&buySellList=&yieldStart=&yieldEnd=&netProfitStart=&netProfitEnd=&durationStart=&durationEnd=&takeProfitStart=&takeProfitEnd=&stopLoss=&stopLossEnd=&sizingStart=&sizingEnd=&selectedTime=&pipsStart=&pipsEnd=";
			
			String filename = IntStr(url.GetHashValue()) + ".html";
			String filepath = AppendFileName(cache_dir, filename);
			String xmlname = IntStr(url.GetHashValue()) + ".xml";
			String xmlpath = AppendFileName(cache_dir, xmlname);
			
			if (!FileExists(filepath)) {
				LOG(url);
				HttpRequest h;
				BasicHeaders(h);
				h.Url(url);
				String content = h.Execute();
				FileOut fout(filepath);
				fout << content;
				fout.Close();
				
				Tidy(xmlpath, filepath);
				
				Sleep(500);
			}
			
			
			String xml = LoadFile(xmlpath);
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
						
						o.time   = row[1-s][0].GetText();
						o.symbol = row[5-s][0][0].GetText();
						o.action = action;
						o.open   = row[9-s][0][0].GetText();
						o.profit = row[9-s][2][0].GetText();
						
						symbol_list.GetAdd(o.symbol, 0)++;
					}
				}
			}
		}
		
		LOG("   trades " << a.history_orders.GetCount());
	}
	
	SortByValue(symbol_list, StdGreater<int>());
	DUMPM(symbol_list);
}

void Myfxbook::RefreshOpen() {
	String cache_dir = ConfigFile("cache");
	
	VectorMap<String, int> sym_signals;
	
	for(int i = 0; i < accounts.GetCount() && running; i++) {
		Account& a = accounts[i];
		
		a.orders.Clear();
		
		VectorMap<String, double> account_symlots;
		
		for (int page = 1; page < 100 && running; page++) {
			
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
			
			XmlNode xn = ParseXML(xml);
			//LOG(XmlTreeString(xn));
			
			int errorcode = 0;
			const XmlNode& rows = TryOpenLocation("0 1 0 0", xn, errorcode);
			
			int items = rows.GetCount() - 2;
			for(int j = 1; j < rows.GetCount(); j++) {
				const XmlNode& row = rows[j];
				String action = row[4][0].GetText();
				
				if (action == "Sell" || action == "Buy") {
					Order& o = a.orders.Add();
					
					o.time   = row[1][0].GetText();
					o.symbol = row[3][0][0].GetText();
					o.action = action;
					o.units  = row[5][0].GetText();
					o.open   = row[6][0].GetText();
					o.profit = row[9][0][0].GetText();
					o.lots = StrDbl(o.units);
					
					double mult = action == "Sell" ? -1 : +1;
					
					account_symlots.GetAdd(o.symbol, 0.0) += mult * o.lots;
					
					//LOG(XmlTreeString(row));
				}
			}
			
			if (items < 20) {
				LOG("page " << page << " items " << items);
				break;
			}
		}
		
		DUMPM(account_symlots);
		for(int j = 0; j < account_symlots.GetCount(); j++) {
			String sym = account_symlots.GetKey(j);
			if (allowed_symbols.Find(sym) == -1)
				continue;
			
			int& sig = sym_signals.GetAdd(sym, 0);
			if (sig != 0)
				continue;
			
			double lots = account_symlots[j];
			if (lots == 0)
				continue;
			
			sig = lots > 0 ? +1 : -1;
		}
	}
	
	DUMPM(sym_signals);
	
	System& sys = GetSystem();
	for(int i = 0; i < sym_signals.GetCount(); i++) {
		String key = sym_signals.GetKey(i);
		int sym = sys.FindSymbol(key);
		ASSERT(sym != -1);
		sys.SetSignal(sym, sym_signals[i]);
	}
	
}


void Myfxbook::Data() {
	
	for(int i = 0; i < accounts.GetCount(); i++) {
		Account& a = accounts[i];
		accountlist.Set(i, 0, a.id);
	}
	
	int cursor = accountlist.GetCursor();
	if (cursor >= 0 && cursor < accounts.GetCount()) {
		Account& a = accounts[cursor];
		
		for(int i = 0; i < a.orders.GetCount(); i++) {
			Order& o = a.orders[i];
			orderlist.Set(i, 0, o.time);
			orderlist.Set(i, 1, o.symbol);
			orderlist.Set(i, 2, o.action);
			orderlist.Set(i, 3, o.units);
			orderlist.Set(i, 4, o.open);
			orderlist.Set(i, 5, o.profit);
		}
		orderlist.SetCount(a.orders.GetCount());
		
		for(int i = 0; i < a.history_orders.GetCount(); i++) {
			Order& o = a.history_orders[i];
			historylist.Set(i, 0, o.time);
			historylist.Set(i, 1, o.symbol);
			historylist.Set(i, 2, o.action);
			historylist.Set(i, 3, o.units);
			historylist.Set(i, 4, o.open);
			historylist.Set(i, 5, o.profit);
		}
		historylist.SetCount(a.history_orders.GetCount());
		
	}
	
}

}
