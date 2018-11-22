
#include <plugin/webdriverxx/webdriverxx.h>
using namespace webdriverxx;

#include <Core/Core.h>
using namespace Upp;

namespace Config {

INI_STRING(username, "", "username");
INI_STRING(password, "", "password");

};

String msg;

void Listener() {
	TcpSocket listener;
	
	if (!listener.Listen(17777)) {
		Panic("Couldn't listen port");
	}
	
	while (true) {
		TcpSocket s;
		
		s.Accept(listener);
		
		if (s.IsOpen()) {
			s.Put(msg);
		}
	}
	
}

CONSOLE_APP_MAIN
{
	SetIniFile(ConfigFile("ForexSignal.ini"));
	
	Thread::Start(callback(Listener));
	
	if ((String)Config::username == "") return;
	if ((String)Config::password == "") return;
	
	Vector<String> symbols;
	symbols.Add("EURUSD");
	symbols.Add("USDCHF");
	symbols.Add("GBPUSD");
	symbols.Add("USDJPY");
	symbols.Add("USDCAD");
	symbols.Add("AUDUSD");
	symbols.Add("EURJPY");
	symbols.Add("NZDUSD");
	symbols.Add("GBPCHF");
	
	WebDriver firefox = Start(Chrome());
    Session ses = firefox.Navigate("https://foresignal.com/en/login/index");
    
    Element user_name = ses.FindElement(ByCss("input[name=user_name]"));
    user_name.SendKeys(((String)Config::username).Begin());
    
	Element user_password = ses.FindElement(ByCss("input[name=user_password]"));
    user_password.SendKeys(((String)Config::password).Begin());
    
	user_password.Submit();
	while (true) {
		
		String content = ses.GetSource().c_str();
		
		//LOG(content);
		String m;
		
		int a = 0;
		for(int i = 0; i < symbols.GetCount(); i++) {
			a = content.Find("signal-body", a+1);
			if (a == -1) break;
			int b = content.Find("signal-body", a+1);
			if (b == -1) b = content.GetCount();
			
			String sub = content.Mid(a, b-a);
			int sym_a = sub.Find("col-sm-6");
			sym_a = sub.Find(">", sym_a) + 1;
			int sym_b = sub.Find(" ", sym_a);
			String sym = sub.Mid(sym_a, sym_b - sym_a);
			sym = sym.Left(3) + sym.Right(3);
			
			int from_a = sub.Find("From");
			from_a = sub.Find("showDTgmt", from_a) + 10;
			int from_b = sub.Find(")", from_a);
			String from = sub.Mid(from_a, from_b-from_a);
			
			int till_a = sub.Find("Till");
			till_a = sub.Find("showDTgmt", till_a) + 10;
			int till_b = sub.Find(")", till_a);
			String till = sub.Mid(till_a, till_b-till_a);
			
			String sig;
			int sig_a = sub.Find("Sell at");
			if (sig_a == -1) {
				sig = "Buy";
				sig_a = sub.Find("Buy at");
				if (sig_a == -1) {
					sig_a = sub.Find("Bought at");
					if (sig_a == -1) {
						sig_a = sub.Find("Sold at");
						sig = "Sell";
					}
				}
			} else {
				sig = "Sell";
			}
			sig_a = sub.Find("/script", sig_a) + 8;
			int sig_b = sub.Find("<", sig_a);
			String sig_at = sub.Mid(sig_a, sig_b - sig_a);
			
			String tp;
			int tp_a = sub.Find("Take profit");
			if (tp_a != -1) {
				tp_a = sub.Find("/script", tp_a) + 8;
				int tp_b = sub.Find("<", tp_a);
				tp = sub.Mid(tp_a, tp_b - tp_a);
			}
			
			String sl;
			int sl_a = sub.Find("Stop loss");
			if (sl_a != -1) {
				sl_a = sub.Find("/script", sl_a) + 8;
				int sl_b = sub.Find("<", sl_a);
				sl = sub.Mid(sl_a, sl_b - sl_a);
			}
			
			
			m << sym << "\n";
			m << from << "\n";
			m << till << "\n";
			m << sig << "\n";
			m << sig_at << "\n";
			m << tp << "\n";
			m << sl << "\n";
			
		}
		
		msg = m;
		
		for(int i = 0; i < 30; i++) {
			Sleep(1000);
		}
		
		ses.Refresh();
	}
}
