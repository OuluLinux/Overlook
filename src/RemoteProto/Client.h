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
	int sym = 0, tf = 0;
	bool pending_graph = false;
	
	Vector<double> opens, lows, highs, specs;
	Vector<bool> bools;
	int shift = 0;
	int border = 5;
	int count = 100;
	
	Vector<Point> cache;
	
	typedef CandlestickCtrl CLASSNAME;
	virtual void Paint(Draw& d);
	
	void Refresh0() {Refresh();}
	void Data();
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

class Session {
	
protected:
	friend class SingleCandlestick;
	friend class Events;
	friend class MultiCandlestick;
	friend class MultiActivity;
	friend class Orders;
	friend class OpenOrderCtrl;
	friend class Client;
	
	static const bool continuous = false;
	
	// Persistent
	int user_id = -1;
	String pass;
	bool is_registered = false;
	
	
	Index<String> symbols;
	VectorMap<int, String> tfs;
	String addr = "127.0.0.1";
	One<TcpSocket> s;
	int64 login_id = 0;
	int port = 17000;
	bool is_logged_in = false;
	bool running = false, stopped = true;
	Mutex call_lock, lock;
	
	Vector<String> events;
	Mutex event_lock;
	
	Vector<Order> open_orders;
	bool pending_open_orders = false;
	
	bool pending_status = false;
	double balance, equity, freemargin;
	
public:
	typedef Session CLASSNAME;
	~Session();
	
	bool Connect();
	void Disconnect();
	void CloseConnection() {if (!s.IsEmpty()) s->Close(); is_logged_in = false;}
	bool LoginScript();
	bool RegisterScript();
	void HandleConnection();
	void Start() {if (!stopped) return; stopped = false; running = true; Thread::Start(THISBACK(HandleConnection));}
	void Call(Stream& out, Stream& in);
	void SetAddress(String a, int p) {addr = a; port = p;}
	void DataInit();
	void DataOrders();
	void DataStatus();
	
	void Register();
	void Login();
	bool Set(const String& key, const String& value);
	void Get(const String& key, String& value);
	void Poll();
	
	bool IsConnected() {return is_logged_in;}
	
	int GetUserId() const {return user_id;}
	int64 GetLoginId() const {return login_id;}
	String GetPassword() const {return pass;}
	String GetSymbol(int i) const {return symbols[i];}
	int FindSymbol(const String& s) const {return symbols.Find(s);}
	int FindSymbolLeft(const String& s) const;
	bool HasOrders(const String& sym);
	int GetOrderSig(const String& sym);
	double GetOrderVolume(const String& sym);
	double GetOrderProfit(const String& sym);
	
	void Serialize(Stream& s) {s % user_id % pass % is_registered;}
	void StoreThis() {StoreToFile(*this, ConfigFile("Client" + IntStr64(GetServerHash()) + ".bin"));}
	void LoadThis() {LoadFromFile(*this, ConfigFile("Client" + IntStr64(GetServerHash()) + ".bin"));}
	unsigned GetServerHash() {CombineHash h; h << addr << port; return h;}
	
};

inline Session& GetSession() {return Single<Session>();}


class Quotes : public ParentCtrl {
	ArrayCtrl quotes;
	bool pending_quotes = false;
	Vector<QuotesData> quote_values;
	
public:
	Quotes();
	void Data();
};

class SingleCandlestick : public ParentCtrl {
	DropList symlist, tflist;
	CandlestickCtrl graph;
	
public:
	typedef SingleCandlestick CLASSNAME;
	SingleCandlestick();
	
	void Set(int sym, int tf) {symlist.SetIndex(sym); tflist.SetIndex(tf);}
	void Data();
	void DataList();
};

class MultiCandlestick : public ParentCtrl {
	DropList tflist;
	Splitter vsplit, hsplit0, hsplit1;
	Array<CandlestickCtrl> candles;
	
public:
	typedef MultiCandlestick CLASSNAME;
	MultiCandlestick();
	
	void Data();
};

class SpeculationMatrix : public ParentCtrl {
	
	struct SpeculationMatrixCtrl : public Ctrl {
		SpeculationMatrix* m = NULL;
		virtual void Paint(Draw& d);
		virtual void LeftDown(Point p, dword keyflags);
		int Tf(int i) {
			switch (i) {
				case 0: return 0;
				case 1: return 2;
				case 2: return 4;
				case 3: return 5;
				case 4: return 6;
				default: return -1;
			}
		}
	};
	
	Splitter hsplit;
	SpeculationMatrixCtrl ctrl;
	ArrayCtrl list;
	Index<String> sym;
	Vector<int> tfs;
	Vector<bool> values;
	Vector<bool> signals;
	bool pending_data = false;
	
public:
	SpeculationMatrix();
	
	void Data();
	
	
	Callback2<int, int> WhenGraph;
	Callback2<int, bool> WhenOpenOrder;
	Callback1<int> WhenCloseOrder;
};

struct ActivityCtrl : public Ctrl {
	int sym = 0, tf = 0;
	bool pending_graph = false;
	
	Vector<double> vol, volat;
	Vector<bool> vol_bools, volat_bools;
	int shift = 0;
	int border = 5;
	int count = 100;
	
	Vector<Point> cache;
	
	typedef ActivityCtrl CLASSNAME;
	virtual void Paint(Draw& d);
	
	void Refresh0() {Refresh();}
	void Data();
};

class MultiActivity : public ParentCtrl {
	DropList tflist;
	Splitter vsplit, hsplit0, hsplit1;
	Array<ActivityCtrl> activities;
	
public:
	typedef MultiActivity CLASSNAME;
	MultiActivity();
	
	void Data();
};

class ActivityMatrix : public ParentCtrl {
	
	struct ActivityMatrixCtrl : public Ctrl {
		ActivityMatrix* m = NULL;
		virtual void Paint(Draw& d);
	};
	
	Splitter hsplit;
	ActivityMatrixCtrl ctrl;
	ArrayCtrl list;
	Index<String> sym;
	Vector<int> tfs;
	Vector<bool> vol, volat;
	bool pending_data = false;
public:
	ActivityMatrix();
	
	void Data();
};

class Orders : public ParentCtrl {
	ArrayCtrl orders;
	
public:
	Orders();
	
	void Data();
};

class HistoryOrders : public ParentCtrl {
	ArrayCtrl hisorders;
	Vector<Order> history_orders;
	bool pending_history_orders = false;
	Mutex hisorders_lock;
	
public:
	HistoryOrders();
	
	void Data();
};

class CalendarCtrl : public ParentCtrl {
	ArrayCtrl calendar;
	bool pending_calendar = false;
	Vector<CalEvent> cal_events;
	Mutex calendar_lock;
	
public:
	CalendarCtrl();
	
	void Data();
};

class Events : public ParentCtrl {
	ArrayCtrl events_list;
	bool pending_poll = false;
	
public:
	typedef Events CLASSNAME;
	Events();
	
	void Data();
};

class OpenOrderCtrl : public WithOpenOrder<ParentCtrl> {
	int sym = 0;
	bool sig = 0;
	double vol = 0.01;
	int tp_count = 30;
	int sl_count = 30;
	
public:
	typedef OpenOrderCtrl CLASSNAME;
	OpenOrderCtrl();
	
	void Set(int sym, bool sig);
	void Data();
	void OpenOrder();
	
	Callback WhenOrderSent;
};

class CloseOrderCtrl : public WithCloseOrder<ParentCtrl> {
	int sym = 0;
	
public:
	typedef CloseOrderCtrl CLASSNAME;
	CloseOrderCtrl();
	
	void Set(int sym);
	void Data();
	void CloseOrders();
	
	Callback WhenOrderSent;
};

class Client : public TopWindow {
	
	
	
	// Gui
	MenuBar menu;
	TabCtrl tabs;
	ArrayCtrl nearestlist;
	TimeCallback tc;
	
	Label status;
	
	Quotes quotes;
	SingleCandlestick single_candlestick;
	MultiCandlestick multi_candlestick;
	SpeculationMatrix specmat;
	MultiActivity multact;
	ActivityMatrix actmat;
	Orders orders;
	HistoryOrders hisorders;
	CalendarCtrl calendar;
	Events events;
	OpenOrderCtrl open_order;
	CloseOrderCtrl close_order;
	
public:
	typedef Client CLASSNAME;
	Client();
	~Client();
	
	void SetGraph(int sym, int tf);
	void OpenOrder(int sym, bool sig);
	void CloseOrder(int sym);
	void SetTab(int i) {tabs.Set(i);}
	
	void Refresher();
	void ToggleFullScreen() {TopWindow::FullScreen(!IsFullScreen());}
	void PostInit() {tc.Set(1000, THISBACK(TimedRefresh));}
	void TimedRefresh();
	void DataStatus();
	
	void MainMenu(Bar& bar);
	
	
	void RefreshGui();
	void RefreshGuiChannel();
	void RefreshNearest();
	
	
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
	bool connecting = false;
	RegisterDialog rd;
	
public:
	typedef ServerDialog CLASSNAME;
	ServerDialog();
	
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
