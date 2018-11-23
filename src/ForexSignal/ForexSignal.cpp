
#include <plugin/webdriverxx/webdriverxx.h>
using namespace webdriverxx;

#include <Core/Core.h>
using namespace Upp;

#include <MailClient/IMAPClient.h>

namespace Config {

INI_STRING(username, "", "username");
INI_STRING(password, "", "password");

INI_STRING(email_user, "", "Email username");
INI_STRING(email_pass, "", "Email password");
INI_STRING(email_server, "", "Email server");
INI_INT(email_port, 0, "Email port");

};


struct Symbol : Moveable<Symbol> {
	String sym, from, till, sig, sig_at, tp, sl;
};

struct App {
	typedef App CLASSNAME;
	
	VectorMap<String, Symbol> symbols;
	String msg;
	WebDriver firefox;
	bool email_init = false;
	int email_count = 0;
	CIMAPClient client;
	Mutex lock;
	
	
	App() :
		client([](const std::string& strLogMsg) { std::cout << strLogMsg << std::endl;  }),
		firefox(Start(Firefox())) {}
	
	void Listener() {
		TcpSocket listener;
		
		if (!listener.Listen(17777)) {
			Panic("Couldn't listen port");
		}
		
		while (true) {
			TcpSocket s;
			
			s.Accept(listener);
			
			if (s.IsOpen()) {
				lock.Enter();
				s.Put(msg);
				lock.Leave();
			}
		}
		
	}
	
	void ParseSymbol(String sub) {
		int sym_a = sub.Find("col-sm-6");
		sym_a = sub.Find(">", sym_a) + 1;
		int sym_b = sub.Find(" ", sym_a);
		String sym = sub.Mid(sym_a, sym_b - sym_a);
		sym = sym.Left(3) + sym.Right(3);
		
		Symbol& s = symbols.Get(sym);
		s.sym = sym;
		
		int from_a = sub.Find("From");
		from_a = sub.Find("showDTgmt", from_a) + 10;
		int from_b = sub.Find(")", from_a);
		s.from = sub.Mid(from_a, from_b-from_a);
		
		int till_a = sub.Find("Till");
		till_a = sub.Find("showDTgmt", till_a) + 10;
		int till_b = sub.Find(")", till_a);
		s.till = sub.Mid(till_a, till_b-till_a);
		
		int sig_a = sub.Find("Sell at");
		if (sig_a == -1) {
			s.sig = "Buy";
			sig_a = sub.Find("Buy at");
			if (sig_a == -1) {
				sig_a = sub.Find("Bought at");
				if (sig_a == -1) {
					sig_a = sub.Find("Sold at");
					s.sig = "Sell";
				}
			}
		} else {
			s.sig = "Sell";
		}
		sig_a = sub.Find("/script", sig_a) + 8;
		int sig_b = sub.Find("<", sig_a);
		s.sig_at = sub.Mid(sig_a, sig_b - sig_a);
		
		s.tp = "";
		int tp_a = sub.Find("Take profit");
		if (tp_a != -1) {
			tp_a = sub.Find("/script", tp_a) + 8;
			int tp_b = sub.Find("<", tp_a);
			s.tp = sub.Mid(tp_a, tp_b - tp_a);
		}
		
		s.sl = "";
		int sl_a = sub.Find("Stop loss");
		if (sl_a != -1) {
			sl_a = sub.Find("/script", sl_a) + 8;
			int sl_b = sub.Find("<", sl_a);
			s.sl = sub.Mid(sl_a, sl_b - sl_a);
		}
	}
	
	void RefreshAll(String content) {
		
		if (content.Find("We're sorry") != -1)
			Sleep(3*60*1000);
		
		//LOG(content);
		String m;
		
		int a = 0;
		for(int i = 0; i < 9; i++) {
			a = content.Find("signal-body", a+1);
			if (a == -1) break;
			int b = content.Find("signal-body", a+1);
			if (b == -1) b = content.GetCount();
			
			String sub = content.Mid(a, b-a);
			ParseSymbol(sub);
		}
		
		CreateMessage();
	}
	
	void CreateMessage() {
		String m;
		
		for(int i = 0; i < symbols.GetCount(); i++) {
			Symbol& s = symbols[i];
			m << s.sym << "\n";
			m << s.from << "\n";
			m << s.till << "\n";
			m << s.sig << "\n";
			m << s.sig_at << "\n";
			m << s.tp << "\n";
			m << s.sl << "\n";
		}
		
		lock.Enter();
		msg = m;
		lock.Leave();
	}
	
	
	void InitMail() {
		String user = Config::email_user;
		String pass = Config::email_pass;
		String srv = Config::email_server;
		int port = Config::email_port;
		String srv_str = srv + ":" + IntStr(port);
		client.InitSession(srv_str.Begin(), user.Begin(), pass.Begin(),
			CMailClient::SettingsFlag::ALL_FLAGS, CMailClient::SslTlsFlag::ENABLE_SSL);
	}
	
	int GetMailCount() {
		/*
		* FLAGS (\Answered \Flagged \Draft \Deleted \Seen $ATTACHMENT $Junk $MDNSent $NotJunk $NotPhishing $Phishing Junk NonJunk NotJunk receipt-handled)
		* OK [PERMANENTFLAGS ()] Flags permitted.
		* OK [UIDVALIDITY 631009610] UIDs valid.
		* 3387 EXISTS
		* 0 RECENT
		* OK [UIDNEXT 8539] Predicted next UID.
		* OK [HIGHESTMODSEQ 2865771]
		*/
		std::string info, path;
		path = "INBOX";
		client.InfoFolder(path, info);
		String s = info;
		if (s.IsEmpty()) {
			InitMail();
			client.InfoFolder(path, info);
			s = info;
		}
		LOG(s);
		//int b = s.Find(" EXISTS");
		//int a = s.ReverseFind(" ", b-1) + 1;
		int a = s.Find("UIDNEXT") + 8;
		int b = s.Find(" ", a);
		s = s.Mid(a, b-a);
		email_count = StrInt(s);
		return email_count;
	}
	
	String GetMail(int i) {
		std::string strEmail;
		String str = IntStr(i + 1);
		/* retrieve the mail number 1 and store it in strEmail */
		bool bResRcvStr = client.GetString(str.Begin(), strEmail);
		
		return strEmail;
	}
	
	void ProcessMail(String url) {
		int hash = url.GetHashValue();
		String dir = ConfigFile("html");
		RealizeDirectory(dir);
		String file = AppendFileName(dir, IntStr(hash) + ".htm");
		String content;
		
		if (FileExists(file)) {
			content = LoadFile(file);
		} else {
			Session ses = firefox.Navigate(url.Begin());
			content = ses.GetSource();
			
			if (content.GetCount()) {
				FileOut fout(file);
				fout << content;
			}
		}
		
		int a = content.Find("signal-body", a+1);
		if (a == -1)
			return;
		int b = content.GetCount();
		
		String sub = content.Mid(a, b-a);
		ParseSymbol(sub);
	}
	
	void MailData() {
		String mail_dir = ConfigFile("mail");
		RealizeDirectory(mail_dir);
		
		String user = Config::email_user;
		String pass = Config::email_pass;
		String srv = Config::email_server;
		int port = Config::email_port;
		if (!email_init && !user.IsEmpty() && !pass.IsEmpty() && !srv.IsEmpty() && port) {
			
			InitMail();
			
			email_init = true;
		}
		else if (!email_init) Panic("No mail settings in ForexSignal.ini");
		
		if (email_init) {
			GetMailCount();
			
			VectorMap<String, String> urls;
			for(int i = max(0, email_count - 30); i < email_count; i++) {
				String mail_file = AppendFileName(mail_dir, IntStr(i) + ".txt");
				String mail;
				if (FileExists(mail_file)) {
					mail = LoadFile(mail_file);
				} else {
					mail = GetMail(i);
					
					if (mail.GetCount()) {
						FileOut fout(mail_file);
						fout << mail;
					}
				}
				
				//LOG("MAILLLL");
				//LOG(mail);
				
				if (mail.Find("From: Foresignal.com <signal@foresignal.com>") != -1) {
					int a = mail.Find("href") + 6;
					int b = mail.Find("\"", a);
					String url = mail.Mid(a, b-a);
					//https://foresignal.com/en/signals/one/audusd
					a = url.Find("/one/") + 5;
					b = a + 6;
					String code = url.Mid(a, b-a);
					urls.GetAdd(code, url) = url;
				}
			}
			
			for(int i = 0; i < urls.GetCount(); i++) {
				ProcessMail(urls[i]);
				Sleep(3*1000);
			}
			
			CreateMessage();
		}
	}
	
	
	void Run() {
		Thread::Start(THISBACK(Listener));
		
		if ((String)Config::username == "") return;
		if ((String)Config::password == "") return;
		
		
		symbols.Add("EURUSD");
		symbols.Add("USDCHF");
		symbols.Add("GBPUSD");
		symbols.Add("USDJPY");
		symbols.Add("USDCAD");
		symbols.Add("AUDUSD");
		symbols.Add("EURJPY");
		symbols.Add("NZDUSD");
		symbols.Add("GBPCHF");
		
		Session ses = firefox.Navigate("https://foresignal.com/en/login/index");
	    
	    Element user_name = ses.FindElement(ByCss("input[name=user_name]"));
	    user_name.SendKeys(((String)Config::username).Begin());
	    
		Element user_password = ses.FindElement(ByCss("input[name=user_password]"));
	    user_password.SendKeys(((String)Config::password).Begin());
	    
		user_password.Submit();
	
		RefreshAll(ses.GetSource().c_str());
		
		
		while (true) {
			
			MailData();
			
			Sleep(30*1000);
		}
	}
};

CONSOLE_APP_MAIN
{
	SetIniFile(ConfigFile("ForexSignal.ini"));
	
	App().Run();
}
