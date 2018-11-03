#include "Client.h"
#include <plugin/jpg/jpg.h>


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
    x = border;
	y = r.top;
    h = r.GetHeight();
    w = r.GetWidth();
	c = opens.GetCount();
	diff = hi - lo;
	
	for(int i = 0; i < count; i++) {
        Vector<Point> P;
        double O, H, L, C;
        pos = c - (count + shift - i);
        if (pos >= c || pos < 0) continue;
        
        double open  = opens[pos];
        double low   = lows[pos];
        double high  = highs[pos];
		double close =
			pos+1 < c ?
				opens[pos+1] :
				open;
        
        O = (1 - (open  - lo) / diff) * h;
        H = (1 - (high  - lo) / diff) * h;
        L = (1 - (low   - lo) / diff) * h;
        C = (1 - (close - lo) / diff) * h;
		
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
}




Client::Client() {
	Icon(Images::icon());
	Title("Overlook remote client prototype");
	Sizeable().MaximizeBox().MinimizeBox();
	
	AddFrame(menu);
	menu.Set(THISBACK(MainMenu));
	
	Add(tabs.SizePos());
	tabs.Add(quotes.SizePos(), "Quotes");
	tabs.Add(graph_parent.SizePos(), "Graph");
	tabs.Add(orders_parent.SizePos(), "Orders");
	tabs.Add(hisorders_parent.SizePos(), "History");
	tabs.Add(calendar.SizePos(), "Calendar");
	tabs.Add(events_parent.SizePos(), "Events");
	tabs.Add(sent_parent.SizePos(), "Sentiment");
	
	quotes.AddColumn("Symbol");
	quotes.AddColumn("Ask");
	quotes.AddColumn("Bid");
	quotes.AddColumn("Spread");
	
	graph_parent.Add(symlist.TopPos(0,30).LeftPos(0, 200));
	graph_parent.Add(tflist.TopPos(0,30).LeftPos(200, 200));
	graph_parent.Add(graph.VSizePos(30).HSizePos());
	symlist <<= THISBACK(DataGraph);
	tflist <<= THISBACK(DataGraph);
	
	orders_parent.Add(orders_header.TopPos(0,30).HSizePos());
	orders_parent.Add(orders.VSizePos(30).HSizePos());
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
	orders.ColumnWidths ( "5 5 3 3 3 3 3 3 3 3 3 5" );
	
	hisorders_parent.Add(hisorders_header.TopPos(0,30).HSizePos());
	hisorders_parent.Add(hisorders.VSizePos(30).HSizePos());
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
	
	calendar.AddColumn("Time");
	calendar.AddColumn("Title");
	calendar.AddColumn("Currency");
	calendar.AddColumn("Impact");
	calendar.AddColumn("Forecast");
	calendar.AddColumn("Previous");
	calendar.AddColumn("Actual");
	
	events_parent.Add(events_list.SizePos());
	events_list.AddColumn("Message");
	
	sent_parent.Add(split.SizePos());
	split << historylist << curpreslist << pairpreslist << errlist << sent_console;
	sent_console.Add(comment.HSizePos().VSizePos(0, 60));
	sent_console.Add(fmlevel.BottomPos(30, 30).HSizePos());
	sent_console.Add(save.BottomPos(0, 30).HSizePos());
	historylist.AddColumn("Time");
	historylist.AddColumn("Comment");
	historylist.AddColumn("Free-margin level");
	historylist.AddColumn("Take-profit limit");
	historylist.ColumnWidths("1 3 1 1");
	historylist <<= THISBACK(LoadHistory);
	curpreslist.AddColumn("Symbol");
	curpreslist.AddColumn("Pressure");
	curpreslist.ColumnWidths("1 2");
	//curpreslist <<= THISBACK(SetCurProfile);
	pairpreslist.AddColumn("Symbol");
	pairpreslist.AddColumn("Pressure");
	pairpreslist.ColumnWidths("1 2");
	//pairpreslist <<= THISBACK(SetPairProfile);
	errlist.AddColumn("Message");
	fmlevel.SetData(0);
	save.SetLabel("Save");
	save <<= THISBACK(SaveSentiment);
	
	
	
}

Client::~Client() {
	running = false;
	if (!s.IsEmpty()) s->Close();
	while (!stopped) Sleep(100);
}

void Client::TimedRefresh() {
	
	
	switch (tabs.Get()) {
		case 0:	DataQuotes(); break;
		case 1: DataGraph(); break;
		case 2: DataOrders(); break;
		case 3: DataHistory(); break;
		case 4: DataCalendar(); break;
		case 5: DataEvents(); break;
		case 6: DataSentiment(); break;
		
	}
	tc.Set(1000, THISBACK(TimedRefresh));
}

void Client::DataInit() {
	String data;
	Get("symtf", data);
	MemStream mem((void*)data.Begin(), data.GetCount());
	
	symbols.Clear();
	tfs.Clear();
	sent_pairs.Clear();
	currencies.Clear();
	sent_currencies.Clear();
	
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
	
	int cur_count = mem.Get32();
	for(int i = 0; i < cur_count; i++) {
		int cur_len = mem.Get32();
		String cur = mem.Get(cur_len);
		currencies.Add(cur);
	}
	
	int pair_count = mem.Get32();
	for(int i = 0; i < pair_count; i++) {
		int pair_len = mem.Get32();
		String pair = mem.Get(pair_len);
		sent_pairs.Add(pair);
	}
	
	int sentcur_count = mem.Get32();
	for(int i = 0; i < sentcur_count; i++) {
		int cur_len = mem.Get32();
		String cur = mem.Get(cur_len);
		sent_currencies.Add(cur);
	}
	
}

void Client::DataQuotes() {
	if (!pending_quotes) {
		pending_quotes = true;
		
		Thread::Start([=] {
			String data;
			Get("quotes", data);
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

void Client::DataGraph() {
	if (symlist.GetCount() == 0) {
		for(int i = 0; i < symbols.GetCount(); i++)
			symlist.Add(symbols[i]);
		symlist.SetIndex(0);
		for(int i = 0; i < tfs.GetCount(); i++)
			tflist.Add(tfs[i]);
		tflist.SetIndex(0);
	}
	if (!pending_graph) {
		pending_graph = true;
		
		Thread::Start([=] {
			int sym = symlist.GetIndex();
			int tf = tflist.GetIndex();
			if (sym < 0) sym = 0;
			if (tf < 0) tf = 0;
			String data;
			Get("graph," + IntStr(sym) + "," + IntStr(tf) + "," + IntStr(graph.count), data);
			MemStream mem((void*)data.Begin(), data.GetCount());
			
			int count = mem.Get32();
			Vector<double> opens, lows, highs;
			opens.SetCount(count);
			lows.SetCount(count);
			highs.SetCount(count);
			for(int i = 0; i < count; i++) {
				double open, low, high;
				mem.Get(&open, sizeof(double));
				mem.Get(&low,  sizeof(double));
				mem.Get(&high, sizeof(double));
				opens[i] = open;
				lows[i] = low;
				highs[i] = high;
			}
			Swap(graph.highs, highs);
			Swap(graph.lows, lows);
			Swap(graph.opens, opens);
			
			pending_graph = false;
		});
	}
	
	graph.Refresh();
}

void Client::DataOrders() {
	if (!pending_open_orders) {
		pending_open_orders = true;
		
		Thread::Start([=] {
			String data;
			Get("openorders", data);
			MemStream mem((void*)data.Begin(), data.GetCount());
			
			mem.Get(&balance, sizeof(double));
			mem.Get(&equity, sizeof(double));
			mem.Get(&freemargin, sizeof(double));
			
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
	
	orders_header.SetLabel(Format("Balance: %f Equity: %f Free-margin: %f Profit: %f", balance, equity, freemargin, equity - balance));
	
	for(int i = 0; i < open_orders.GetCount(); i++) {
		const Order& o = open_orders[i];
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
	}
}

void Client::DataHistory() {
	if (!pending_history_orders) {
		pending_history_orders = true;
		
		Thread::Start([=] {
			String data;
			Get("historyorders", data);
			MemStream mem((void*)data.Begin(), data.GetCount());
			
			mem.Get(&balance, sizeof(double));
			mem.Get(&equity, sizeof(double));
			mem.Get(&freemargin, sizeof(double));
			
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
	
	hisorders_header.SetLabel(Format("Balance: %f Equity: %f Free-margin: %f Profit: %f", balance, equity, freemargin, equity - balance));
	
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

void Client::DataCalendar() {
	if (!pending_history_orders) {
		pending_calendar = true;
		
		Thread::Start([=] {
			String data;
			Get("calendar", data);
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

void Client::DataEvents() {
	if (!pending_poll) {
		pending_poll = true;
		
		Thread::Start([=] {
			Poll();
			pending_poll = false;
		});
	}
	
	event_lock.Enter();
	for(int i = 0; i < events.GetCount(); i++) {
		String e = events[i];
		events_list.Set(i, 0, e);
		//events_list.SetDisplay(i, 0, Single<CalendarTimeDisplay>());
	}
	event_lock.Leave();
}

void Client::RefreshSentimentList() {
	String data;
	Get("senthist", data);
	MemStream mem((void*)data.Begin(), data.GetCount());
	
	int count = mem.Get32();
	senthist_list.SetCount(count);
	for(int i = 0; i < count; i++) {
		SentimentSnapshot& snap = senthist_list[i];
		int cur_count = mem.Get32();
		snap.cur_pres.SetCount(cur_count);
		for(int j = 0; j < cur_count; j++)
			snap.cur_pres[j] = mem.Get32();
		int pair_count = mem.Get32();
		snap.pair_pres.SetCount(pair_count);
		for(int j = 0; j < pair_count; j++)
			snap.pair_pres[j] = mem.Get32();
		int comment_len = mem.Get32();
		snap.comment = mem.Get(comment_len);
		snap.added = Time(1970,1,1) + mem.Get32();
		mem.Get(&snap.fmlevel, sizeof(double));
		mem.Get(&snap.tplimit, sizeof(double));
	}
	
	pending_senthist = false;
}

void Client::DataSentiment() {
	
	if (senthist_list.IsEmpty() && !pending_senthist) {
		pending_senthist = true;
		
		Thread::Start([=] {
			RefreshSentimentList();
		});
	}
	
	
	ASSERT(!currencies.IsEmpty());
	ASSERT(!sent_pairs.IsEmpty());
	
	if (curpreslist.GetCount() == 0) {
		curpreslist.SetLineCy(30);
		for(int i = 0; i < sent_currencies.GetCount(); i++) {
			curpreslist.Set(i, 0, sent_currencies[i]);
			SentPresCtrl& ctrl = cur_pres_ctrl.Add();
			ctrl <<= THISBACK(SetCurPairPressures);
			curpreslist.SetCtrl(i, 1, ctrl);
		}
		
		
		pairpreslist.SetLineCy(30);
		for(int i = 0; i < sent_pairs.GetCount(); i++) {
			pairpreslist.Set(i, 0, sent_pairs[i]);
			pairpreslist.SetCtrl(i, 1, pair_pres_ctrl.Add());
		}
		
		LoadHistory();
	}
	
	
	int count = senthist_list.GetCount();
	for(int i = 0; i < count; i++) {
		SentimentSnapshot& snap = senthist_list[i];
		
		historylist.Set(i, 0, snap.added);
		historylist.Set(i, 1, snap.comment);
		historylist.Set(i, 2, snap.fmlevel);
		historylist.Set(i, 3, snap.tplimit);
	}
	historylist.SetCount(count);
	
	int his = historylist.GetCursor();
	if (count && (his < 0 || his >= count))
		historylist.SetCursor(historylist.GetCount()-1);
	
	
	Thread::Start([=] {
		SentimentSnapshot snap;
		snap.pair_pres.SetCount(pairpreslist.GetCount());
		for(int i = 0; i < pairpreslist.GetCount(); i++)
			snap.pair_pres[i] = pairpreslist.Get(i, 1);
		Index<EventError> errors;
		GetErrorList(snap, errors);
		
		GuiLock __;
		for(int i = 0; i < errors.GetCount(); i++) {
			const EventError& e = errors[i];
			String msg;
			switch (e.level) {
				case 0: msg += "info: "; break;
				case 1: msg += "warning: "; break;
				case 2: msg += "error: "; break;
			}
			msg += e.msg;
			errlist.Set(i, 0, msg);
		}
		errlist.SetCount(errors.GetCount());
	});
}

void Client::GetErrorList(SentimentSnapshot& snap, Index<EventError>& errors) {
	StringStream out;
	out << "errorlist,";
	PutSent(snap, out);
	out.Seek(0);
	String data;
	Get(out.Get(out.GetSize()), data);
	MemStream mem((void*)data.Begin(), data.GetCount());
	
	int error_count = mem.Get32();
	for(int i = 0; i < error_count; i++) {
		EventError e;
		int msg_len = mem.Get32();
		e.msg = mem.Get(msg_len);
		e.level = mem.Get32();
		e.time = Time(1970,1,1) + mem.Get32();
		errors.Add(e);
	}
}

void Client::SendSentiment(SentimentSnapshot& snap) {
	StringStream out;
	out << "sendsent,";
	PutSent(snap, out);
	out.Seek(0);
	String data;
	Get(out.Get(out.GetSize()), data);
}

void Client::PutSent(SentimentSnapshot& snap, Stream& out) {
	out.Put32(snap.cur_pres.GetCount());
	for(int j = 0; j < snap.cur_pres.GetCount(); j++)
		out.Put32(snap.cur_pres[j]);
	out.Put32(snap.pair_pres.GetCount());
	for(int j = 0; j < snap.pair_pres.GetCount(); j++)
		out.Put32(snap.pair_pres[j]);
	out.Put32(snap.comment.GetCount());
	out.Put(snap.comment);
	out.Put32(snap.added.Get() - Time(1970,1,1).Get());
	out.Put(&snap.fmlevel, sizeof(double));
	out.Put(&snap.tplimit, sizeof(double));
}

void Client::LoadHistory() {
	int his = historylist.GetCursor();
	if (his == -1) return;
	
	SentimentSnapshot& snap = senthist_list[his];
	
	
	for(int i = 0; i < curpreslist.GetCount() && i < snap.cur_pres.GetCount(); i++) {
		curpreslist.Set(i, 1, snap.cur_pres[i]);
	}
	
	for(int i = 0; i < pairpreslist.GetCount() && i < snap.pair_pres.GetCount(); i++) {
		pairpreslist.Set(i, 1, snap.pair_pres[i]);
	}
	
	comment.SetData(snap.comment);
	
	fmlevel.SetData(snap.fmlevel);
}

void Client::SaveSentiment() {
	SentimentSnapshot snap;
	
	snap.added = GetUtcTime();
	
	snap.cur_pres.SetCount(curpreslist.GetCount());
	for(int i = 0; i < curpreslist.GetCount(); i++) {
		snap.cur_pres[i] = curpreslist.Get(i, 1);
	}
	
	snap.pair_pres.SetCount(pairpreslist.GetCount());
	for(int i = 0; i < pairpreslist.GetCount(); i++) {
		snap.pair_pres[i] = pairpreslist.Get(i, 1);
	}
	
	snap.comment = comment.GetData();
	snap.fmlevel = fmlevel.GetData();
	
	SendSentiment(snap);
	
	RefreshSentimentList();
	DataSentiment();
	historylist.SetCursor(historylist.GetCount()-1);
}

void Client::SetCurPairPressures() {
	VectorMap<String, int> pres;

	for(int i = 0; i < curpreslist.GetCount(); i++) {
		String a = curpreslist.Get(i, 0);
		int p = curpreslist.Get(i, 1);
		pres.Add(a, p);
	}

	for(int i = 0; i < pairpreslist.GetCount(); i++) {
		String s = pairpreslist.Get(i, 0);
		String a = s.Left(3);
		String b = s.Right(3);
		int p = pres.GetAdd(a, 0) - pres.GetAdd(b, 0);
		pairpreslist.Set(i, 1, p);
	}


}

void Client::MainMenu(Bar& bar) {
	bar.Sub("File", [=](Bar& bar) {
		
	});
	bar.Sub("Help", [=](Bar& bar) {
		
	});
	
}

bool Client::Connect() {
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

void Client::Disconnect() {
	s.Clear();
}

bool Client::RegisterScript() {
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

bool Client::LoginScript() {
	if (!is_logged_in) {
		try {
			Login();
			is_logged_in = true;
			PostCallback(THISBACK(RefreshGui));
		}
		catch (Exc e) {
			return false;
		}
	}
	return true;
}

void Client::HandleConnection() {
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

void Client::Call(Stream& out, Stream& in) {
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

void Client::Register() {
	StringStream out, in;
	
	out.Put32(10);
	
	Call(out, in);
	
	in.Get(&user_id, sizeof(int));
	pass = in.Get(8);
	if (pass.GetCount() != 8) throw Exc("Invalid password");
	
	Print("Client " + IntStr(user_id) + " registered (pass " + pass + ")");
}

void Client::Login() {
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

bool Client::Set(const String& key, const String& value) {
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

void Client::Get(const String& key, String& value) {
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

void Client::Poll() {
	StringStream out, in;
	
	out.Put32(50);
	
	out.Put64(login_id);
	
	Call(out, in);
	
	lock.Enter();
	
	int count = in.Get32();
	if (count < 0 || count >= 10000) {
		lock.Leave(); throw Exc("Polling failed");}
	for(int i = 0; i < count; i++) {
		int msg_len = in.Get32();
		String message;
		if (msg_len > 0)
			message = in.Get(msg_len);
		if (message.GetCount() != msg_len) {lock.Leave(); throw Exc("Polling failed");}
		Print("Client " + IntStr(user_id) + " received: " + IntStr(message.GetCount()));
		
		int j = message.Find(" ");
		if (j == -1) continue;
		String key = message.Left(j);
		message = message.Mid(j + 1);
		
		if (key == "info" || key == "warning" || key == "error") {
			event_lock.Enter();
			events.Add(key + " " + message);
			event_lock.Leave();
			PostCallback(THISBACK(RefreshGui));
		}
	}
	
	
	lock.Leave();
}


void Client::RefreshGui() {
	lock.Enter();
	
	lock.Leave();
}

void Client::Command(String cmd) {
	if (cmd.IsEmpty()) return;
	
	if (cmd[0] == '/') {
		Vector<String> args = Split(cmd.Mid(1), " ");
		if (args.IsEmpty()) return;
		String key = args[0];
		/*if (key == "join") {
			if (args.GetCount() < 2) return;
			String ch_name = args[1];
			Join(ch_name);
			irc.SetActiveChannel(ch_name);
			try {
				RefreshUserlist();
			}
			catch (Exc e) {
			
			}
			RefreshGuiChannel();
		}*/
	}
}



























int PasswordFilter(int c) {
	return '*';
}

ServerDialog::ServerDialog(Client& c) : cl(c), rd(*this) {
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
	srv_addr = addr.GetData();
	srv_port = port.GetData();
	cl.SetAddress(srv_addr, srv_port);
	cl.LoadThis();
	cl.CloseConnection();
	return cl.Connect() && cl.RegisterScript() && (!do_login || cl.LoginScript());
}

void ServerDialog::StopConnect() {
	cl.CloseConnection();
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
		username.SetData(sd.cl.GetUserId());
		password.SetData(sd.cl.GetPassword());
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


