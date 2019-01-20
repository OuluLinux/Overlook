#include "Client.h"
#include <plugin/jpg/jpg.h>

Session::~Session() {
	running = false;
	if (!s.IsEmpty()) s->Close();
	while (!stopped) Sleep(100);
}

int Session::FindSymbolLeft(const String& s) const {
	for(int i = 0; i < symbols.GetCount(); i++) {
		const String& sym = symbols[i];
		if (sym.Left(s.GetCount()) == s)
			return i;
	}
	return -1;
}

bool Session::HasOrders(const String& sym) {
	for(int i = 0; i < open_orders.GetCount(); i++) {
		const Order& o = open_orders[i];
		if (o.symbol == sym)
			return true;
	}
	return false;
}

int Session::GetOrderSig(const String& sym) {
	for(int i = 0; i < open_orders.GetCount(); i++) {
		const Order& o = open_orders[i];
		if (o.symbol == sym)
			return o.type % 2;
	}
	return -1;
}

double Session::GetOrderVolume(const String& sym) {
	double vol = 0.0;
	for(int i = 0; i < open_orders.GetCount(); i++) {
		const Order& o = open_orders[i];
		if (o.symbol == sym)
			vol += o.size;
	}
	return vol;
}

double Session::GetOrderProfit(const String& sym) {
	double profit = 0.0;
	for(int i = 0; i < open_orders.GetCount(); i++) {
		const Order& o = open_orders[i];
		if (o.symbol == sym)
			profit += o.profit;
	}
	return profit;
}

void Session::DataInit() {
	String data;
	Get("symtf", data);
	MemStream mem((void*)data.Begin(), data.GetCount());
	
	symbols.Clear();
	tfs.Clear();
	
	int sym_count = mem.Get32();
	for(int i = 0; i < sym_count; i++) {
		int sym_len = mem.Get32();
		String sym = mem.Get(sym_len);
		symbols.Add(sym);
	}
	
	int tf_count = mem.Get32();
	for(int i = 0; i < tf_count; i++) {
		int tf = mem.Get32();
		int tf_len = mem.Get32();
		String tf_str = mem.Get(tf_len);
		tfs.Add(tf, tf_str);
	}
}

void Session::DataOrders() {
	if (!pending_open_orders) {
		pending_open_orders = true;
		
		Thread::Start([=] {
			String data;
			GetSession().Get("openorders", data);
			MemStream mem((void*)data.Begin(), data.GetCount());
			
			int order_count = mem.Get32();
			Vector<Order> tmp;
			tmp.SetCount(order_count);
			for(int i = 0; i < order_count; i++) {
				Order& o = tmp[i];
				o.ticket = mem.Get32();
				o.begin = Time(1970,1,1) + mem.Get32();
				o.end = Time(1970,1,1) + mem.Get32();
				o.type = mem.Get32();
				mem.Get(&o.size, sizeof(double));
				int sym_len = mem.Get32();
				o.symbol = mem.Get(sym_len);
				mem.Get(&o.open, sizeof(double));
				mem.Get(&o.stoploss, sizeof(double));
				mem.Get(&o.takeprofit, sizeof(double));
				mem.Get(&o.close, sizeof(double));
				mem.Get(&o.commission, sizeof(double));
				mem.Get(&o.swap, sizeof(double));
				mem.Get(&o.profit, sizeof(double));
			}
			
			Swap(tmp, this->open_orders);
			pending_open_orders = false;
		});
	}
}

void Session::DataStatus() {
	if (!pending_status) {
		pending_status = true;
		
		Thread::Start([=] {
			String data;
			GetSession().Get("status", data);
			MemStream mem((void*)data.Begin(), data.GetCount());
			
			mem.Get(&balance, sizeof(double));
			mem.Get(&equity, sizeof(double));
			mem.Get(&freemargin, sizeof(double));
		});
	}
}

bool Session::Connect() {
	if (s.IsEmpty() || !s->IsOpen()) {
		if (!s.IsEmpty()) s.Clear();
		s.Create();
		
		if(!s->Connect(addr, port)) {
			Print("Client " + IntStr(user_id) + " Unable to connect to server!");
			return false;
		}
	}
	return true;
}

void Session::Disconnect() {
	s.Clear();
}

bool Session::RegisterScript() {
	if (!is_registered) {
		try {
			Register();
			is_registered = true;
			StoreThis();
		}
		catch (Exc e) {
			return false;
		}
	}
	return true;
}

bool Session::LoginScript() {
	if (!is_logged_in) {
		try {
			Login();
			is_logged_in = true;
		}
		catch (Exc e) {
			return false;
		}
	}
	return true;
}

void Session::HandleConnection() {
	Print("Client " + IntStr(user_id) + " Running");
	
	int count = 0;
	
	
	while (!Thread::IsShutdownThreads() && running) {
		
		if (continuous)
			Connect();
		
		try {
			while (!Thread::IsShutdownThreads() && running) {
				RegisterScript();
				LoginScript();
				Poll();
				
				Sleep(1000);
				count++;
			}
		}
		catch (Exc e) {
			Print("Client " + IntStr(user_id) + " Error: " + e);
			count = min(count, 1);
		}
		catch (const char* e) {
			Print("Client " + IntStr(user_id) + " Error: " + e);
			count = min(count, 1);
		}
		catch (...) {
			Print("Client " + IntStr(user_id) + " Unexpected error");
			break;
		}
		
		is_logged_in = false;
		
		if (continuous)
			Disconnect();
	}
	
	Print("Client " + IntStr(user_id) + " Stopping");
	stopped = true;
}

void Session::Call(Stream& out, Stream& in) {
	int r;
	
	out.Seek(0);
	String out_str = out.Get(out.GetSize());
	int out_size = out_str.GetCount();
	
	call_lock.Enter();
	
	if (!continuous) {
		s.Clear();
		Connect();
	}
	
	r = s->Put(&out_size, sizeof(out_size));
	if (r != sizeof(out_size)) {if (!continuous) Disconnect(); call_lock.Leave(); throw Exc("Data sending failed");}
	r = s->Put(out_str.Begin(), out_str.GetCount());
	if (r != out_str.GetCount()) {if (!continuous) Disconnect(); call_lock.Leave(); call_lock.Leave(); throw Exc("Data sending failed");}
	
	s->Timeout(30000);
	int in_size;
	r = s->Get(&in_size, sizeof(in_size));
	if (r != sizeof(in_size) || in_size < 0 || in_size >= 10000000) {if (!continuous) Disconnect(); call_lock.Leave(); call_lock.Leave(); throw Exc("Received invalid size");}
	
	String in_data = s->Get(in_size);
	if (in_data.GetCount() != in_size) {if (!continuous) Disconnect(); call_lock.Leave(); call_lock.Leave(); throw Exc("Received invalid data");}
	
	if (!continuous)
		Disconnect();
	
	call_lock.Leave();
	
	int64 pos = in.GetPos();
	in << in_data;
	in.Seek(pos);
}

void Session::Register() {
	StringStream out, in;
	
	out.Put32(10);
	
	Call(out, in);
	
	in.Get(&user_id, sizeof(int));
	pass = in.Get(8);
	if (pass.GetCount() != 8) throw Exc("Invalid password");
	
	Print("Client " + IntStr(user_id) + " registered (pass " + pass + ")");
}

void Session::Login() {
	StringStream out, in;
	
	out.Put32(20);
	out.Put32(user_id);
	out.Put(pass.Begin(), pass.GetCount());
	
	Call(out, in);
	
	int ret = in.Get32();
	if (ret != 0) throw Exc("Login failed");
	
	login_id = in.Get64();
	
	Print("Client " + IntStr(user_id) + " logged in (" + IntStr(user_id) + ", " + pass + ")");
}

bool Session::Set(const String& key, const String& value) {
	StringStream out, in;
	
	out.Put32(30);
	
	out.Put64(login_id);
	
	out.Put32(key.GetCount());
	out.Put(key.Begin(), key.GetCount());
	out.Put32(value.GetCount());
	out.Put(value.Begin(), value.GetCount());
	
	Call(out, in);
	
	int ret = in.Get32();
	if (ret == 1) {
		Print("Client " + IntStr(user_id) + " set " + key + " FAILED");
		return false;
	}
	else if (ret != 0) throw Exc("Setting value failed");
	
	Print("Client " + IntStr(user_id) + " set " + key);
	return true;
}

void Session::Get(const String& key, String& value) {
	StringStream out, in;
	
	out.Put32(40);
	
	out.Put64(login_id);
	
	out.Put32(key.GetCount());
	out.Put(key.Begin(), key.GetCount());
	
	Call(out, in);
	
	int value_len = in.Get32();
	value = in.Get(value_len);
	if (value.GetCount() != value_len) throw Exc("Getting value failed");
	
	int ret = in.Get32();
	if (ret != 0) throw Exc("Getting value failed");
	
	Print("Client " + IntStr(user_id) + " get " + key);
}

void Session::Poll() {
	
	StringStream out, in;
	
	out.Put32(50);
	
	out.Put64(login_id);
	
	Call(out, in);
	
	int count = in.Get32();
	if (count < 0 || count >= 10000)
		throw Exc("Polling failed");
	bool new_data = false;
	for(int i = 0; i < count; i++) {
		int msg_len = in.Get32();
		String message;
		if (msg_len > 0)
			message = in.Get(msg_len);
		if (message.GetCount() != msg_len) {throw Exc("Polling failed");}
		Print("Client " + IntStr(user_id) + " received: " + IntStr(message.GetCount()));
		
		int j = message.Find(" ");
		if (j == -1) continue;
		String key = message.Left(j);
		message = message.Mid(j + 1);
		
		if (key == "info" || key == "warning" || key == "error") {
			event_lock.Enter();
			events.Add(key + " " + message);
			event_lock.Leave();
			new_data = true;
		}
	}
}






void Print(const String& s) {
	static Mutex lock;
	lock.Enter();
	Cout() << s;
	Cout().PutEol();
	LOG(s);
	lock.Leave();
}

double DegreesToRadians(double degrees) {
  return degrees * M_PI / 180.0;
}

double CoordinateDistanceKM(Pointf a, Pointf b) {
	double earth_radius_km = 6371.0;
	
	double dLat = DegreesToRadians(b.y - a.y);
	double dLon = DegreesToRadians(b.x - a.x);
	
	a.y = DegreesToRadians(a.y);
	b.y = DegreesToRadians(b.y);
	
	double d = sin(dLat/2) * sin(dLat/2) +
		sin(dLon/2) * sin(dLon/2) * cos(a.y) * cos(b.y);
	double c = 2 * atan2(sqrt(d), sqrt(1-d));
	return earth_radius_km * c;
}


#define DATAUP Color(56, 212, 150)
#define DATADOWN Color(28, 85, 150)
#define DATAUP_DARK Color(0, 138, 78)
#define DATADOWN_DARK Color(23, 58, 99)

void CandlestickCtrl::Paint(Draw& d) {
	int f, pos, x, y, h, c, w;
	double diff;
    Rect r(GetSize());
	d.DrawRect(r, White());
	
	double hi = -DBL_MAX, lo = +DBL_MAX;
	for(double d : lows)
		if (d < lo) lo = d;
	for(double d : highs)
		if (d > hi) hi = d;
	
	int div = 8;
	count = r.GetWidth() / div;
	
    f = 2;
    x = border - (div - r.GetWidth() % div);
	y = r.top;
    h = r.GetHeight();
    w = r.GetWidth();
	c = opens.GetCount();
	diff = hi - lo;
	
	d.DrawText(0, 0, GetSession().GetSymbol(sym), Arial(28));
	
	int cs_h = h * 0.7;
	for(int i = 0; i < count; i++) {
        Vector<Point> P;
        double O, H, L, C;
        pos = c - (count + shift - i);
        if (pos >= opens.GetCount() || pos < 0) continue;
        
        double open  = opens[pos];
        double low   = lows[pos];
        double high  = highs[pos];
		double close =
			pos+1 < c ?
				opens[pos+1] :
				open;
        
        O = (1 - (open  - lo) / diff) * cs_h;
        H = (1 - (high  - lo) / diff) * cs_h;
        L = (1 - (low   - lo) / diff) * cs_h;
        C = (1 - (close - lo) / diff) * cs_h;
		
		P <<
			Point((int)(x+i*div+f),		(int)(y+O)) <<
			Point((int)(x+(i+1)*div-f),	(int)(y+O)) <<
			Point((int)(x+(i+1)*div-f),	(int)(y+C)) <<
			Point((int)(x+i*div+f),		(int)(y+C)) <<
			Point((int)(x+i*div+f),		(int)(y+O));
        
        {
	        Color c, c2;
	        if (C < O) {c = DATAUP; c2 = DATAUP_DARK;}
	        else {c = DATADOWN; c2 = DATADOWN_DARK;}
	        
	        d.DrawLine(
				(int)(x+(i+0.5)*div), (int)(y+H),
				(int)(x+(i+0.5)*div), (int)(y+L),
				2, c2);
	        d.DrawPolygon(P, c, 1, c2);
        }
    }
    
    int indi_h = h - cs_h;
    y += cs_h;
    hi = -DBL_MAX;
    lo = +DBL_MAX;
	for(double d : specs) {
		if (d < lo) lo = d;
		if (d > hi) hi = d;
	}
	diff = hi - lo;
	
	cache.SetCount(0);
	
	for(int i = 0; i < count; i++) {
        Vector<Point> P;
        double O;
        pos = c - (count + shift - i);
        if (pos >= c || pos < 0) continue;
        
        double spec  = specs[pos];
        bool b = bools[pos];
        
        O = (1 - (spec  - lo) / diff) * indi_h;
		
		cache <<
			Point((int)(x+(i+0.5)*div),		(int)(y+O));
		
		int rx = x + i * div + 1;
		d.DrawRect(rx, y, div-2, div-2, b ? Color(28, 127, 255) : Color(28, 212, 0));
    }
    if (!cache.IsEmpty())
		d.DrawPolyline(cache, 1, Green());
    
    double O = (1 - (0  - lo) / diff) * indi_h;
    if (IsFin(O)) {
	    y += O;
	    d.DrawLine(0, y, w, y, 1, Black());
    }
}

void CandlestickCtrl::Data() {
	if (!pending_graph) {
		pending_graph = true;
		
		Thread::Start([=] {
			if (sym < 0) sym = 0;
			if (tf < 0) tf = 0;
			String data;
			GetSession().Get("graph," + IntStr(sym) + "," + IntStr(tf) + "," + IntStr(count), data);
			MemStream mem((void*)data.Begin(), data.GetCount());
			
			int count = mem.Get32();
			Vector<double> opens, lows, highs;
			Vector<bool> bools;
			opens.SetCount(count);
			lows.SetCount(count);
			highs.SetCount(count);
			specs.SetCount(count);
			bools.SetCount(count);
			for(int i = 0; i < count; i++) {
				double open, low, high, spec;
				bool b;
				mem.Get(&open, sizeof(double));
				mem.Get(&low,  sizeof(double));
				mem.Get(&high, sizeof(double));
				mem.Get(&spec, sizeof(double));
				mem.Get(&b, sizeof(bool));
				opens[i] = open;
				lows[i] = low;
				highs[i] = high;
				specs[i] = spec;
				bools[i] = b;
			}
			Swap(this->highs, highs);
			Swap(this->lows, lows);
			Swap(this->opens, opens);
			Swap(this->specs, specs);
			Swap(this->bools, bools);
			
			pending_graph = false;
		});
	}
	
	PostCallback(THISBACK(Refresh0));
}





Quotes::Quotes() {
	Add(quotes.SizePos());
	quotes.AddColumn("Symbol");
	quotes.AddColumn("Ask");
	quotes.AddColumn("Bid");
	quotes.AddColumn("Spread");
}

void Quotes::Data() {
	if (!pending_quotes) {
		pending_quotes = true;
		
		Thread::Start([=] {
			String data;
			GetSession().Get("quotes", data);
			MemStream mem((void*)data.Begin(), data.GetCount());
			
			Vector<QuotesData> tmp;
			int count = mem.Get32();
			tmp.SetCount(count);
			for(int i = 0; i < count; i++) {
				QuotesData& q = tmp[i];
				
				int sym_len;
				mem.Get(&sym_len, sizeof(int));
				q.symbol = mem.Get(sym_len);
				
				mem.Get(&q.ask, sizeof(double));
				mem.Get(&q.bid, sizeof(double));
				mem.Get(&q.point, sizeof(double));
			}
			Swap(tmp, quote_values);
			
			pending_quotes = false;
		});
	
	}
	
	for(int i = 0; i < quote_values.GetCount(); i++) {
		const QuotesData& q = quote_values[i];
		quotes.Set(i, 0, q.symbol);
		quotes.Set(i, 1, q.ask);
		quotes.Set(i, 2, q.bid);
		quotes.Set(i, 3, (q.ask - q.bid) / q.point);
	}
}




SingleCandlestick::SingleCandlestick() {
	Add(symlist.TopPos(0,30).LeftPos(0, 200));
	Add(tflist.TopPos(0,30).LeftPos(200, 200));
	Add(graph.VSizePos(30).HSizePos());
	
	symlist <<= THISBACK(Data);
	tflist <<= THISBACK(Data);
}

void SingleCandlestick::DataList() {
	Session& ses = GetSession();
	
	if (symlist.GetCount() == 0) {
		for(int i = 0; i < ses.symbols.GetCount(); i++)
			symlist.Add(ses.symbols[i]);
		symlist.SetIndex(0);
		for(int i = 0; i < ses.tfs.GetCount(); i++)
			tflist.Add(ses.tfs[i]);
		tflist.SetIndex(0);
	}
	
}
void SingleCandlestick::Data() {
	DataList();
	graph.sym = symlist.GetIndex();
	graph.tf = tflist.GetIndex();
	graph.Data();
}






MultiCandlestick::MultiCandlestick() {
	Add(tflist.TopPos(0,30).LeftPos(0, 200));
	Add(vsplit.VSizePos(30).HSizePos());
	vsplit.Vert();
	vsplit << hsplit0 << hsplit1;
	
	tflist <<= THISBACK(Data);
	
	CandlestickCtrl& eur = candles.Add();
	eur.sym = GetSession().symbols.Find("EUR1");
	
	CandlestickCtrl& usd = candles.Add();
	usd.sym = GetSession().symbols.Find("USD1");
	
	CandlestickCtrl& gbp = candles.Add();
	gbp.sym = GetSession().symbols.Find("GBP1");
	
	CandlestickCtrl& jpy = candles.Add();
	jpy.sym = GetSession().symbols.Find("JPY1");
	
	CandlestickCtrl& chf = candles.Add();
	chf.sym = GetSession().symbols.Find("CHF1");
	
	CandlestickCtrl& cad = candles.Add();
	cad.sym = GetSession().symbols.Find("CAD1");
	
	hsplit0 << eur << usd << gbp;
	hsplit1 << jpy << chf << cad;
}

void MultiCandlestick::Data() {
	Session& ses = GetSession();
	
	if (tflist.GetCount() == 0) {
		for(int i = 0; i < ses.tfs.GetCount(); i++)
			tflist.Add(ses.tfs[i]);
		tflist.SetIndex(0);
	}
	
	for(int i = 0; i < candles.GetCount(); i++) {
		candles[i].tf = tflist.GetIndex();
		candles[i].Data();
	}
}





SpeculationMatrix::SpeculationMatrix() {
	Add(hsplit.SizePos());
	hsplit.Horz();
	hsplit << ctrl << list;
	hsplit.SetPos(8000);
	ctrl.m = this;
	
	sym.Add("EUR");
	sym.Add("USD");
	sym.Add("GBP");
	sym.Add("JPY");
	sym.Add("CHF");
	sym.Add("CAD");
	sym.Add("EURUSD");
	sym.Add("EURGBP");
	sym.Add("EURJPY");
	sym.Add("EURCHF");
	sym.Add("EURCAD");
	sym.Add("GBPUSD");
	sym.Add("USDJPY");
	sym.Add("USDCHF");
	sym.Add("USDCAD");
	sym.Add("GBPJPY");
	sym.Add("GBPCHF");
	sym.Add("GBPCAD");
	sym.Add("CHFJPY");
	sym.Add("CADJPY");
	sym.Add("CADCHF");
	
	tfs.Add(0);
	tfs.Add(2);
	tfs.Add(4);
	tfs.Add(5);
	tfs.Add(6);
	
	values.SetCount(sym.GetCount() * tfs.GetCount(), false);
	signals.SetCount(sym.GetCount(), false);
	
	list.AddColumn("Symbol");
	list.AddColumn("Sum");
	list.AddColumn("Sig");
}

void SpeculationMatrix::Data() {
	if (!pending_data) {
		pending_data = true;
		
		Thread::Start([=] {
			String data;
			GetSession().Get("speculation", data);
			MemStream mem((void*)data.Begin(), data.GetCount());
			
			Vector<bool> tmp;
			tmp.SetCount(sym.GetCount() * tfs.GetCount());
			int row = 0;
			for(int i = 0; i < sym.GetCount(); i++) {
				for(int j = 0; j < tfs.GetCount(); j++) {
					bool b;
					mem.Get(&b, sizeof(bool));
					tmp[row++] = b;
				}
			}
			
			Swap(tmp, values);
			
			pending_data = false;
		});
	
	}
	
	Refresh();
	
	int cursor = 0;
	if (list.IsCursor()) cursor = list.GetCursor();
	for(int i = 6; i < sym.GetCount(); i++) {
		String s = sym[i];
		String a = s.Left(3);
		String b = s.Mid(3,3);
		int ai = sym.Find(a);
		int bi = sym.Find(b);
		int sum = 0;
		for(int j = 0; j < tfs.GetCount(); j++) {
			bool av = values[ai * tfs.GetCount() + j];
			bool bv = values[bi * tfs.GetCount() + j];
			bool v = values[i * tfs.GetCount() + j];
			sum += v ? -1 : +1;
			int ab = (av ? -1 : +1) + (bv ? +1 : -1);
			if (ab) sum += ab < 0 ? -1 : +1;
		}
		list.Set(i-6, 0, s);
		list.Set(i-6, 1, abs(sum));
		list.Set(i-6, 2, sum > 0 ? "Buy" : "Sell");
		signals[i] = sum < 0;
	}
	list.SetSortColumn(1, true);
	list.SetCursor(cursor);
}

void SpeculationMatrix::SpeculationMatrixCtrl::Paint(Draw& d) {
	Session& ses = GetSession();
	
	Rect r(GetSize());
	d.DrawRect(GetSize(), White());
	
	int grid_count = 7;
	double row = r.GetHeight() / (double)grid_count;
	double col = r.GetWidth() / (double)grid_count;
	double subrow = row / 2;
	double subsubrow = row / 4;
	double subcol = col / m->tfs.GetCount();
	Font fnt = Arial(row / 3);
	
	for(int i = 0; i < 6; i++) {
		String a = m->sym[i];
		
		int x = 0 + 2;
		int y = (1 + i) * row;
		d.DrawText(x, y, a, fnt);
		
		y += subrow;
		for(int j = 0; j < m->tfs.GetCount(); j++) {
			bool b = m->values[i * m->tfs.GetCount() + j];
			x = j * subcol;
			d.DrawRect(x, y, subcol + 1, subrow + 1, b ? Color(56, 127, 255) : Color(28, 212, 0));
		}
		
		x = (1 + i) * col;
		y = 0 + 2;
		d.DrawText(x, y, a, fnt);
		y = subrow;
		for(int j = 0; j < m->tfs.GetCount(); j++) {
			bool b = m->values[i * m->tfs.GetCount() + j];
			x = (1 + i) * col + j * subcol;
			d.DrawRect(x, y, subcol + 1, subrow + 1, b ? Color(56, 127, 255) : Color(28, 212, 0));
		}
		
		
		for(int j = 0; j < 6; j++) {
			if (i == j) continue;
			String b = m->sym[j];
			String ab = a + b;
			String ba = b + a;
			String s;
			bool is_ab = m->sym.Find(ab) != -1;
			if (is_ab)
				s = ab;
			else
				s = ba;
			int sympos = m->sym.Find(s);
			
			int mtsym_id = ses.FindSymbolLeft(s);
			ASSERT(mtsym_id != 0);
			String mtsym = ses.GetSymbol(mtsym_id);
			bool has_orders = ses.HasOrders(mtsym);
			if (has_orders) {
				bool sig = ses.GetOrderSig(mtsym);
				d.DrawRect(x, y, col + 1, row + 1, sig ? Color(170, 255, 255) : Color(216, 255, 164));
			}
			
			x = (1 + i) * col + 2;
			y = (1 + j) * row + 2;
			d.DrawText(x, y, s, fnt);
			for(int k = 0; k < m->tfs.GetCount(); k++) {
				x = (1 + i) * col + k * subcol;
				y = (1 + j) * row + subrow;
				
				bool b = m->values[sympos * m->tfs.GetCount() + k];
				d.DrawRect(x, y, subcol + 1, subsubrow + 1, b ? Color(56, 127, 255) : Color(28, 212, 0));
				
				y = (1 + j) * row + subrow + subsubrow;
				bool av = m->values[i * m->tfs.GetCount() + k];
				bool bv = m->values[j * m->tfs.GetCount() + k];
				int sum = 0;
				if (is_ab) {
					sum += av ? -1 : +1;
					sum += bv ? +1 : -1;
				} else {
					sum += av ? +1 : -1;
					sum += bv ? -1 : +1;
				}
				Color c;
				if (sum > 0)		c = Color(28, 212, 0);
				else if (sum < 0)	c = Color(56, 127, 255);
				else				c = GrayColor();
				d.DrawRect(x, y, subcol + 1, subsubrow + 1, c);
			}
		}
	}
	
	for(int i = 0; i < 6; i++) {
		int y = (1 + i) * row;
		int x = (1 + i) * col;
		int y2 = (1 + i) * row + subrow;
		int y3 = (1 + i) * row + subrow + subsubrow;
		int y4 = (1 + i + 1) * row;
		
		for(int j = 0; j < 6; j++) {
			if (j == i) continue;
			for(int k = 1; k < m->tfs.GetCount(); k++) {
				int x = (1 + j) * col + k * subcol;
				d.DrawLine(x, y2, x, y4, 1, GrayColor());
			}
		}
		
		for(int j = 1; j < m->tfs.GetCount(); j++) {
			int x = j * subcol;
			d.DrawLine(x, y2, x, y4, 1, GrayColor());
			x = (1 + i) * col + j * subcol;
			d.DrawLine(x, subrow, x, row, 1, GrayColor());
		}
		
		d.DrawLine(0, y2, r.GetWidth(), y2, 1, GrayColor());
		d.DrawLine(col, y3, r.GetWidth(), y3, 1, GrayColor());
		d.DrawLine(0, y, r.GetWidth(), y, 1, Black());
		d.DrawLine(x, 0, x, r.GetHeight(), 1, Black());
	}
}

void SpeculationMatrix::SpeculationMatrixCtrl::LeftDown(Point p, dword keyflags) {
	Rect rect(GetSize());
	
	int grid_count = 7;
	double row = rect.GetHeight() / (double)grid_count;
	double col = rect.GetWidth() / (double)grid_count;
	double subrow = row / 2;
	double subcol = col / m->tfs.GetCount();
	
	int c = p.x / col;
	int r = p.y / row;
	
	if (c == 0 && r == 0) {
		
	}
	else if (c == 0 && r > 0) {
		int sym = r - 1;
		String s = m->sym[sym] + "1";
		sym = GetSession().FindSymbol(s);
		int tf = Tf((p.x - c * col) / subcol);
		bool is_subtf = (p.y - r * row) / subrow >= 1.0;
		if (is_subtf)
			m->WhenGraph(sym, tf);
	}
	else if (r == 0 && c > 0) {
		int sym = c - 1;
		String s = m->sym[sym] + "1";
		sym = GetSession().FindSymbol(s);
		int tf = Tf((p.x - c * col) / subcol);
		bool is_subtf = (p.y - r * row) / subrow >= 1.0;
		if (is_subtf)
			m->WhenGraph(sym, tf);
	}
	else {
		int ai = r - 1;
		int bi = c - 1;
		if (ai != bi) {
			String a = m->sym[ai];
			String b = m->sym[bi];
			int sym = GetSession().FindSymbolLeft(a + b);
			if (sym == -1) sym = GetSession().FindSymbolLeft(b + a);
			String mtsym = GetSession().GetSymbol(sym);
			bool has_orders = GetSession().HasOrders(mtsym);
			
			bool sig = m->signals[m->sym.Find(mtsym.Left(6))];
			
			int tf = Tf((p.x - c * col) / subcol);
			bool is_subtf = (p.y - r * row) / subrow >= 1.0;
			if (is_subtf)
				m->WhenGraph(sym, tf);
			else {
				if (!has_orders)
					m->WhenOpenOrder(sym, sig);
				else
					m->WhenCloseOrder(sym);
			}
		}
	}
}







void ActivityCtrl::Paint(Draw& d) {
	int f, pos, x, y, h, c, w, shift;
	double diff;
    Rect r(GetSize());
	d.DrawRect(r, White());
	
	double hi = -DBL_MAX, lo = +DBL_MAX;
	for(double d : vol) {
		if (d < lo) lo = d;
		if (d > hi) hi = d;
	}
	
	int div = 8;
	count = r.GetWidth() / div;
	shift = count / 2;
	
    f = 2;
    x = border - (div - r.GetWidth() % div);
	y = r.top;
    h = r.GetHeight();
    w = r.GetWidth();
	c = vol.GetCount();
	diff = hi - lo;
	
	d.DrawText(0, 0, GetSession().GetSymbol(sym), Arial(28));
	
	
	int vol_h = h * 0.5;
	cache.SetCount(0);
	for(int i = 0; i < count; i++) {
        Vector<Point> P;
        double O, H, L, C;
        pos = c - (count + this->shift - i);
        if (pos >= vol.GetCount() || pos < 0) continue;
        
        double v  = vol[pos];
        bool b = vol_bools[pos];
        
		double V = (1 - (v  - lo) / diff) * vol_h;
		
		cache << Point((int)(x+(i+0.5)*div), (int)(y+V));
		
		int rx = x + i * div + 1;
		if (b)
			d.DrawRect(rx, y, div-2, div-2, Color(226, 42, 0));
    }
    if (!cache.IsEmpty())
		d.DrawPolyline(cache, 2, Green());
    
    y += vol_h;
    hi = -DBL_MAX;
    lo = +DBL_MAX;
	for(double d : volat) {
		if (d < lo) lo = d;
		if (d > hi) hi = d;
	}
	diff = hi - lo;
	
	cache.SetCount(0);
	for(int i = 0; i < count; i++) {
        Vector<Point> P;
        double O, H, L, C;
        pos = c - (count + this->shift - i);
        if (pos >= vol.GetCount() || pos < 0) continue;
        
        double v  = volat[pos];
        bool b = volat_bools[pos];
        
		double V = (1 - (v  - lo) / diff) * vol_h;
		
		cache << Point((int)(x+(i+0.5)*div), (int)(y+V));
		
		int rx = x + i * div + 1;
		if (b)
			d.DrawRect(rx, y, div-2, div-2, Color(226, 42, 0));
    }
    if (!cache.IsEmpty())
		d.DrawPolyline(cache, 2, Green());
    
    x += (count - shift + 0.5) * div;
    d.DrawLine(x, 0, x, r.GetHeight(), 2, Black());
}

void ActivityCtrl::Data() {
	if (!pending_graph) {
		pending_graph = true;
		
		Thread::Start([=] {
			if (sym < 0) sym = 0;
			if (tf < 0) tf = 0;
			String data;
			GetSession().Get("vol," + IntStr(sym) + "," + IntStr(tf) + "," + IntStr(count), data);
			MemStream mem((void*)data.Begin(), data.GetCount());
			
			int count = mem.Get32();
			Vector<double> vol, volat;
			Vector<bool> vol_bools, volat_bools;
			vol.SetCount(count);
			volat.SetCount(count);
			vol_bools.SetCount(count);
			volat_bools.SetCount(count);
			for(int i = 0; i < count; i++) {
				double v, va;
				bool vb, vab;
				mem.Get(&v, sizeof(double));
				mem.Get(&va,  sizeof(double));
				mem.Get(&vb, sizeof(bool));
				mem.Get(&vab, sizeof(bool));
				vol[i] = v;
				volat[i] = va;
				vol_bools[i] = vb;
				volat_bools[i] = vab;
			}
			Swap(this->vol, vol);
			Swap(this->volat, volat);
			Swap(this->vol_bools, vol_bools);
			Swap(this->volat_bools, volat_bools);
			
			pending_graph = false;
		});
	}
	
	PostCallback(THISBACK(Refresh0));
}




MultiActivity::MultiActivity() {
	Add(tflist.TopPos(0,30).LeftPos(0, 200));
	Add(vsplit.VSizePos(30).HSizePos());
	vsplit.Vert();
	vsplit << hsplit0 << hsplit1;
	
	tflist <<= THISBACK(Data);
	
	ActivityCtrl& eur = activities.Add();
	eur.sym = GetSession().symbols.Find("EUR1");
	
	ActivityCtrl& usd = activities.Add();
	usd.sym = GetSession().symbols.Find("USD1");
	
	ActivityCtrl& gbp = activities.Add();
	gbp.sym = GetSession().symbols.Find("GBP1");
	
	ActivityCtrl& jpy = activities.Add();
	jpy.sym = GetSession().symbols.Find("JPY1");
	
	ActivityCtrl& chf = activities.Add();
	chf.sym = GetSession().symbols.Find("CHF1");
	
	ActivityCtrl& cad = activities.Add();
	cad.sym = GetSession().symbols.Find("CAD1");
	
	hsplit0 << eur << usd << gbp;
	hsplit1 << jpy << chf << cad;
}

void MultiActivity::Data() {
	Session& ses = GetSession();
	
	if (tflist.GetCount() == 0) {
		for(int i = 0; i < ses.tfs.GetCount(); i++)
			tflist.Add(ses.tfs[i]);
		tflist.SetIndex(2);
	}
	
	for(int i = 0; i < activities.GetCount(); i++) {
		activities[i].tf = tflist.GetIndex();
		activities[i].Data();
	}
}


Color ActivityColor(int i) {
	switch (i) {
		case 0: return White();
		case 1: return Color(255, 139, 139);
		case 2: return Color(255, 42, 0);
		case 3: return Color(255, 127, 0);
		case 4: return Color(255, 254, 0);
		default: return GrayColor();
	}
}

void ActivityMatrix::ActivityMatrixCtrl::Paint(Draw& d) {
	Rect r(GetSize());
	d.DrawRect(GetSize(), White());
	
	int grid_count = 7;
	double row = r.GetHeight() / (double)grid_count;
	double col = r.GetWidth() / (double)grid_count;
	double subrow = row / 2;
	double subsubrow = row / 4;
	double subcol = col / m->tfs.GetCount();
	Font fnt = Arial(row / 3);
	
	for(int i = 0; i < 6; i++) {
		String a = m->sym[i];
		
		int x = 0 + 2;
		int y = (1 + i) * row;
		d.DrawText(x, y, a, fnt);
		
		y += subrow;
		for(int j = 0; j < m->tfs.GetCount(); j++) {
			bool vb = m->vol[i * m->tfs.GetCount() + j];
			bool vab = m->volat[i * m->tfs.GetCount() + j];
			x = j * subcol;
			d.DrawRect(x, y, subcol + 1, subrow + 1, ActivityColor(vb + vab));
		}
		
		x = (1 + i) * col;
		y = 0 + 2;
		d.DrawText(x, y, a, fnt);
		y = subrow;
		for(int j = 0; j < m->tfs.GetCount(); j++) {
			bool vb = m->vol[i * m->tfs.GetCount() + j];
			bool vab = m->volat[i * m->tfs.GetCount() + j];
			x = (1 + i) * col + j * subcol;
			d.DrawRect(x, y, subcol + 1, subrow + 1, ActivityColor(vb + vab));
		}
		
		
		for(int j = 0; j < 6; j++) {
			if (i == j) continue;
			String b = m->sym[j];
			String ab = a + b;
			String ba = b + a;
			String s;
			bool is_ab = m->sym.Find(ab) != -1;
			if (is_ab)
				s = ab;
			else
				s = ba;
			int sympos = m->sym.Find(s);
			
			x = (1 + i) * col + 2;
			y = (1 + j) * row + 2;
			d.DrawText(x, y, s, fnt);
			for(int k = 0; k < m->tfs.GetCount(); k++) {
				x = (1 + i) * col + k * subcol;
				y = (1 + j) * row + subrow;
				
				bool vb = m->vol[sympos * m->tfs.GetCount() + k];
				bool vab = m->volat[sympos * m->tfs.GetCount() + k];
				d.DrawRect(x, y, subcol + 1, subsubrow + 1, ActivityColor(vb + vab));
				
				y = (1 + j) * row + subrow + subsubrow;
				bool av = m->vol[i * m->tfs.GetCount() + k];
				bool bv = m->vol[j * m->tfs.GetCount() + k];
				bool ava = m->volat[i * m->tfs.GetCount() + k];
				bool bva = m->volat[j * m->tfs.GetCount() + k];
				d.DrawRect(x, y, subcol + 1, subsubrow + 1, ActivityColor(av + bv + ava + bva));
			}
		}
	}
	
	for(int i = 0; i < 6; i++) {
		int y = (1 + i) * row;
		int x = (1 + i) * col;
		int y2 = (1 + i) * row + subrow;
		int y3 = (1 + i) * row + subrow + subsubrow;
		int y4 = (1 + i + 1) * row;
		
		for(int j = 0; j < 6; j++) {
			if (j == i) continue;
			for(int k = 1; k < m->tfs.GetCount(); k++) {
				int x = (1 + j) * col + k * subcol;
				d.DrawLine(x, y2, x, y4, 1, GrayColor());
			}
		}
		
		for(int j = 1; j < m->tfs.GetCount(); j++) {
			int x = j * subcol;
			d.DrawLine(x, y2, x, y4, 1, GrayColor());
			x = (1 + i) * col + j * subcol;
			d.DrawLine(x, subrow, x, row, 1, GrayColor());
		}
		
		d.DrawLine(0, y2, r.GetWidth(), y2, 1, GrayColor());
		d.DrawLine(col, y3, r.GetWidth(), y3, 1, GrayColor());
		d.DrawLine(0, y, r.GetWidth(), y, 1, Black());
		d.DrawLine(x, 0, x, r.GetHeight(), 1, Black());
	}
}




ActivityMatrix::ActivityMatrix() {
	Add(hsplit.SizePos());
	hsplit.Horz();
	hsplit << ctrl << list;
	hsplit.SetPos(8000);
	ctrl.m = this;
	
	sym.Add("EUR");
	sym.Add("USD");
	sym.Add("GBP");
	sym.Add("JPY");
	sym.Add("CHF");
	sym.Add("CAD");
	sym.Add("EURUSD");
	sym.Add("EURGBP");
	sym.Add("EURJPY");
	sym.Add("EURCHF");
	sym.Add("EURCAD");
	sym.Add("GBPUSD");
	sym.Add("USDJPY");
	sym.Add("USDCHF");
	sym.Add("USDCAD");
	sym.Add("GBPJPY");
	sym.Add("GBPCHF");
	sym.Add("GBPCAD");
	sym.Add("CHFJPY");
	sym.Add("CADJPY");
	sym.Add("CADCHF");
	
	tfs.Add(0);
	tfs.Add(2);
	tfs.Add(4);
	tfs.Add(5);
	
	vol.SetCount(sym.GetCount() * tfs.GetCount(), false);
	volat.SetCount(sym.GetCount() * tfs.GetCount(), false);
	
	list.AddColumn("Symbol");
	list.AddColumn("Sum");
}

void ActivityMatrix::Data() {
	if (!pending_data) {
		pending_data = true;
		
		Thread::Start([=] {
			String data;
			GetSession().Get("activity", data);
			MemStream mem((void*)data.Begin(), data.GetCount());
			
			Vector<bool> vol_tmp, volat_tmp;
			vol_tmp.SetCount(sym.GetCount() * tfs.GetCount());
			volat_tmp.SetCount(sym.GetCount() * tfs.GetCount());
			int row = 0;
			for(int i = 0; i < sym.GetCount(); i++) {
				for(int j = 0; j < tfs.GetCount(); j++) {
					bool vol_b;
					mem.Get(&vol_b, sizeof(bool));
					vol_tmp[row] = vol_b;
					bool volat_b;
					mem.Get(&volat_b, sizeof(bool));
					volat_tmp[row++] = volat_b;
				}
			}
			
			Swap(vol_tmp, vol);
			Swap(volat_tmp, volat);
			
			pending_data = false;
		});
	
	}
	
	Refresh();
	
	int cursor = 0;
	if (list.IsCursor()) cursor = list.GetCursor();
	for(int i = 6; i < sym.GetCount(); i++) {
		String s = sym[i];
		String a = s.Left(3);
		String b = s.Mid(3,3);
		int ai = sym.Find(a);
		int bi = sym.Find(b);
		int sum = 0;
		for(int j = 0; j < tfs.GetCount(); j++) {
			{
				bool av = vol[ai * tfs.GetCount() + j];
				bool bv = vol[bi * tfs.GetCount() + j];
				bool v = vol[i * tfs.GetCount() + j];
				sum += v ? 1 : 0;
				sum += (av ? 1 : 0) + (bv ? 1 : 0);
			} {
				bool av = volat[ai * tfs.GetCount() + j];
				bool bv = volat[bi * tfs.GetCount() + j];
				bool v = volat[i * tfs.GetCount() + j];
				sum += v ? 1 : 0;
				sum += (av ? 1 : 0) + (bv ? 1 : 0);
			}
		}
		list.Set(i-6, 0, s);
		list.Set(i-6, 1, abs(sum));
	}
	list.SetSortColumn(1, true);
	list.SetCursor(cursor);
}





Orders::Orders() {
	Add(orders.SizePos());
	orders.AddColumn ( "Order" );
	orders.AddColumn ( "Time" );
	orders.AddColumn ( "Type" );
	orders.AddColumn ( "Size" );
	orders.AddColumn ( "Symbol" );
	orders.AddColumn ( "Price" );
	orders.AddColumn ( "S / L" );
	orders.AddColumn ( "T / P" );
	orders.AddColumn ( "Price" );
	orders.AddColumn ( "Commission" );
	orders.AddColumn ( "Swap" );
	orders.AddColumn ( "Profit" );
	orders.AddColumn ( "Profit / 0.01lot" );
	orders.ColumnWidths ( "5 5 3 3 3 3 3 3 3 3 3 5 3" );
	
}

void Orders::Data() {
	Session& ses = GetSession();
	
	//ses.DataOrders();
	
	for(int i = 0; i < ses.open_orders.GetCount(); i++) {
		const Order& o = ses.open_orders[i];
		orders.Set(i, 0, o.ticket);
		orders.Set(i, 1, o.begin);
		orders.Set(i, 2, o.type);
		orders.Set(i, 3, o.size);
		orders.Set(i, 4, o.symbol);
		orders.Set(i, 5, o.open);
		orders.Set(i, 6, o.stoploss);
		orders.Set(i, 7, o.takeprofit);
		orders.Set(i, 8, o.close);
		orders.Set(i, 9, o.commission);
		orders.Set(i, 10, o.swap);
		orders.Set(i, 11, o.profit);
		orders.Set(i, 12, o.profit / (o.size / 0.01));
	}
}





HistoryOrders::HistoryOrders() {
	Add(hisorders.SizePos());
	hisorders.AddColumn ( "Order" );
	hisorders.AddColumn ( "Time" );
	hisorders.AddColumn ( "Type" );
	hisorders.AddColumn ( "Size" );
	hisorders.AddColumn ( "Symbol" );
	hisorders.AddColumn ( "Price" );
	hisorders.AddColumn ( "S / L" );
	hisorders.AddColumn ( "T / P" );
	hisorders.AddColumn ( "Price" );
	hisorders.AddColumn ( "Commission" );
	hisorders.AddColumn ( "Swap" );
	hisorders.AddColumn ( "Profit" );
	hisorders.ColumnWidths ( "5 5 3 3 3 3 3 3 3 3 3 5" );
}

void HistoryOrders::Data() {
	
	if (!pending_history_orders) {
		pending_history_orders = true;
		
		Thread::Start([=] {
			String data;
			GetSession().Get("historyorders", data);
			MemStream mem((void*)data.Begin(), data.GetCount());
			
			int order_count = mem.Get32();
			Vector<Order> tmp;
			tmp.SetCount(order_count);
			for(int i = 0; i < order_count; i++) {
				Order& o = tmp[i];
				o.ticket = mem.Get32();
				o.begin = Time(1970,1,1) + mem.Get32();
				o.end = Time(1970,1,1) + mem.Get32();
				o.type = mem.Get32();
				mem.Get(&o.size, sizeof(double));
				int sym_len = mem.Get32();
				o.symbol = mem.Get(sym_len);
				mem.Get(&o.open, sizeof(double));
				mem.Get(&o.stoploss, sizeof(double));
				mem.Get(&o.takeprofit, sizeof(double));
				mem.Get(&o.close, sizeof(double));
				mem.Get(&o.commission, sizeof(double));
				mem.Get(&o.swap, sizeof(double));
				mem.Get(&o.profit, sizeof(double));
			}
			
			hisorders_lock.Enter();
			Swap(tmp, this->history_orders);
			hisorders_lock.Leave();
			pending_history_orders = false;
		});
	}
	
	hisorders_lock.Enter();
	for(int i = 0; i < history_orders.GetCount(); i++) {
		const Order& o = history_orders[i];
		hisorders.Set(i, 0, o.ticket);
		hisorders.Set(i, 1, o.begin);
		hisorders.Set(i, 2, o.type);
		hisorders.Set(i, 3, o.size);
		hisorders.Set(i, 4, o.symbol);
		hisorders.Set(i, 5, o.open);
		hisorders.Set(i, 6, o.stoploss);
		hisorders.Set(i, 7, o.takeprofit);
		hisorders.Set(i, 8, o.close);
		hisorders.Set(i, 9, o.commission);
		hisorders.Set(i, 10, o.swap);
		hisorders.Set(i, 11, o.profit);
	}
	hisorders_lock.Leave();
}





CalendarCtrl::CalendarCtrl() {
	Add(calendar.SizePos());
	calendar.AddColumn("Time");
	calendar.AddColumn("Title");
	calendar.AddColumn("Currency");
	calendar.AddColumn("Impact");
	calendar.AddColumn("Forecast");
	calendar.AddColumn("Previous");
	calendar.AddColumn("Actual");
	
}

void CalendarCtrl::Data() {
	
	if (!pending_calendar) {
		pending_calendar = true;
		
		Thread::Start([=] {
			String data;
			GetSession().Get("calendar", data);
			MemStream mem((void*)data.Begin(), data.GetCount());
			
			
			int ev_count = mem.Get32();
			Vector<CalEvent> tmp;
			tmp.SetCount(ev_count);
			for(int i = 0; i < ev_count; i++) {
				CalEvent& ev = tmp[i];
				ev.id = mem.Get32();
				ev.timestamp = Time(1970,1,1) + mem.Get32();
				ev.impact = mem.Get32();
				ev.direction = mem.Get32();
				int l;
				l = mem.Get32();	ev.title = mem.Get(l);
				l = mem.Get32();	ev.unit = mem.Get(l);
				l = mem.Get32();	ev.currency = mem.Get(l);
				l = mem.Get32();	ev.forecast = mem.Get(l);
				l = mem.Get32();	ev.previous = mem.Get(l);
				l = mem.Get32();	ev.actual = mem.Get(l);
			}
			
			calendar_lock.Enter();
			Swap(tmp, this->cal_events);
			calendar_lock.Leave();
			pending_calendar = false;
		});
	}
		
	
	int utc_diff = GetSysTime().Get() - GetUtcTime().Get();
	
	calendar_lock.Enter();
	for(int i = 0; i < cal_events.GetCount(); i++) {
		const CalEvent& e = cal_events[i];
		calendar.Set(i, 0, e.timestamp + utc_diff);
		calendar.SetDisplay(i, 0, Single<CalendarTimeDisplay>());
		calendar.Set(i, 1, e.title);
		calendar.Set(i, 2, e.currency);
		calendar.SetDisplay(i, 2, Single<CalendarCurrencyDisplay>());
		calendar.Set(i, 3, e.GetImpactString());
		calendar.SetDisplay(i, 3, Single<CalendarImpactDisplay>());
		calendar.Set(i, 4, e.forecast + e.unit);
		calendar.Set(i, 5, e.previous + e.unit);
		calendar.Set(i, 6, e.actual + e.unit);
	}
	calendar_lock.Leave();
}








Events::Events() {
	Add(events_list.SizePos());
	events_list.AddColumn("Message");
}

void Events::Data() {
	if (!pending_poll) {
		pending_poll = true;
		
		Thread::Start([=] {
			try {
				GetSession().Poll();
			}
			catch (Exc e) {
			    
			}
			pending_poll = false;
		});
	}
	
	Session& s = GetSession();
	s.event_lock.Enter();
	for(int i = 0; i < s.events.GetCount(); i++) {
		String e = s.events[i];
		events_list.Set(i, 0, e);
		//events_list.SetDisplay(i, 0, Single<CalendarTimeDisplay>());
	}
	s.event_lock.Leave();
}






OpenOrderCtrl::OpenOrderCtrl() {
	CtrlLayout(*this);
	open << THISBACK(OpenOrder);
}

void OpenOrderCtrl::Set(int sym, bool sig) {
	this->sym = sym;
	this->sig = sig;
	open.Enable(true);
}

void OpenOrderCtrl::Data() {
	Session& ses = GetSession();
	
	String s = ses.GetSymbol(sym);
	
	this->symbol.SetLabel(s);
	this->signal.SetLabel(sig ? "Sell" : "Buy");
	vol = ses.balance / 10000.0;
	if (vol < 0.01) vol = 0.01;
	this->volume.SetLabel(Format("%2n", vol));
	this->tp.SetLabel(IntStr(tp_count));
	this->sl.SetLabel(IntStr(sl_count));
}

void OpenOrderCtrl::OpenOrder() {
	
	open.Enable(false);
	
	Thread::Start([=] {
		String data;
		GetSession().Get(
			"openorder," +
			IntStr(sym) + "," +
			IntStr(sig) + "," +
			Format("%2n", vol) + "," +
			IntStr(tp_count) + "," +
			IntStr(sl_count)
			,
			data);
		MemStream mem((void*)data.Begin(), data.GetCount());
		
	});
	
	WhenOrderSent();
}







CloseOrderCtrl::CloseOrderCtrl() {
	CtrlLayout(*this);
	
	close << THISBACK(CloseOrders);
}

void CloseOrderCtrl::Set(int sym) {
	this->sym = sym;
	close.Enable(true);
}

void CloseOrderCtrl::Data() {
	Session& ses = GetSession();
	
	String s = ses.GetSymbol(sym);
	
	this->symbol.SetLabel(s);
	
	double vol = ses.GetOrderVolume(s);
	this->volume.SetLabel(Format("%2n", vol));
	
	double profit = ses.GetOrderProfit(s);
	this->profit.SetLabel(Format("%2n", profit));
}

void CloseOrderCtrl::CloseOrders() {
	
	close.Enable(false);
	
	Thread::Start([=] {
		String data;
		GetSession().Get(
			"closeorders," +
			IntStr(sym)
			,
			data);
		MemStream mem((void*)data.Begin(), data.GetCount());
		
	});
	
	WhenOrderSent();
}






Client::Client() {
	Icon(Images::icon());
	Title("Overlook remote");
	Sizeable().MaximizeBox().MinimizeBox();
	
	AddFrame(menu);
	menu.Set(THISBACK(MainMenu));
	
	Add(status.BottomPos(0,30).HSizePos());
	status.SetFont(Arial(28));
	
	Add(tabs.HSizePos().VSizePos(0,30));
	tabs.Add(quotes.SizePos(), "Quotes");
	tabs.Add(single_candlestick.SizePos(), "Graph");
	tabs.Add(multi_candlestick.SizePos(), "MultiGraph");
	tabs.Add(specmat.SizePos(), "Speculation");
	tabs.Add(multact.SizePos(), "MultiActivity");
	tabs.Add(actmat.SizePos(), "Activity");
	tabs.Add(orders.SizePos(), "Orders");
	tabs.Add(hisorders.SizePos(), "History");
	tabs.Add(calendar.SizePos(), "Calendar");
	tabs.Add(events.SizePos(), "Events");
	tabs.Add(open_order.SizePos(), "Open");
	tabs.Add(close_order.SizePos(), "Close");
	
	specmat.WhenGraph << THISBACK(SetGraph);
	specmat.WhenOpenOrder << THISBACK(OpenOrder);
	specmat.WhenCloseOrder << THISBACK(CloseOrder);
	open_order.WhenOrderSent << THISBACK1(SetTab, 6);
	close_order.WhenOrderSent << THISBACK1(SetTab, 6);
}

Client::~Client() {
	
}

void Client::SetGraph(int sym, int tf) {
	tabs.Set(1);
	single_candlestick.DataList();
	single_candlestick.Set(sym, tf);
	single_candlestick.Data();
}

void Client::OpenOrder(int sym, bool sig) {
	tabs.Set(10);
	open_order.Set(sym, sig);
	open_order.Data();
}

void Client::CloseOrder(int sym) {
	tabs.Set(11);
	close_order.Set(sym);
	close_order.Data();
}

void Client::TimedRefresh() {
	
	DataStatus();
	GetSession().DataOrders();
	
	switch (tabs.Get()) {
		case 0:	quotes.Data(); break;
		case 1: single_candlestick.Data(); break;
		case 2: multi_candlestick.Data(); break;
		case 3: specmat.Data(); break;
		case 4: multact.Data(); break;
		case 5: actmat.Data(); break;
		case 6: orders.Data(); break;
		case 7: hisorders.Data(); break;
		case 8: calendar.Data(); break;
		case 9: events.Data(); break;
		case 10: open_order.Data(); break;
		case 11: close_order.Data(); break;
		
	}
	tc.Set(1000, THISBACK(TimedRefresh));
}

void Client::DataStatus() {
	Session& ses = GetSession();
	ses.DataStatus();
	
	String s;
	Time now = GetUtcTime();
	s << "Time: " << Format("%", now);
	s << " Bal: " << Format("%2n", ses.balance);
	s << " Eq: " << Format("%2n", ses.equity);
	s << " Fm: " << Format("%2n", ses.freemargin);
	s << " Pr: " << Format("%2n", ses.equity - ses.balance);
	status.SetLabel(s);
}

void Client::MainMenu(Bar& bar) {
	bar.Sub("File", [=](Bar& bar) {
		bar.Add("Full Screen", THISBACK(ToggleFullScreen)).Key(K_F11);
	});
	bar.Sub("Help", [=](Bar& bar) {
		
	});
	
}

































int PasswordFilter(int c) {
	return '*';
}

ServerDialog::ServerDialog() : rd(*this) {
	CtrlLayout(*this, "F2F select server");
	
	password.SetFilter(PasswordFilter);
	
	LoadThis();
	
	addr.SetData(srv_addr);
	port.SetData(srv_port);
	auto_connect.Set(autoconnect);
	
	reg <<= THISBACK(Register);
	
	addr.SetFocus();
	Enable(true);
}

void ServerDialog::Enable(bool b) {
	reg.Enable(b);
	username.Enable(b);
	password.Enable(b);
	addr.Enable(b);
	port.Enable(b);
	auto_connect.Enable(b);
	connect.Enable(b);
	if (b) {
		connect.WhenAction = THISBACK(StartTryConnect);
		connect.SetLabel(t_("Connect"));
	} else {
		connect.WhenAction = THISBACK(StartStopConnect);
		connect.SetLabel(t_("Stop"));
	}
}

void ServerDialog::TryConnect() {
	autoconnect = auto_connect.Get();
	srv_addr = addr.GetData();
	srv_port = port.GetData();
	StoreThis();
	
	if (srv_addr.IsEmpty())
		PostCallback(THISBACK1(SetError, "Server address is empty"));
	else if (Connect(true))
		PostCallback(THISBACK(Close0));
	else
		PostCallback(THISBACK1(SetError, "Connecting server failed"));
	PostCallback(THISBACK1(Enable, true));
}

bool ServerDialog::Connect(bool do_login) {
	Session& ses = GetSession();
	srv_addr = addr.GetData();
	srv_port = port.GetData();
	ses.SetAddress(srv_addr, srv_port);
	ses.LoadThis();
	ses.CloseConnection();
	return ses.Connect() && ses.RegisterScript() && (!do_login || ses.LoginScript());
}

void ServerDialog::StopConnect() {
	GetSession().CloseConnection();
}

void ServerDialog::Register() {
	rd.Register();
	if (rd.Execute() == IDOK) {
		username.SetData(rd.username.GetData());
		password.SetData(rd.password.GetData());
		StoreThis();
	}
}






















RegisterDialog::RegisterDialog(ServerDialog& sd) : sd(sd) {
	CtrlLayoutOKCancel(*this, "Register to server");
}

RegisterDialog::~RegisterDialog() {
	
}
	
void RegisterDialog::Register() {
	Thread::Start(THISBACK(TryRegister));
}

void RegisterDialog::TryRegister() {
	if (sd.Connect(false)) {
		GuiLock __;
		Session& ses = GetSession();
		username.SetData(ses.GetUserId());
		password.SetData(ses.GetPassword());
	}
}







String CalEvent::GetImpactString() const {
	switch (impact) {
		case 1:  return "Low";
		case 2:  return "Medium";
		case 3:  return "High";
		default: return "";
	}
}













void SentPresCtrl::Paint(Draw& w) {
	Size sz(GetSize());
	
	w.DrawRect(sz, White());
	
	int value = GetData();
	
	if (value < 0.0) {
		int W = -value * (sz.cx / 2) / SIGNALSCALE;
		w.DrawRect(sz.cx / 2 - W, 0, W, sz.cy, Blue());
	} else {
		int W = value * (sz.cx / 2) / SIGNALSCALE;
		w.DrawRect(sz.cx / 2, 0, W, sz.cy, Green());
	}
	
	w.DrawRect(sz.cx/2, 0, 1, sz.cy, GrayColor());
}

void SentPresCtrl::LeftDown(Point p, dword keyflags) {
	Size sz(GetSize());
	int value = (p.x - sz.cx / 2) * SIGNALSCALE / (sz.cx / 2);
	SetData(value);
	Refresh();
	WhenAction();
	SetCapture();
}

void SentPresCtrl::LeftUp(Point p, dword keyflags) {
	ReleaseCapture();
}

void SentPresCtrl::MouseMove(Point p, dword keyflags) {
	if (HasCapture()) {
		Size sz(GetSize());
		int value = (p.x - sz.cx / 2) * SIGNALSCALE / (sz.cx / 2);
		SetData(value);
		Refresh();
		WhenAction();
	}
}


