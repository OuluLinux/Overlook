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

Myfxbook::Myfxbook() {
	System& sys = GetSystem();
	
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
	
	
	LoadThis();
	
	if (symbols.IsEmpty()) {
		symbols.Add("EURUSD");
		symbols.Add("GBPUSD");
		#ifndef flagDEBUG
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
		#endif
		
		for(int i = 0; i < symbols.GetCount(); i++) {
			SymbolStats& s = symbols[i];
			s.symbol = symbols.GetKey(i);
			s.id = sys.FindSymbol(s.symbol);
			if (s.id == -1)
				Panic("Symbol " + s.symbol + " wasn't found from system");
		}
		
		Add("https://www.myfxbook.com/members/katamike/bravepointgettereurusd/2529001",					100, 184);
		Add("https://www.myfxbook.com/members/chainniji/c-project/2515913",								100, 160);
		Add("https://www.myfxbook.com/members/strawbellytiger/gps-forex-robot/2315367",					100,  79);
		Add("https://www.myfxbook.com/members/a40datsusara/%E4%B8%8A%E3%81%8C%E3%82%8A3%E3%83%8F%E3%83%AD%E3%83%B3/2437366", 100, 49);
		Add("https://www.myfxbook.com/members/SPAROBOT/said-taihou/2402325",							98,  95);
		Add("https://www.myfxbook.com/members/zevenday/mmc1-fixed-lot-everyday/2508275",				97,  15);
		Add("https://www.myfxbook.com/members/ForexMark/gps-forex-robot-fxchoice/301639",				96, 178);
		Add("https://www.myfxbook.com/members/ForexMark/gps-robot-fxchoice-1mio/1213851",				96,  39);
		Add("https://www.myfxbook.com/members/frugalist/ea-cutter-eurusd/2514101",						95,  39);
		Add("https://www.myfxbook.com/members/ForexMark/gps-robot-fxchoice-1mio-tr/1355807",			94, 176);
		Add("https://www.myfxbook.com/members/ForexMark/gps-robot-fxchoice-100k/396026",				93, 443);
		Add("https://www.myfxbook.com/members/EDROID/the-retirement-plan-2sangevt-/2494726",			93,  73);
		Add("https://www.myfxbook.com/members/DenVP/den-hamst4/2452457",								93,  51);
		Add("https://www.myfxbook.com/members/LuckForex/luckforex/2536294",								92,  18);
		Add("https://www.myfxbook.com/members/C7strategies/c7trolls/2508637",							91,   7);
		Add("https://www.myfxbook.com/members/Ludopatico/ciccio-eur/2421614",							90,  21);
		Add("https://www.myfxbook.com/members/SmartForexTrader/profit-miner/2360409",					89, 251);
		Add("https://www.myfxbook.com/members/skgchshnjmfb/khs-synpa-titan/2437409",					88,  41);
		Add("https://www.myfxbook.com/members/volk3888/volk3888/2499590",								87, 152);
		Add("https://www.myfxbook.com/members/SmartForexTrader/smart-pip/2266006",						85, 156);
		Add("https://www.myfxbook.com/members/KenjiMiyazaki/ea-imitate006/2507325",						85,  39);
		Add("https://www.myfxbook.com/members/ywhcg805/gaitame-1/2486823",								85,  25);
		Add("https://www.myfxbook.com/members/katamike/katamike/2447680",								84,  91);
		Add("https://www.myfxbook.com/members/chainniji/dt-reborn/2489118",								82,  52);
		Add("https://www.myfxbook.com/members/goldenstrike/ninja-turtle/2186338",						82,  22);
		Add("https://www.myfxbook.com/members/hayes/t1-chuinhooi-9909922/2338673",						81, 136);
		Add("https://www.myfxbook.com/members/forexwallstreet/wallstreet-20-evolution-real/2498506",	81,  63);
		Add("https://www.myfxbook.com/members/JackDaniil/safety-low-risk/2199095",						81,  41);
		Add("https://www.myfxbook.com/members/GeorgeDow/gdow/1549874",									74, 488);
		Add("https://www.myfxbook.com/members/Bosko/my-way/2406456",									71, 275);
		
		Add("https://www.myfxbook.com/members/globalFXteam/inclusivefx-1/2127487", 0, 0);
		Add("https://www.myfxbook.com/members/globalFXteam/inclusivefx-2/2127462", 0, 0);
		Add("https://www.myfxbook.com/members/Matt098/mattpamm/1739314", 0, 0);
		Add("https://www.myfxbook.com/members/globalFXteam/inclusivefx-3/2127467", 0, 0);
		Add("https://www.myfxbook.com/members/AI4Forex/ai4forex-extremely-aggressive/2457740", 0, 0);
		Add("https://www.myfxbook.com/members/flok/vranjevcan/2496214", 0, 0);
		Add("https://www.myfxbook.com/members/podzhigai/ecn-pro-tickmill/1749899", 0, 0);
		Add("https://www.myfxbook.com/members/Al_En/al-en/1751076", 0, 0);
		Add("https://www.myfxbook.com/members/danntrader/aguilarcasanova/2420608", 0, 0);
		Add("https://www.myfxbook.com/members/iBestForexRobot/ibestforexrobot/1686973", 0, 0);
		Add("https://www.myfxbook.com/members/kucindacat/forexakademi/2511543", 0, 0);
		Add("https://www.myfxbook.com/members/Manul/worldforex-10/2474401", 0, 0);
		Add("https://www.myfxbook.com/members/cornic/quatrefoil-2015-2017/2082591", 0, 0);
		Add("https://www.myfxbook.com/members/Lazard/high-risk-blackwave-alpine/1302652", 0, 0);
		Add("https://www.myfxbook.com/members/skillforex/skillfx-mix-kzm-xm/2476170", 0, 0);
		Add("https://www.myfxbook.com/members/TradersLife77/traderslife77/1863274", 0, 0);
		Add("https://www.myfxbook.com/members/m1800/fxopen-533478/718852", 0, 0);
		Add("https://www.myfxbook.com/members/quangforex/tickmill-66914/2346470", 0, 0);
		Add("https://www.myfxbook.com/members/Stonecore/stonecore-system-2/2232714", 0, 0);
		Add("https://www.myfxbook.com/members/sonthisak/4x4rad-o32174446/2521633", 0, 0);
		Add("https://www.myfxbook.com/members/Vryzafx/vryzafx003/2508129", 0, 0);
		Add("https://www.myfxbook.com/members/Matt098/matthewacc/2442630", 0, 0);
		Add("https://www.myfxbook.com/members/AlekseyB2014/forex-warrior-v902/2441905", 0, 0);
		Add("https://www.myfxbook.com/members/SolFx/trufx/2099921", 0, 0);
		Add("https://www.myfxbook.com/members/Flash_Trader/flash/2273174", 0, 0);
		Add("https://www.myfxbook.com/members/2088952393/maxprofit/1283790", 0, 0);
		Add("https://www.myfxbook.com/members/adisbv/forexrealprofitea2/1242167", 0, 0);
		Add("https://www.myfxbook.com/members/IceFXMarkets/polar5/1647989", 0, 0);
		Add("https://www.myfxbook.com/members/IceFXMarkets/polar5/1647989", 0, 0);
		Add("https://www.myfxbook.com/members/quangforex/tickmill-63322/2246051", 0, 0);
		Add("https://www.myfxbook.com/members/Yuliya_Z/get-lucky/2420203", 0, 0);
		Add("https://www.myfxbook.com/members/inishiefx/inishie-shihyou5/2527892", 0, 0);
		Add("https://www.myfxbook.com/members/arpan366/pps-eventstrategy-live/2419114", 0, 0);
		Add("https://www.myfxbook.com/members/ala7000/grid-ea/2228943", 0, 0);
		Add("https://www.myfxbook.com/members/OTMCapital/otm-capital-hft/2070059", 0, 0);
		Add("https://www.myfxbook.com/members/serg7800/stream/2535373", 0, 0);
		Add("https://www.myfxbook.com/members/bankworapob/jarvis-27317/2048317", 0, 0);
		Add("https://www.myfxbook.com/members/fxtrenddetector/forex-trend-detector/1280220", 0, 0);
		Add("https://www.myfxbook.com/members/KOTMAX/robobonus/646274", 0, 0);
		Add("https://www.myfxbook.com/members/EOSFX/eos-mid-risk/1765327", 0, 0);
		Add("https://www.myfxbook.com/members/manager_mgmforex/3-2015/2335878", 0, 0);
		Add("https://www.myfxbook.com/members/autotrade/sfe-price-action/1331484", 0, 0);
		Add("https://www.myfxbook.com/members/IceFXMarkets/polar4/1647987", 0, 0);
		Add("https://www.myfxbook.com/members/miin/min/2455275", 0, 0);
		Add("https://www.myfxbook.com/members/OLGA68/3007200/2489205", 0, 0);
		Add("https://www.myfxbook.com/members/KillerOnPips/aquamarine/2286922", 0, 0);
		Add("https://www.myfxbook.com/members/jasma23/genstar-1/2039592", 0, 0);
		Add("https://www.myfxbook.com/members/nornx/nornx-lean-trader/2505233", 0, 0);
		Add("https://www.myfxbook.com/members/GloryForex/fxtsignal/2090294", 0, 0);
		Add("https://www.myfxbook.com/members/Savagea/%E4%B8%89%E6%B5%81%E7%90%86%E6%83%B3%E5%AE%B6/2508451", 0, 0);
		Add("https://www.myfxbook.com/members/EtWan/ascension-high-risk/1875772", 0, 0);
		Add("https://www.myfxbook.com/members/IceFXMarkets/polar4/1647987", 0, 0);
		Add("https://www.myfxbook.com/members/KillerOnPips/aquamarine/2286922", 0, 0);
		Add("https://www.myfxbook.com/members/jasma23/genstar-1/2039592", 0, 0);
		Add("https://www.myfxbook.com/members/nornx/nornx-lean-trader/2505233", 0, 0);
		Add("https://www.myfxbook.com/members/GloryForex/fxtsignal/2090294", 0, 0);
		Add("https://www.myfxbook.com/members/Savagea/%E4%B8%89%E6%B5%81%E7%90%86%E6%83%B3%E5%AE%B6/2508451", 0, 0);
		Add("https://www.myfxbook.com/members/csongor/uyscuti08/2534897", 0, 0);
		Add("https://www.myfxbook.com/members/EnNubis/ma-cross/2172440", 0, 0);
		Add("https://www.myfxbook.com/members/Mihail_Somov/fibo-nightscalper-ic-markets-real/1971418", 0, 0);
		Add("https://www.myfxbook.com/members/ProfiMaster/adamant/2298177", 0, 0);
		Add("https://www.myfxbook.com/members/Xaileng/protrader-micro/2487394", 0, 0);
		Add("https://www.myfxbook.com/members/FxPlyr/6387729-fbs-40k-real-crea/2433750", 0, 0);
		Add("https://www.myfxbook.com/members/xingtong825/%E7%BA%B5%E6%A8%AA%E5%A4%A9%E4%B8%8Bv168/1930174", 0, 0);
		Add("https://www.myfxbook.com/members/SmartForexTrader/smart-profits/2240602", 0, 0);
		Add("https://www.myfxbook.com/members/mollyred/molly-fsg-real/2499706", 0, 0);
		Add("https://www.myfxbook.com/members/katajikenai/opus-09/2336691", 0, 0);
		Add("https://www.myfxbook.com/members/autotrade/queen/1733312", 0, 0);
		Add("https://www.myfxbook.com/members/Soulofalion470/ea-fuzz-manage/2534203", 0, 0);
		Add("https://www.myfxbook.com/members/Paulgraham1/hedgehunter-ea-start-date-16/1762283", 0, 0);
		Add("https://www.myfxbook.com/members/hayes/t2-chuinhooi-9950300/2338680", 0, 0);
		Add("https://www.myfxbook.com/members/hayes/t2-chuinhooi-9950301/2338685", 0, 0);
		Add("https://www.myfxbook.com/members/checksignal/master-2k/2473000", 0, 0);
		Add("https://www.myfxbook.com/members/IceFXMarkets/polar3/1536005", 0, 0);
		Add("https://www.myfxbook.com/members/OLGA68/3007200/2489205", 0, 0);
		Add("https://www.myfxbook.com/members/coymahrens/mt4-930760/2092201", 0, 0);
		Add("https://www.myfxbook.com/members/Gustavo4X/553566-real/2454721", 0, 0);
		Add("https://www.myfxbook.com/members/hayes/t2-hayes1987-9950327/2338695", 0, 0);
		Add("https://www.myfxbook.com/members/smoki/sovenok/2297111", 0, 0);
		Add("https://www.myfxbook.com/members/lulipot/gio-ic-profit/907947", 0, 0);
		Add("https://www.myfxbook.com/members/Zamioculcas/cycle-ver/1794789", 0, 0);
		Add("https://www.myfxbook.com/members/wazhanudin/ea-leo-00/2532126", 0, 0);
		Add("https://www.myfxbook.com/members/autotrade/forexsmartprofit/1354782", 0, 0);
		Add("https://www.myfxbook.com/members/jcdmp/tickmill/2237014", 0, 0);
		Add("https://www.myfxbook.com/members/numanuel/ic-market-1000026402/2468499", 0, 0);
		Add("https://www.myfxbook.com/members/Aleksei3122/forex-factory-news-ea/2541329", 0, 0);
		Add("https://www.myfxbook.com/members/PeregrimEA/evonightea/2252381", 0, 0);
		Add("https://www.myfxbook.com/members/globalFXteam/inclusivefx-4/2438903", 0, 0);
		Add("https://www.myfxbook.com/members/strueli/momentum-eurusd-st1-h1-octafx/1559256", 0, 0);
		Add("https://www.myfxbook.com/members/hayes/t2-hayes727-9950778/2338697", 0, 0);
		Add("https://www.myfxbook.com/members/master_ice/pamm-amarkets/1295229", 0, 0);
		Add("https://www.myfxbook.com/members/vavatrade2/fx-43/1577993", 0, 0);
		Add("https://www.myfxbook.com/members/DALT/mzansi-forex-stockvel-90/1738001", 0, 0);
		Add("https://www.myfxbook.com/members/corni/night-scalper-portfolio/2175846", 0, 0);
		Add("https://www.myfxbook.com/members/MonsterKing/project-f/2482235", 0, 0);
		Add("https://www.myfxbook.com/members/myopportunity/ea-fusion-master-atr-316/2357869", 0, 0);
		Add("https://www.myfxbook.com/members/Pikarnight/system-ea/1750888", 0, 0);
		Add("https://www.myfxbook.com/members/Vasilii29RUS/pamm-mts/1716866", 0, 0);
		Add("https://www.myfxbook.com/members/mt4easystemtrade/khs-3000-s10/1825325", 0, 0);
		Add("https://www.myfxbook.com/members/Nomad2103/bfp-lrisk/2193307", 0, 0);
		Add("https://www.myfxbook.com/members/xplorer/xplorer/2402884", 0, 0);
		Add("https://www.myfxbook.com/members/hayes/t1-hayes727b-9951823/2338702", 0, 0);
		Add("https://www.myfxbook.com/members/HouseDr/%D1%82%D0%B5%D1%81%D1%82-%D0%B3%D1%80%D0%B0%D0%B0%D0%BB%D1%8F/2377377", 0, 0);
		Add("https://www.myfxbook.com/members/FXmagickey/alpha/2134976", 0, 0);
		Add("https://www.myfxbook.com/members/hayes/t2-hayes727a-9951228/2338700", 0, 0);
		Add("https://www.myfxbook.com/members/massdon/profitinvest/2327758", 0, 0);
		Add("https://www.myfxbook.com/members/akki38/akki38ti10/2238685", 0, 0);
		Add("https://www.myfxbook.com/members/hayes/t3-hayes727c-9952207/2338705", 0, 0);
		Add("https://www.myfxbook.com/members/iouslh/immortal/2426167", 0, 0);
		Add("https://www.myfxbook.com/members/LSegura/tna-fortfs/2515634", 0, 0);
		Add("https://www.myfxbook.com/members/cellist/at2/1755241", 0, 0);
		Add("https://www.myfxbook.com/members/Megabot/rapier-by-algoland/1657111", 0, 0);
		Add("https://www.myfxbook.com/members/MNSZA89/meor-v2-xm/2372712", 0, 0);
		Add("https://www.myfxbook.com/members/arnoldTT/ic-lnsc/2230705", 0, 0);
		Add("https://www.myfxbook.com/members/mt4easystemtrade/khs-3000-s30/1917803", 0, 0);
		Add("https://www.myfxbook.com/members/stratos/stratos1/1852228", 0, 0);
		Add("https://www.myfxbook.com/members/consultfinance/myalgotradecom/2446818", 0, 0);
		Add("https://www.myfxbook.com/members/ReVeR27/pricelessalpari/2349765", 0, 0);
		Add("https://www.myfxbook.com/members/Megabot/dissident-aggressor-by-algoland/1270220", 0, 0);
		Add("https://www.myfxbook.com/members/ATMBlueprint/servicepoint/2197952", 0, 0);
		Add("https://www.myfxbook.com/members/mt4easystemtrade/khs-3000-s20/1914305", 0, 0);
		Add("https://www.myfxbook.com/members/cheesybites1/hamilton/2446870", 0, 0);
		Add("https://www.myfxbook.com/members/JacksonCapital/k452b/1325330", 0, 0);
		Add("https://www.myfxbook.com/members/thewayofmoney/perfetto-ft/2128467", 0, 0);
		
		Add("https://www.myfxbook.com/members/manvarov/berndale-usd-1000000/2329847", 0, 0);
		Add("https://www.myfxbook.com/members/hieutv/batman-ea/1668823", 0, 0);
		Add("https://www.myfxbook.com/members/EDROID/the-retirement-plansangevt--eladiob/2369715", 0, 0);
		Add("https://www.myfxbook.com/members/HappyForex/happy-market-hours-v21/1407880", 0, 0);
		Add("https://www.myfxbook.com/members/ronnyarruda/tradermbrasil/2509193", 0, 0);
		Add("https://www.myfxbook.com/members/danntrader/top-aguilar-casanova/2432622", 0, 0);
		Add("https://www.myfxbook.com/members/AtnetFX/gt2/2267791", 0, 0);
		Add("https://www.myfxbook.com/members/yatarfx/yatar5/1764499", 0, 0);
		Add("https://www.myfxbook.com/members/nmthiyane/khethy/1786887", 0, 0);
		Add("https://www.myfxbook.com/members/yatarfx/yatar4/1765116", 0, 0);
		Add("https://www.myfxbook.com/members/forexwallstreet/wallstreet-asia-demo-all-pairs/1204194", 0, 0);
		Add("https://www.myfxbook.com/members/Mbarak1179/marcotrader/2501602", 0, 0);
		Add("https://www.myfxbook.com/members/FxChampion/testkonto-risiko-05/1831212", 0, 0);
		Add("https://www.myfxbook.com/members/forexdiamond/forex-diamond-ea-usdjpy/1081183", 0, 0);
		Add("https://www.myfxbook.com/members/sakura11/angel-heart/2443428", 0, 0);
		Add("https://www.myfxbook.com/members/iqsoftware2015/gkfx-metatrader-4couk/2517135", 0, 0);
		Add("https://www.myfxbook.com/members/traderinput/alog12-setfile7-traderinputcom/2395027", 0, 0);
		Add("https://www.myfxbook.com/members/myforexinvest/momentumstrategy/775182", 0, 0);
		Add("https://www.myfxbook.com/members/traderpusa/maxthecat/2530957", 0, 0);
		Add("https://www.myfxbook.com/members/ProForexCompany/forex-earth-robot-eurgbp/1927390", 0, 0);
		Add("https://www.myfxbook.com/members/FxChampion/testkonto-risiko-075/2027287", 0, 0);
		Add("https://www.myfxbook.com/members/LuizSchiavi/vps-ger30--ger30jun18/2457720", 0, 0);
		Add("https://www.myfxbook.com/members/IGTrading/b-trading-corporation/2403072", 0, 0);
		
	}
	
	Thread::Start(THISBACK(Updater));
}

Myfxbook::~Myfxbook() {
	running = false;
	while (!stopped) Sleep(100);
}

void Myfxbook::Updater() {
	running = true;
	stopped = false;
	
	
	if (accounts.IsEmpty()) {
		accounts.SetCount(urls.GetCount());
		for(int i = 0; i < accounts.GetCount(); i++) {
			Account& a = accounts[i];
			a.url = urls[i];
			a.id = a.url.Mid(a.url.ReverseFind("/") + 1);
		}
		
		RefreshHistory();
		FixOrders();
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
		
		int sec = ts.Elapsed() / 1000;
		for(int i = sec; i < 60 && running; i++) {
			Sleep(1000);
		}
	}
	
	stopped = true;
}

void Myfxbook::SolveSources() {
	System& sys = GetSystem();
	
	
	// Refresh DataBridges for allowed symbols
	for(int i = 0; i < symbols.GetCount() && running; i++) {
		SymbolStats& s = symbols[i];
		s.accounts.Clear();
		
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
		double point = db.GetPoint();
		ASSERT(point > 0.0);
		
		VectorMap<int, double> account_results;
		
		for(int j = 0; j < accounts.GetCount() && running; j++) {
			Account& a = accounts[j];
			
			int read_pos = 0;
			
			double total_pips = 0.0;
			double total_gain = 1.0;
			double pos_pips = 0.0, neg_pips = 0.0;
			int trade_count = 0;
			
			for(int k = 0; k < a.history_orders.GetCount() && running; k++) {
				Order& o = a.history_orders[k];
				
				if (o.symbol != s.symbol)
					continue;
				
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
				
				if (open_price == 0.0 || close_price == 0.0)
					continue;
				
				double diff = close_price - open_price;
				double pips = diff / point;
				double gain = close_price / open_price - 1;
				
				
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
			
			if (profitability > 0.66 && trade_count >= 10) {
				AccountResult& ar = s.accounts.Add();
				ar.id = j;
				ar.profitability = profitability;
				ar.pips = total_pips;
				ar.gain = total_gain;
			}
		}
		
		Sort(s.accounts, AccountResult());
		DUMPC(account_results);
	}
	
	
	
}

void Myfxbook::FixOrders() {
	
	
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

void Myfxbook::RefreshHistory() {
	String cache_dir = ConfigFile("cache");
	RealizeDirectory(cache_dir);
	
	VectorMap<String, int> symbol_list;
	
	for(int i = 0; i < accounts.GetCount(); i++) {
		Account& a = accounts[i];
		LOG("Account " << i << ": " << a.id);
		
		a.history_orders.Clear();
		
		for (int page = 1; page < 100; page++) {
			
			//String url = "https://www.myfxbook.com/paging.html?pt=4&p=" + IntStr(page) + "&ts=29&&l=x&id=" + a.id + "&invitation=&start=2015-05-18%2000:00&end=&sb=27&st=1&symbols=&magicNumbers=&types=0,1,2,4,19,5&orderTagList=&daysList=&hoursList=&buySellList=&yieldStart=&yieldEnd=&netProfitStart=&netProfitEnd=&durationStart=&durationEnd=&takeProfitStart=&takeProfitEnd=&stopLoss=&stopLossEnd=&sizingStart=&sizingEnd=&selectedTime=&pipsStart=&pipsEnd=";
			String url = "https://www.myfxbook.com/paging.html?pt=4&p=" + IntStr(page) + "&ts=105&&id=" + a.id + "&l=a&invitation=&start=2015-05-18%2000:00&end=&sb=28&st=2&magicNumbers=&symbols=&types=0,1,2,4,19,5&orderTagList=&daysList=&hoursList=&buySellList=&yieldStart=&yieldEnd=&netProfitStart=&netProfitEnd=&durationStart=&durationEnd=&takeProfitStart=&takeProfitEnd=&stopLoss=&stopLossEnd=&sizingStart=&sizingEnd=&selectedTime=&pipsStart=&pipsEnd=&ts=105&z=0.6986965124862867";
			
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
						
						StrTime(o.begin, row[2-s][0].GetText());
						StrTime(o.end, row[4-s][0].GetText());
						o.symbol = row[5-s][0][0].GetText();
						o.action = action == "Sell";
						o.open   = row[9-s][0][0].GetText();
						o.profit = row[9-s][2][0].GetText();
						o.lots   = StrDbl(row[7-s][0].GetText());
						
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
	System& sys = GetSystem();
	ReleaseLog("Myfxbook::RefreshOpen");
	
	String cache_dir = ConfigFile("cache");
	
	Index<int> used_accounts;
	for(int i = 0; i < symbols.GetCount(); i++) {
		SymbolStats& s = symbols[i];
		for(int j = 0; j < s.accounts.GetCount(); j++) {
			used_accounts.FindAdd(s.accounts[j].id);
		}
	}
	
	VectorMap<int, VectorMap<String, double> > all_account_symlots;
	
	for(int i = 0; i < used_accounts.GetCount() && running; i++) {
		int a_id = used_accounts[i];
		Account& a = accounts[a_id];
		VectorMap<String, double>& account_symlots = all_account_symlots.Add(a_id);
		
		a.orders.Clear();
		
		
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
				String action = row[4][0].GetText();
				
				if (action == "Sell" || action == "Buy") {
					Order& o = a.orders.Add();
					
					StrTime(o.begin, row[1][0].GetText());
					o.symbol = row[3][0][0].GetText();
					o.action = action == "Sell";
					o.open   = row[6][0].GetText();
					o.profit = row[9][0][0].GetText();
					o.lots = StrDbl(row[5][0].GetText());
					
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
	}
	
	if (!running) return;
	
	
	for (int i = 0; i < symbols.GetCount(); i++) {
		SymbolStats& s = symbols[i];
		
		symbols[i].signal = 0;
		
		for(int j = 0; j < s.accounts.GetCount(); j++) {
			int a_id = s.accounts[j].id;
			Account& a = accounts[a_id];
			const VectorMap<String, double>& account_symlots = all_account_symlots.Get(a_id);
			
			for(int k = 0; k < account_symlots.GetCount(); k++) {
				String sym = account_symlots.GetKey(k);
				if (sym != s.symbol)
					continue;
				
				if (s.signal != 0)
					continue;
				
				double lots = account_symlots[k];
				if (lots == 0)
					continue;
				
				s.signal += lots > 0 ? +1 : -1;
			}
		}
		
		if (s.signal < -1) s.signal = -1;
		if (s.signal > +1) s.signal = +1;
		
		if (s.signal == 0)
			s.wait = false;
		if (s.wait)
			s.signal = 0;
		
		int id = sys.FindSymbol(s.symbol);
		ReleaseLog("Set real signal " + IntStr(id) + " to " + IntStr(s.signal));
		sys.SetSignal(id, s.signal);
	}
	
	
	latest_update = GetSysTime();
}


void Myfxbook::Data() {
	
	int valuecursor = valuelist.GetCursor();
	
	valuelist.Set(0, 0, "Latest update");
	valuelist.Set(0, 1, latest_update);
	for(int i = 0; i < symbols.GetCount(); i++) {
		SymbolStats& s = symbols[i];
		valuelist.Set(1+i, 0, s.symbol);
		valuelist.Set(1+i, 1, s.signal);
	}
	
	int a_id = -1;
	if (valuecursor <= 0) {
		
		for(int i = 0; i < accounts.GetCount(); i++) {
			Account& a = accounts[i];
			accountlist.Set(i, 0, a.id);
			for(int j = 1; j < 4; j++)
				accountlist.Set(i, j, "");
		}
		
		a_id = accountlist.GetCursor();
	} else {
		SymbolStats& s = symbols[valuecursor-1];
		
		for(int i = 0; i < s.accounts.GetCount(); i++) {
			AccountResult& ar = s.accounts[i];
			Account& a = accounts[ar.id];
			
			accountlist.Set(i, 0, a.id);
			accountlist.Set(i, 1, ar.profitability);
			accountlist.Set(i, 2, ar.pips);
			accountlist.Set(i, 3, ar.gain);
		}
		accountlist.SetCount(s.accounts.GetCount());
		
		a_id = accountlist.GetCursor();
		if (a_id >= 0 && a_id < s.accounts.GetCount())
			a_id = s.accounts[a_id].id;
	}
	
	if (a_id >= 0 && a_id < accounts.GetCount()) {
		Account& a = accounts[a_id];
		
		for(int i = 0; i < a.orders.GetCount(); i++) {
			Order& o = a.orders[i];
			orderlist.Set(i, 0, o.begin);
			orderlist.Set(i, 1, o.symbol);
			orderlist.Set(i, 2, o.action);
			orderlist.Set(i, 3, o.lots);
			orderlist.Set(i, 4, o.open);
			orderlist.Set(i, 5, o.profit);
		}
		orderlist.SetCount(a.orders.GetCount());
		
		for(int i = 0; i < a.history_orders.GetCount(); i++) {
			Order& o = a.history_orders[i];
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
