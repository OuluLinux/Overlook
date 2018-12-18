#include "Overlook.h"

namespace Overlook {
using namespace libmt;



ForeSignal::ForeSignal() :
	client([](const std::string& strLogMsg) { std::cout << strLogMsg << std::endl;  }) {
	
}

void ForeSignal::Init() {
	LoadThis();
}

void ForeSignal::Start() {
	if (ts.Elapsed() > 60*1000) {
		MailData();
		ts.Reset();
	}
}

void ForeSignal::InitMail() {
	String user = Config::email_user;
	String pass = Config::email_pass;
	String srv = Config::email_server;
	int port = Config::email_port;
	String srv_str = srv + ":" + IntStr(port);
	client.InitSession(srv_str.Begin(), user.Begin(), pass.Begin(),
		CMailClient::SettingsFlag::ALL_FLAGS, CMailClient::SslTlsFlag::ENABLE_SSL);
}

int ForeSignal::GetMailCount() {
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
	int b = s.Find(" EXISTS");
	int a = s.ReverseFind(" ", b-1) + 1;
	//int a = s.Find("UIDNEXT") + 8;
	//int b = s.Find(" ", a);
	s = s.Mid(a, b-a);
	email_count = StrInt(s);
	return email_count;
}

String ForeSignal::GetMail(int i) {
	std::string strEmail;
	String str = IntStr(i + 1);
	/* retrieve the mail number 1 and store it in strEmail */
	bool bResRcvStr = client.GetString(str.Begin(), strEmail);
	
	return strEmail;
}

void ForeSignal::MailData() {
	String mail_dir = GetOverlookFile("mail");
	RealizeDirectory(mail_dir);
	
	String user = Config::email_user;
	String pass = Config::email_pass;
	String srv = Config::email_server;
	int port = Config::email_port;
	if (!email_init && !user.IsEmpty() && !pass.IsEmpty() && !srv.IsEmpty() && port) {
		
		InitMail();
		
		email_init = true;
	}
	
	if (email_init) {
		GetMailCount();
		
		VectorMap<String, String> urls;
		for(int i = max(0, email_count - 30); i < email_count; i++) {
			String mail_file = AppendFileName(mail_dir, IntStr(i) + ".txt");
			String mail;
			if (!FileExists(mail_file)) {
				mail = GetMail(i);
				
				if (mail.GetCount()) {
					FileOut fout(mail_file);
					fout << mail;
					
					if (mail.Find("From: Foresignal.com <signal@foresignal.com>") != -1) {
						LOG(mail);
						int a = mail.Find("href") + 6;
						int b = mail.Find("\"", a);
						String url = mail.Mid(a, b-a);
						//https://foresignal.com/en/signals/one/audusd
						a = url.Find("/one/") + 5;
						b = a + 6;
						String code = url.Mid(a, b-a);
						//urls.GetAdd(code, url) = url;
						
						SingleForeSignal& signal = signals.Add();
						signal.time = GetUtcTime();
						signal.symbol = ToUpper(code);
						
						StoreThis();
						
						WhenEvent(2, signal.symbol + " signal");
					}

				}
			}
			
			//LOG("MAILLLL");
			//LOG(mail);
			
		}
	}
}


ForeSignalCtrl::ForeSignalCtrl() {
	Add(list.SizePos());
	
	list.AddColumn("Time");
	list.AddColumn("Symbol");
}

void ForeSignalCtrl::Data() {
	ForeSignal& fs = GetForeSignal();
	
	for(int i = list.GetCount(); i < fs.signals.GetCount(); i++) {
		SingleForeSignal& sig = fs.signals[i];
		list.Add(sig.time, sig.symbol);
		list.ScrollEnd();
	}
}




}
