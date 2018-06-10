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
		
		symbols.Add("EURNZD");
		
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
		
		#ifndef flagDEBUG
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
		
		Add("https://www.myfxbook.com/members/ronnyarruda/tradermbrasil/2509193", 0, 0);
		Add("https://www.myfxbook.com/members/HappyForex/happy-market-hours-v21/1407880", 0, 0);
		Add("https://www.myfxbook.com/members/HappyForexBLU/happy-market-hours-v21-blu/2382866", 0, 0);
		Add("https://www.myfxbook.com/members/FreeWorkForex/abamk-2-e-avuadores/2005079", 0, 0);
		Add("https://www.myfxbook.com/members/Mbarak1179/mbarek-belouse/2489926", 0, 0);
		Add("https://www.myfxbook.com/members/MoshiM/cot/2521550", 0, 0);
		Add("https://www.myfxbook.com/members/MerlinBrasil/tradeview-march-2018/2451595", 0, 0);
		Add("https://www.myfxbook.com/members/danntrader/top-aguilar-casanova/2432622", 0, 0);
		Add("https://www.myfxbook.com/members/nmthiyane/khethy/1786887", 0, 0);
		Add("https://www.myfxbook.com/members/AtnetFX/gt2/2267791", 0, 0);
		Add("https://www.myfxbook.com/members/yatarfx/yatar5/1764499", 0, 0);
		Add("https://www.myfxbook.com/members/Mbarak1179/marcotrader/2501602", 0, 0);
		Add("https://www.myfxbook.com/members/LuizSchiavi/vps-ger30--ger30jun18/2457720", 0, 0);
		Add("https://www.myfxbook.com/members/Thandah/kwanda/2180958", 0, 0);
		Add("https://www.myfxbook.com/members/dcansi/dsc-lucas-forex-turbo/2523854", 0, 0);
		Add("https://www.myfxbook.com/members/yatarfx/yatar4/1765116", 0, 0);
		Add("https://www.myfxbook.com/members/forexwallstreet/wallstreet-asia-demo-all-pairs/1204194", 0, 0);
		Add("https://www.myfxbook.com/members/FxChampion/testkonto-risiko-05/1831212", 0, 0);
		Add("https://www.myfxbook.com/members/sakura11/angel-heart/2443428", 0, 0);
		Add("https://www.myfxbook.com/members/forexdiamond/forex-diamond-ea-usdjpy/1081183", 0, 0);
		Add("https://www.myfxbook.com/members/traderinput/alog12-setfile7-traderinputcom/2395027", 0, 0);
		Add("https://www.myfxbook.com/members/iqsoftware2015/gkfx-metatrader-4couk/2517135", 0, 0);
		Add("https://www.myfxbook.com/members/myforexinvest/momentumstrategy/775182", 0, 0);
		Add("https://www.myfxbook.com/members/sakura11/zeus/2443430", 0, 0);
		Add("https://www.myfxbook.com/members/Listapad/hejj/1674244", 0, 0);
		Add("https://www.myfxbook.com/members/traderpusa/maxthecat/2530957", 0, 0);
		Add("https://www.myfxbook.com/members/redliner/usdcad-set-2/1711732", 0, 0);
		Add("https://www.myfxbook.com/members/forexTik/ccccc/2460783", 0, 0);
		Add("https://www.myfxbook.com/members/4FM1PerezHazelyn/4fm1perezhazelyn/1750257", 0, 0);
		Add("https://www.myfxbook.com/members/cvbuy/4fm5uycarol/1753327", 0, 0);
		Add("https://www.myfxbook.com/members/yeray007/yeray-zurita/2538566", 0, 0);
		Add("https://www.myfxbook.com/members/dubips/dubips-anna/2415218", 0, 0);
		Add("https://www.myfxbook.com/members/ProForexCompany/forex-earth-robot-eurgbp/1927390", 0, 0);
		Add("https://www.myfxbook.com/members/FxChampion/testkonto-risiko-075/2027287", 0, 0);
		Add("https://www.myfxbook.com/members/bel3ouchi_/contest-bel3ouchi/2552500", 0, 0);
		Add("https://www.myfxbook.com/members/hieutv/batman-ea/1668823", 0, 0);
		Add("https://www.myfxbook.com/members/manvarov/berndale-usd-1000000/2329847", 0, 0);
		Add("https://www.myfxbook.com/members/ImolaJu/finch-demo-eur/2343562", 0, 0);
		Add("https://www.myfxbook.com/members/volatilityfactor/volatility-factor-v70-eurusdgbpusd/1510370", 0, 0);
		Add("https://www.myfxbook.com/members/Romm/contest-romm/2529429", 0, 0);
		Add("https://www.myfxbook.com/members/alphax28/16-pairs-sl3000-pepperstone/2452850", 0, 0);
		Add("https://www.myfxbook.com/members/mccoyducatillon/20150502/1151204", 0, 0);
		Add("https://www.myfxbook.com/members/mam_ea/icm-1/2353357", 0, 0);
		Add("https://www.myfxbook.com/members/mphilipp/3--dayli/2540701", 0, 0);
		Add("https://www.myfxbook.com/members/twhiediana/contest-twhiediana/2529400", 0, 0);
		Add("https://www.myfxbook.com/members/Alison_Ian/demo2/2379680", 0, 0);
		Add("https://www.myfxbook.com/members/Sartaz_/contest-sartaz/2552561", 0, 0);
		Add("https://www.myfxbook.com/members/abusaad4123/abusaad/2477316", 0, 0);
		Add("https://www.myfxbook.com/members/Nikolay56/no-sleep/2486755", 0, 0);
		Add("https://www.myfxbook.com/members/forexwallstreet/wallstreet-forex-robots-all/1742982", 0, 0);
		Add("https://www.myfxbook.com/members/SergeiSergei/-14/2334416", 0, 0);
		Add("https://www.myfxbook.com/members/ProForexCompany/forex-earth-robot-gbpusd/2405011", 0, 0);
		Add("https://www.myfxbook.com/members/ProForexCompany/forex-earth-robot-eurgbp-new/2151232", 0, 0);
		Add("https://www.myfxbook.com/members/vinhlp/contest-vinhlp/2529428", 0, 0);
		Add("https://www.myfxbook.com/members/william90127447/william-heung-demo-20226216/2469414", 0, 0);
		Add("https://www.myfxbook.com/members/forexwallstreet/wallstreet-20-evolution-all-2/2250695", 0, 0);
		Add("https://www.myfxbook.com/members/sakura11/hanabi/2461217", 0, 0);
		Add("https://www.myfxbook.com/members/IntelFx/mt4-20254983-02/2528238", 0, 0);
		Add("https://www.myfxbook.com/members/meynard_p/magic7-meynard/2423381", 0, 0);
		Add("https://www.myfxbook.com/members/fxwatanabe3/sfe-price-action-tickmill-demo/1681694", 0, 0);
		Add("https://www.myfxbook.com/members/dubips/dubips-ruth/2415223", 0, 0);
		Add("https://www.myfxbook.com/members/abudz/de-ch/2510628", 0, 0);
		Add("https://www.myfxbook.com/members/fxking_net/fxking03-test/2438509", 0, 0);
		Add("https://www.myfxbook.com/members/gilsapir86/demo-all-3k/1930202", 0, 0);
		Add("https://www.myfxbook.com/members/Dmitro75/basic/2503570", 0, 0);
		Add("https://www.myfxbook.com/members/spt22/5674/2398662", 0, 0);
		Add("https://www.myfxbook.com/members/ProSystems/professional-trading-portfolio/1550639", 0, 0);
		Add("https://www.myfxbook.com/members/FxChampion/testkonto-risiko-025/1868008", 0, 0);
		Add("https://www.myfxbook.com/members/Alison_Ian/demo1/2379389", 0, 0);
		Add("https://www.myfxbook.com/members/bebert83/contest-bebert83/2529451", 0, 0);
		Add("https://www.myfxbook.com/members/forexwallstreet/wallstreet-recovery-pro-20-evolution/1934826", 0, 0);
		Add("https://www.myfxbook.com/members/Jonsummer99/star/2481327", 0, 0);
		Add("https://www.myfxbook.com/members/Yury48/century22-29686365/2505138", 0, 0);
		Add("https://www.myfxbook.com/members/Stone79/flash-renko-v6/2509232", 0, 0);
		Add("https://www.myfxbook.com/members/oshaban/oshaban-ea-v542/2487280", 0, 0);
		Add("https://www.myfxbook.com/members/forexwallstreet/wallstreet-asia-demo-all-pairs/1204188", 0, 0);
		Add("https://www.myfxbook.com/members/pdy2017/fission123/2488583", 0, 0);
		Add("https://www.myfxbook.com/members/thunder55/modest-ea-hr/2538819", 0, 0);
		Add("https://www.myfxbook.com/members/sakura11/easy-bowl/2205254", 0, 0);
		Add("https://www.myfxbook.com/members/Bancha_B/thaiban-ctradergu/2499484", 0, 0);
		Add("https://www.myfxbook.com/members/maida0922/wsfrfdvf-ref/1103514", 0, 0);
		Add("https://www.myfxbook.com/members/IRTIRIRI/irtiriri/2497250", 0, 0);
		Add("https://www.myfxbook.com/members/bosspinlac/markpinlac-hotforex-demo/2450214", 0, 0);
		Add("https://www.myfxbook.com/members/dubips/dubips-kathleen/2415206", 0, 0);
		Add("https://www.myfxbook.com/members/jnotaro/xm-maxwinner-demo/2519875", 0, 0);
		Add("https://www.myfxbook.com/members/Walle_500_ea/1000-fxpro-19th-march/2488203", 0, 0);
		Add("https://www.myfxbook.com/members/Topu007/contest-topu007/2529464", 0, 0);
		Add("https://www.myfxbook.com/members/ypyea/ypy-serena-pro-darwinex/1891091", 0, 0);
		Add("https://www.myfxbook.com/members/Merlin777/gena-crocodile-tradelikeaproru/2358860", 0, 0);
		Add("https://www.myfxbook.com/members/Phil33/philipp-fragner/2422811", 0, 0);
		Add("https://www.myfxbook.com/members/extreme_warrior/mt4-940561/2457834", 0, 0);
		Add("https://www.myfxbook.com/members/meynard_p/fxpro-mt4/2449300", 0, 0);
		Add("https://www.myfxbook.com/members/simtradepro/flash-forward-forex-standard/2518080", 0, 0);
		Add("https://www.myfxbook.com/members/hongpanshou/hps-6/2467963", 0, 0);
		Add("https://www.myfxbook.com/members/NEXTGEN/start-dec2017-for-nextgen/2325308", 0, 0);
		Add("https://www.myfxbook.com/members/fokeer/contest-fokeer/2529435", 0, 0);
		Add("https://www.myfxbook.com/members/mahmoud66/mahmoud/2524025", 0, 0);
		Add("https://www.myfxbook.com/members/Gideon3310/learn-forex-account-mt4-2111866/2503469", 0, 0);
		Add("https://www.myfxbook.com/members/SwingFish/mf-crowd-mfb/1660620", 0, 0);
		Add("https://www.myfxbook.com/members/highlow159/projectx/2252334", 0, 0);
		Add("https://www.myfxbook.com/members/Thriss/gbpjpy-ra/2502509", 0, 0);
		Add("https://www.myfxbook.com/members/Merlin777/turtles-tradelikeaproru/2331949", 0, 0);
		Add("https://www.myfxbook.com/members/majamivice/stdr-daily-ea/2533151", 0, 0);
		Add("https://www.myfxbook.com/members/pasha_086/demo-1/2406935", 0, 0);
		Add("https://www.myfxbook.com/members/iqsoftware2015/gkfx-metatrader-4couk-300/2517146", 0, 0);
		Add("https://www.myfxbook.com/members/mollyred/chan-hoi-ling/2448112", 0, 0);
		Add("https://www.myfxbook.com/members/ReadyToRoll/wavefighter-30-18-pairs-low/2434002", 0, 0);
		Add("https://www.myfxbook.com/members/ReflexionAi/wsf-icmarkets/2282146", 0, 0);
		Add("https://www.myfxbook.com/members/abudz/ts-50-gu-uj/2479239", 0, 0);
		Add("https://www.myfxbook.com/members/Thanomc/tony-demofxprimusvlfx/2508366", 0, 0);
		Add("https://www.myfxbook.com/members/philipchoi0205/philipchoi-demo/2034587", 0, 0);
		Add("https://www.myfxbook.com/members/Mandal_/contest-mandal/2552632", 0, 0);
		Add("https://www.myfxbook.com/members/philipchoi0205/philipchoi-demo/2034587", 0, 0);
		Add("https://www.myfxbook.com/members/Zamile/zamile3/2199037", 0, 0);
		Add("https://www.myfxbook.com/members/IntelFx/mt4-20254126-03/2509012", 0, 0);
		Add("https://www.myfxbook.com/members/abudz/ts-50-gu-uj/2479239", 0, 0);
		Add("https://www.myfxbook.com/members/Asalnikov/martin-setka/2402596", 0, 0);
		Add("https://www.myfxbook.com/members/ypyea/ypy-ea-immortalis-elite-advancedmarkets/1920895", 0, 0);
		Add("https://www.myfxbook.com/members/lhddlh/wave-106/2467760", 0, 0);
		Add("https://www.myfxbook.com/members/Tianotrader/tianotrader/2557146", 0, 0);
		Add("https://www.myfxbook.com/members/dubips/dubips-virginia/2415231", 0, 0);
		Add("https://www.myfxbook.com/members/zbyszq/0nowe/2537640", 0, 0);
		Add("https://www.myfxbook.com/members/kishisaki/kishisaki-commodities/2263169", 0, 0);
		Add("https://www.myfxbook.com/members/jaecdeguzman/4fm2deguzmanjulieanne/1745142", 0, 0);
		Add("https://www.myfxbook.com/members/romsesua/volnafx/2161488", 0, 0);
		Add("https://www.myfxbook.com/members/tradingwithw/tp-v20-demo/2232332", 0, 0);
		Add("https://www.myfxbook.com/members/dubips/dubips-angus/2260567", 0, 0);
		Add("https://www.myfxbook.com/members/automatedfxtools/dynamic-pro-scalper-5000/1487626", 0, 0);
		Add("https://www.myfxbook.com/members/PFXSltd/standard/2367912", 0, 0);
		Add("https://www.myfxbook.com/members/halo031/oanda-demo-ea/2381857", 0, 0);
		Add("https://www.myfxbook.com/members/dfoster/struggling-trader/2509337", 0, 0);
		Add("https://www.myfxbook.com/members/dubips/dubips-quillon/2260531", 0, 0);
		Add("https://www.myfxbook.com/members/caironews/pc-1/2371395", 0, 0);
		Add("https://www.myfxbook.com/members/matrixxxlxxx/daily-profit/2504310", 0, 0);
		Add("https://www.myfxbook.com/members/Endless_Invest/endless-3k-ei/2226791", 0, 0);
		Add("https://www.myfxbook.com/members/Zabarkin/squared-financial/2409189", 0, 0);
		Add("https://www.myfxbook.com/members/lhddlh/cts02/2320475", 0, 0);
		Add("https://www.myfxbook.com/members/VivoMesa/progressive-flex-hybrid/2205038", 0, 0);
		// Continue from https://www.myfxbook.com/systems?pt=6&p=4&ts=601&profitType=0&profitValue=0.0&drawType=1&drawValue=50.0&profitabilityType=0&profitabilityValue=0.0&ageType=0&ageValue=30&tradingType=0&systemType=0&symbols=&accountType=1&size=40&sb=19&st=2&lastTraded=7&tradesType=1&pipsType=1&pipsValue=30&equityType=1&equityValue=30&serverOid=0&regulationType=0#?pt=6&p=5&ts=602&profitType=0&profitValue=0.0&drawType=1&drawValue=50.0&profitabilityType=0&profitabilityValue=0.0&ageType=0&ageValue=30&tradingType=0&systemType=0&symbols=&accountType=1&size=10&sb=19&st=2&lastTraded=7&tradesType=1&pipsType=1&pipsValue=30&equityType=1&equityValue=30&serverOid=0&regulationType=0
		
		Add("https://www.myfxbook.com/members/galimoms/contest-galimoms/2539449", 0, 0);
		Add("https://www.myfxbook.com/members/Redoy2000/contest-redoy2000/2547555", 0, 0);
		Add("https://www.myfxbook.com/members/Balaji007/contest-balaji007/2529532", 0, 0);
		Add("https://www.myfxbook.com/members/awvwdeyu/contest-awvwdeyu/2541535", 0, 0);
		Add("https://www.myfxbook.com/members/brussolsis/contest-brussolsis/2541556", 0, 0);
		Add("https://www.myfxbook.com/members/Forexsakeuk/contest-forexsakeuk/2545659", 0, 0);
		Add("https://www.myfxbook.com/members/Mutiso/contest-mutiso/2536077", 0, 0);
		Add("https://www.myfxbook.com/members/investi/contest-investi/2548033", 0, 0);
		Add("https://www.myfxbook.com/members/bassistof/contest-bassistof/2540976", 0, 0);
		Add("https://www.myfxbook.com/members/pandi_/contest-pandi/2552711", 0, 0);
		Add("https://www.myfxbook.com/members/FX_Jewel/contest-fx-jewel/2529506", 0, 0);
		Add("https://www.myfxbook.com/members/forexmyfx/contest-forexmyfx/2548176", 0, 0);
		Add("https://www.myfxbook.com/members/AGFXTM/contest-agfxtm/2529489", 0, 0);
		Add("https://www.myfxbook.com/members/himma_satria/contest-himma-satria/2529070", 0, 0);
		Add("https://www.myfxbook.com/members/dinrazali/contest-dinrazali/2547372", 0, 0);
		Add("https://www.myfxbook.com/members/Romm/contest-romm/2529429", 0, 0);
		Add("https://www.myfxbook.com/members/ata16888/contest-ata16888/2545110", 0, 0);
		Add("https://www.myfxbook.com/members/midokaka2/contest-midokaka2/2533626", 0, 0);
		Add("https://www.myfxbook.com/members/jadegmyd_/contest-jadegmyd/2553128", 0, 0);
		Add("https://www.myfxbook.com/members/fagonzo/contest-fagonzo/2545704", 0, 0);
		Add("https://www.myfxbook.com/members/vinhlp/contest-vinhlp/2529428", 0, 0);
		Add("https://www.myfxbook.com/members/Ahsan1234/contest-ahsan1234/2547925", 0, 0);
		Add("https://www.myfxbook.com/members/bel3ouchi_/contest-bel3ouchi/2552500", 0, 0);
		Add("https://www.myfxbook.com/members/Jogoboyo_/contest-jogoboyo/2552831", 0, 0);
		Add("https://www.myfxbook.com/members/migxibit/contest-migxibit/2545740", 0, 0);
		Add("https://www.myfxbook.com/members/HenryLeong/contest-henryleong/2546312", 0, 0);
		Add("https://www.myfxbook.com/members/rogayah/contest-rogayah/2531461", 0, 0);
		Add("https://www.myfxbook.com/members/mazensalama/contest-mazensalama/2537204", 0, 0);
		Add("https://www.myfxbook.com/members/klpsd_/contest-klpsd/2552668", 0, 0);
		Add("https://www.myfxbook.com/members/Cmnady/contest-cmnady/2545285", 0, 0);
		Add("https://www.myfxbook.com/members/forexeztrading03/contest-forexeztrading03/2529197", 0, 0);
		Add("https://www.myfxbook.com/members/cikgufx/contest-cikgufx/2546320", 0, 0);
		Add("https://www.myfxbook.com/members/maxs/contest-maxs/2539455", 0, 0);
		Add("https://www.myfxbook.com/members/Sartaz_/contest-sartaz/2552561", 0, 0);
		Add("https://www.myfxbook.com/members/dyahfx/contest-dyahfx/2544072", 0, 0);
		Add("https://www.myfxbook.com/members/chbou/contest-chbou/2529399", 0, 0);
		Add("https://www.myfxbook.com/members/Atialb/contest-atialb/2530589", 0, 0);
		Add("https://www.myfxbook.com/members/josephlon/contest-josephlon/2531284", 0, 0);
		Add("https://www.myfxbook.com/members/flyl3/contest-flyl3/2545270", 0, 0);
		Add("https://www.myfxbook.com/members/dream144/contest-dream144/2529895", 0, 0);
		#endif
	}
	
	//Thread::Start(THISBACK(Updater));
}

Myfxbook::~Myfxbook() {
	running = false;
	while (!stopped) Sleep(100);
}

void Myfxbook::Updater() {
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
		AddDelay(); // to simulate lagging updating
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

void Myfxbook::SolveSources() {
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

void Myfxbook::AddDelay() {
	for(int i = 0; i < accounts.GetCount() && running; i++) {
		Account& a = accounts[i];
		
		for(int j = 0; j < a.history_orders.GetCount(); j++) {
			Order& o = a.history_orders[j];
			
			o.begin += 5*60;
			o.end += 5*60;
		}
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
		ReleaseLog("Myfxbook::RefreshHistory Account " + IntStr(i) + ": " + a.id);
		
		a.history_orders.Clear();
		
		for (int page = 1; page < 100; page++) {
			
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

void Myfxbook::RefreshOpen() {
	System& sys = GetSystem();
	ReleaseLog("Myfxbook::RefreshOpen");
	
	
	for(int i = 0; i < symbols.GetCount(); i++)
		symbols[i].lots_mult = 0.0;
	
	#if 1
	for(int i = 0; i < accounts.GetCount() && running; i++) {
		Account& a = accounts[i];
		if (a.id == "1657111") {
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

bool Myfxbook::RefreshAccountOpen(int i) {
	Account& a = accounts[i];
	VectorMap<String, double> account_symlots;
	
	if (!(a.profitability > 0.50 && a.gain > 0.0 && a.pips > 0.0))
		return false;
	
	bool new_orders = a.orders.IsEmpty();
	bool is_active = i == active_account;
	
	a.orders.Clear();
	
	String cache_dir = ConfigFile("cache");
	
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
	
	for(int i = 0; i < accounts.GetCount(); i++) {
		Account& a = accounts[i];
		accountlist.Set(i, 0, a.id);
		accountlist.Set(i, 1, a.profitability);
		accountlist.Set(i, 2, a.pips);
		accountlist.Set(i, 3, a.gain);
		accountlist.Set(i, 4, a.av_gain);
	}
	
	a_id = accountlist.GetCursor();
	
	
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
