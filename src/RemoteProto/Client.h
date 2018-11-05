#ifndef _Client_Client_h
#define _Client_Client_h

#include <CtrlLib/CtrlLib.h>
using namespace Upp;

#define LAYOUTFILE <RemoteProto/Client.lay>
#include <CtrlCore/lay.h>

#define IMAGECLASS Images
#define IMAGEFILE <RemoteProto/Client.iml>
#include <Draw/iml_header.h>


void Print(const String& s);

struct QuotesData : Moveable<QuotesData> {
	String symbol;
	double ask, bid, point;
};

struct CandlestickCtrl : public Ctrl {
	Vector<double> opens, lows, highs;
	int shift = 0;
	int border = 5;
	int count = 100;
	
	virtual void Paint(Draw& d);
};

struct Order : Moveable<Order> {
	int ticket;
	Time begin, end;
	int type;
	double size;
	String symbol;
	double open, stoploss, takeprofit;
	double close, commission, swap, profit;
};

struct CalEvent : public Moveable<CalEvent> {
	int id = -1;
	Time timestamp = Null;
	char impact = 0, direction = 0;
	String title, unit, currency, forecast, previous, actual;
	
	CalEvent() {}
	CalEvent(const CalEvent& e) {*this = e;}
	CalEvent& operator=(const CalEvent& e);
	String ToInlineString() const;
	String GetImpactString() const;
	int GetSignalDirection();
	
	bool operator () (const CalEvent& a, const CalEvent& b) const {return a.timestamp < b.timestamp;}
	
};

struct CalendarTimeDisplay : Display {
	virtual void Paint(Draw& w, const Rect& r, const Value& q,
		               Color ink, Color paper, dword style) const
	{
		Time t = q;
		String str = Format("%", t);
		w.DrawRect(r, paper);
		Point pt = r.TopLeft();
		Font fnt = StdFont();
		if (t >= GetSysTime()) fnt.Bold();
		w.DrawText(pt.x, pt.y+1, str, fnt, ink);
	}
};


struct CalendarCurrencyDisplay : Display {
	virtual void Paint(Draw& w, const Rect& r, const Value& q,
		               Color ink, Color paper, dword style) const
	{
		String cur = q;
		if (cur == "GBP") paper = Color(240, 216, 255);
		else if (cur == "EUR") paper = Color(200, 202, 255);
		else if (cur == "USD") paper = Color(208, 255, 219);
		else if (cur == "JPY") paper = Color(255, 208, 205);
		else if (cur == "AUD") paper = Color(255, 255, 188);
		else if (cur == "CAD") paper = Color(255, 212, 187);
		else if (cur == "CHF") paper = Color(205, 188, 216);
		else if (cur == "NZD") paper = Color(205, 255, 255);
		w.DrawRect(r, paper);
		Point pt = r.TopLeft();
		Font fnt = StdFont();
		w.DrawText(pt.x, pt.y+1, cur, fnt, Black());
	}
};


struct CalendarImpactDisplay : Display {
	virtual void Paint(Draw& w, const Rect& r, const Value& q,
		               Color ink, Color paper, dword style) const
	{
		String imp = q;
		if (imp == "Low") paper = Color(132, 255, 94);
		else if (imp == "Medium") paper = Color(255, 255, 117);
		else if (imp == "High") paper = Color(255, 115, 102);
		w.DrawRect(r, paper);
		Point pt = r.TopLeft();
		Font fnt = StdFont();
		w.DrawText(pt.x, pt.y+1, imp, fnt, Black());
	}
};

class SentPresCtrl : public Ctrl {
	int i = 0;
	
public:
	virtual void Paint(Draw& w);
	virtual void LeftDown(Point p, dword keyflags);
	virtual void LeftUp(Point p, dword keyflags);
	virtual void MouseMove(Point p, dword keyflags);

	
	virtual Value GetData() const {return i;}
	virtual void SetData(const Value& v) {i = v;}
};

struct SentimentSnapshot : Moveable<SentimentSnapshot> {
	Vector<int> cur_pres, pair_pres;
	String comment;
	Time added;
	double fmlevel = 0.0, tplimit = 0.1;
	double equity = 0.0;
	
	void Serialize(Stream& s) {s % cur_pres % pair_pres % comment % added % fmlevel % tplimit % equity;}
	
	bool IsPairEqual(const SentimentSnapshot& s) {
		if (s.pair_pres.GetCount() != pair_pres.GetCount())
			return false;
		for(int i = 0; i < pair_pres.GetCount(); i++)
			if (pair_pres[i] != s.pair_pres[i])
				return false;
		return true;
	}
};

struct EventError : Moveable<EventError> {
	String msg;
	byte level = 0;
	Time time;
	
	EventError() {}
	EventError(const EventError& e) {*this = e;}
	void operator=(const EventError& e) {msg = e.msg; level = e.level; time = e.time;}
	unsigned GetHashValue() const {return msg.GetHashValue();}
	void Serialize(Stream& s) {s % msg % level % time;}
	bool operator==(const EventError& e) const {return msg == e.msg;}
};

static const int SIGNALSCALE = 4;

class Client : public TopWindow {
	
	static const bool continuous = false;
	
	// Persistent
	int user_id = -1;
	String pass;
	bool is_registered = false;
	
	// Session
	Index<String> symbols;
	Index<String> sent_pairs;
	Index<String> currencies;
	Index<String> sent_currencies;
	VectorMap<int, String> tfs;
	String addr = "127.0.0.1";
	One<TcpSocket> s;
	double balance, equity, freemargin;
	int64 login_id = 0;
	int port = 17000;
	bool is_logged_in = false;
	bool running = false, stopped = true;
	TimeCallback tc;
	Mutex call_lock, lock;
	
	// Gui
	MenuBar menu;
	TabCtrl tabs;
	ArrayCtrl nearestlist;
	
	ArrayCtrl quotes;
	bool pending_quotes = false;
	Vector<QuotesData> quote_values;
	
	ParentCtrl graph_parent;
	DropList symlist, tflist;
	CandlestickCtrl graph;
	bool pending_graph = false;
	
	ParentCtrl orders_parent;
	Label orders_header;
	ArrayCtrl orders;
	Vector<Order> open_orders;
	bool pending_open_orders = false;
	
	ParentCtrl hisorders_parent;
	Label hisorders_header;
	ArrayCtrl hisorders;
	Vector<Order> history_orders;
	bool pending_history_orders = false;
	Mutex hisorders_lock;
	
	ArrayCtrl calendar;
	bool pending_calendar = false;
	Vector<CalEvent> cal_events;
	Mutex calendar_lock;
	
	ParentCtrl events_parent;
	ArrayCtrl events_list;
	Vector<String> events;
	Mutex event_lock;
	bool pending_poll = false;
	
	ParentCtrl sent_parent;
	Splitter split;
	ArrayCtrl historylist, curpreslist, pairpreslist, errlist;
	EditDouble fmlevel;
	ParentCtrl sent_console;
	::Upp::DocEdit comment;
	Button save;
	Array<SentPresCtrl> pair_pres_ctrl, cur_pres_ctrl;
	Vector<SentimentSnapshot> senthist_list;
	bool pending_senthist = false;
	void LoadHistory();
	void SetCurPairPressures();
	void SaveSentiment();
	void GetErrorList(SentimentSnapshot& snap, Index<EventError>& errors);
	void SendSentiment(SentimentSnapshot& snap);
	void PutSent(SentimentSnapshot& snap, Stream& out);
	void RefreshSentimentList();
	
public:
	typedef Client CLASSNAME;
	Client();
	~Client();
	
	void PostInit() {PostCallback(THISBACK(DataInit)); tc.Set(1000, THISBACK(TimedRefresh));}
	void TimedRefresh();
	void DataInit();
	void DataQuotes();
	void DataGraph();
	void DataOrders();
	void DataHistory();
	void DataCalendar();
	void DataEvents();
	void DataSentiment();
	
	int GetUserId() const {return user_id;}
	String GetPassword() const {return pass;}
	
	void Serialize(Stream& s) {s % user_id % pass % is_registered;}
	void StoreThis() {StoreToFile(*this, ConfigFile("Client" + IntStr64(GetServerHash()) + ".bin"));}
	void LoadThis() {LoadFromFile(*this, ConfigFile("Client" + IntStr64(GetServerHash()) + ".bin"));}
	unsigned GetServerHash() {CombineHash h; h << addr << port; return h;}
	
	void MainMenu(Bar& bar);
	bool Connect();
	void Disconnect();
	void CloseConnection() {if (!s.IsEmpty()) s->Close(); is_logged_in = false;}
	bool LoginScript();
	bool RegisterScript();
	void HandleConnection();
	void Start() {if (!stopped) return; stopped = false; running = true; Thread::Start(THISBACK(HandleConnection));}
	void Call(Stream& out, Stream& in);
	void SetAddress(String a, int p) {addr = a; port = p;}
	
	void Register();
	void Login();
	bool Set(const String& key, const String& value);
	void Get(const String& key, String& value);
	void Poll();
	
	void RefreshGui();
	void RefreshGuiChannel();
	void RefreshNearest();
	void Command(String cmd);
	
	bool IsConnected() {return is_logged_in;}
	
};

class PreviewImage : public ImageCtrl {
	void SetData(const Value& val) {
		String path = val;
		if(IsNull(path.IsEmpty()))
			SetImage(Null);
		else
			SetImage(StreamRaster::LoadFileAny(~path));
	}
};

class ServerDialog;


class RegisterDialog : public WithRegisterDialog<TopWindow> {
	ServerDialog& sd;
	
public:
	typedef RegisterDialog CLASSNAME;
	RegisterDialog(ServerDialog& sd);
	~RegisterDialog();
	
	void Register();
	void TryRegister();
};

class ServerDialog : public WithServerDialog<TopWindow> {
	
protected:
	friend class RegisterDialog;
	
	// Persistent
	String srv_addr;
	int srv_port;
	bool autoconnect = false;
	
	// Temporary
	Client& cl;
	bool connecting = false;
	RegisterDialog rd;
	
public:
	typedef ServerDialog CLASSNAME;
	ServerDialog(Client& c);
	
	void StartTryConnect() {Enable(false); Thread::Start(THISBACK(TryConnect));}
	void StartStopConnect() {Thread::Start(THISBACK(StopConnect));}
	void TryConnect();
	void StopConnect();
	void Close0() {Close();}
	void SetError(String e) {error.SetLabel(e);}
	void Enable(bool b);
	bool Connect(bool do_login);
	void Register();
	
	bool IsAutoConnect() const {return autoconnect;}
	
	void StoreThis() {StoreToFile(*this, ConfigFile("ClientServer.bin"));}
	void LoadThis() {LoadFromFile(*this, ConfigFile("ClientServer.bin"));}
	void Serialize(Stream& s) {s % srv_addr % srv_port % autoconnect;}
	
};


#endif
