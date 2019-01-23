#include "Overlook.h"

namespace Overlook {

ActiveSession::ActiveSession() {
	
}

void ActiveSession::Print(const String& s) {
	if (last_user_id < 0 || last_login_id == 0) return;
	UserDatabase& db = GetDatabase(last_user_id);
	db.lock.Enter();
	UserSessionLog& ses = db.sessions.GetAdd(last_login_id);
	db.lock.Leave();
	if (ses.begin == Time(1970,1,1)) ses.begin = GetSysTime();
	ses.log.Add().Set(s);
}

void ActiveSession::Run() {
	Print("Session " + IntStr(sess_id) + " started handling socket from " + s.GetPeerAddr());
	int count = 0;
	
	struct NoPrintExc : public String {NoPrintExc(String s) : String(s) {}};
	
	String hex("0123456789ABCDEF");
	try {
		
		while (s.IsOpen()) {
			int r;
			int in_size = 0;
			r = s.Get(&in_size, sizeof(in_size));
			if (r != sizeof(in_size) || in_size < 0 || in_size >= 10000000) throw NoPrintExc("Received invalid size " + IntStr(in_size));
			
			String in_data = s.Get(in_size);
			if (in_data.GetCount() != in_size) throw Exc("Received invalid data");
			
			/*String hexdump;
			for(int i = 0; i < in_data.GetCount(); i++) {
				byte b = in_data[i];
				hexdump.Cat(hex[b >> 4]);
				hexdump.Cat(hex[b & 0xF]);
			}
			LOG("in: " + hexdump);*/
			
			MemReadStream in(in_data.Begin(), in_data.GetCount());
			StringStream out;
			
			int code;
			int test = SwapEndian32(-939524096);
			r = in.Get(&code, sizeof(code));
			if (r != sizeof(code)) throw Exc("Received invalid code");
			
			
			switch (code) {
				case 0:			Greeting(in, out); break;
				case 10:		Register(in, out); break;
				case 20:		Login(in, out); break;
				case 40:		Get(in, out); break;
				case 50:		Poll(in, out); break;
				
				default:
					throw Exc("Received invalid code " + IntStr(code));
			}
			
			out.Seek(0);
			String out_str = out.Get(out.GetSize());
			int out_size = out_str.GetCount();
			
			/*hexdump = "";
			for(int i = 0; i < out_str.GetCount(); i++) {
				byte b = out_str[i];
				hexdump.Cat(hex[b >> 4]);
				hexdump.Cat(hex[b & 0xF]);
			}
			LOG("out " + IntStr(out_str.GetCount()) + ":" + hexdump);*/
			
			
			r = s.Put(&out_size, sizeof(out_size));
			if (r != sizeof(out_size)) throw Exc("Data sending failed");
			r = s.Put(out_str.Begin(), out_str.GetCount());
			if (r != out_str.GetCount()) throw Exc("Data sending failed");
			
			count++;
		}
	}
	catch (NoPrintExc e) {
		
	}
	catch (Exc e) {
		Print("Error processing client from: " + s.GetPeerAddr() + " Reason: " + e);
	}
	catch (const char* e) {
		Print("Error processing client from: " + s.GetPeerAddr() + " Reason: " + e);
	}
	catch (...) {
		Print("Error processing client from: " + s.GetPeerAddr());
	}
	
	Logout();
	
	s.Close();
	
	stopped = true;
}

void ActiveSession::Greeting(Stream& in, Stream& out) {
	String title = Config::server_title;
	int i = title.GetCount();
	out.Put(&i, sizeof(int));
	out.Put(title.Begin(), title.GetCount());
	i = server->sessions.GetCount();
	out.Put(&i, sizeof(int));
	i = Config::server_max_sessions;
	out.Put(&i, sizeof(int));
}

void ActiveSession::Register(Stream& in, Stream& out) {
	
	server->lock.EnterWrite();
	
	int user_id = server->db.GetUserCount();
	String name = "User" + IntStr(user_id);
	
	String pass = RandomPassword(8);
	int64 passhash = pass.GetHashValue();
	Time now = GetUtcTime();
	
	UserDatabase& db = GetDatabase(user_id);
	
	server->db.AddUser(user_id, name);
	server->db.Flush();
	
	db.passhash = passhash;
	db.joined = now;
	db.lastlogin = Time(1970,1,1);
	db.logins = 0;
	db.onlinetotal = 0;
	db.visibletotal = 0;
	db.lastupdate = Time(1970,1,1);
	db.Flush();
	
	last_user_id = user_id;
	
	server->lock.LeaveWrite();
	
	out.Put(&user_id, sizeof(user_id));
	out.Put(pass.Begin(), 8);
	
	Print("Registered " + IntStr(user_id) + " " + pass + " (hash " + IntStr64(passhash) + ")");
}

void ActiveSession::Login(Stream& in, Stream& out) {
	try {
		int user_id;
		int r = in.Get(&user_id, sizeof(user_id));
		if (r != sizeof(user_id)) throw Exc("Invalid login id");
		
		if (user_id < 0 || user_id >= server->db.GetUserCount()) throw Exc("Invalid login id");
		UserDatabase& db = GetDatabase(user_id);
		last_user_id = user_id;
		
		String pass = in.Get(8);
		if (pass.GetCount() != 8) throw Exc("Invalid login password");
		int64 passhash = pass.GetHashValue();
		
		int64 correct_passhash = db.passhash;
		if (passhash != correct_passhash || !correct_passhash) throw Exc("Invalid login password");
		
		server->lock.EnterWrite();
		server->user_session_ids.GetAdd(user_id, sess_id) = sess_id;
		int64 login_id = server->GetNewLoginId();
		server->login_session_ids.GetAdd(login_id, user_id);
		server->lock.LeaveWrite();
			
		db.logins++;
		db.Flush();
		
		out.Put32(0);
		
		out.Put64(login_id);
		
		Print("Logged in");
	}
	 catch (Exc e) {
	    out.Put32(1);
	    Print("Login failed");
	 }
}

void ActiveSession::Logout() {
	if (last_user_id < 0)
		return;
	
	server->lock.EnterWrite();
	server->user_session_ids.RemoveKey(last_user_id);
	server->lock.LeaveWrite();
	
	if (last_login_id != 0) {
		UserDatabase& db = GetDatabase(last_user_id);
		UserSessionLog& ses = db.sessions.GetAdd(last_login_id);
		ses.end = GetSysTime();
	}
}

int ActiveSession::LoginId(Stream& in) {
	int64 login_id;
	int r = in.Get(&login_id, sizeof(login_id));
	if (r != sizeof(login_id)) throw Exc("Invalid login id");
	
	server->lock.EnterRead();
	int i = server->login_session_ids.Find(login_id);
	int user_id = -1;
	if (i >= 0) user_id = server->login_session_ids[i];
	server->lock.LeaveRead();
	if (user_id < 0)
		throw Exc("Invalid login id");
	
	last_login_id = login_id;
	last_user_id = user_id;
	
	return user_id;
}

void ActiveSession::Get(Stream& in, Stream& out) {
	System& sys = GetSystem();
	int user_id = LoginId(in);
	
	int r;
	int key_len;
	String key;
	r = in.Get(&key_len, sizeof(key_len));
	if (r != sizeof(key_len) || key_len < 0 || key_len > 200) throw Exc("Invalid key argument");
	key = in.Get(key_len);
	if (key.GetCount() != key_len) throw Exc("Invalid key received");
	
	const UserDatabase& db = GetDatabase(user_id);
	
	int64 size_pos = out.GetPos();
	out.SeekCur(sizeof(int));
	
	int i = key.Find(",");
	Vector<String> args;
	String data;
	if (i != -1) {
		data = key.Mid(i+1);
		args = Split(data, ",");
		key = key.Left(i);
	}
	
	if (key == "symtf") {
		System& sys = GetSystem();
		
		int sym_count = sys.GetSymbolCount();
		out.Put32(sym_count);
		for(int i = 0; i < sym_count; i++) {
			String sym = sys.GetSymbol(i);
			out.Put32(sym.GetCount());
			out.Put(sym);
		}
		
		int tf_count = sys.GetPeriodCount();
		out.Put32(tf_count);
		for(int i = 0; i < tf_count; i++) {
			int tf = sys.GetPeriod(i);
			String tf_str = sys.GetPeriodString(i);
			out.Put32(tf);
			out.Put32(tf_str.GetCount());
			out.Put(tf_str);
		}
		
	}
	else if (key == "quotes") {
		MetaTrader& mt = GetMetaTrader();
		const Vector<Symbol>& symbols = mt.GetSymbols();
		const Vector<Price>& prices = mt.GetAskBid();
		
		out.Put32(symbols.GetCount());
		for(int i = 0; i < symbols.GetCount(); i++) {
			const Symbol& sym = symbols[i];
			const Price& p = prices[i];
			out.Put32(sym.name.GetCount());
			out.Put(sym.name);
			out.Put(&p.ask, sizeof(double));
			out.Put(&p.bid, sizeof(double));
			out.Put(&sym.point, sizeof(double));
		}
	}
	else if (key == "graph") {
		MetaTrader& mt = GetMetaTrader();
		System& sys = GetSystem();
		
		if (args.GetCount() == 3) {
			int sym = ScanInt(args[0]);
			int tf = ScanInt(args[1]);
			int req_count = ScanInt(args[2]);
			if (sym >= 0 && sym < sys.GetSymbolCount() && tf >= 0 && tf < sys.GetPeriodCount()) {
				CoreList c;
				c.AddSymbol(sys.GetSymbol(sym));
				c.AddTf(tf);
				c.AddIndi(0);
				c.AddIndi(System::Find<SpeculationOscillator>());
				c.Init();
				c.Refresh();
				if (!c.IsEmpty()) {
					ConstBuffer& open = c.GetBuffer(0, 0, 0);
					ConstBuffer& low = c.GetBuffer(0, 0, 1);
					ConstBuffer& high = c.GetBuffer(0, 0, 2);
					ConstBuffer& spec = c.GetBuffer(0, 1, 0);
					ConstLabelSignal& specsig = c.GetLabelSignal(0, 1, 0);
					
					int size = open.GetCount();
					int count = min(max(0, req_count), size);
					out.Put32(count);
					for(int i = 0; i < count; i++) {
						int pos = size - count + i;
						double o, l, h, s;
						bool b;
						o = open.Get(pos);
						l = low .Get(pos);
						h = high.Get(pos);
						s = spec.Get(pos);
						b = specsig.signal.Get(pos);
						out.Put(&o, sizeof(double));
						out.Put(&l, sizeof(double));
						out.Put(&h, sizeof(double));
						out.Put(&s, sizeof(double));
						out.Put(&b, sizeof(bool));
					}
				}
				else out.Put32(0);
			}
			else out.Put32(0);
		}
		else out.Put32(0);
	}
	else if (key == "vol") {
		MetaTrader& mt = GetMetaTrader();
		System& sys = GetSystem();
		
		if (args.GetCount() == 3) {
			int sym = ScanInt(args[0]);
			int tf = ScanInt(args[1]);
			int req_count = ScanInt(args[2]);
			if (sym >= 0 && sym < sys.GetSymbolCount() && tf >= 0 && tf < sys.GetPeriodCount()) {
				CoreList c;
				c.AddSymbol(sys.GetSymbol(sym));
				c.AddTf(tf);
				c.AddIndi(System::Find<VolumeSlots>()).AddArg(req_count / 2);
				c.AddIndi(System::Find<VolatilitySlots>()).AddArg(req_count / 2);
				c.Init();
				c.Refresh();
				if (!c.IsEmpty()) {
					
					ConstBuffer& vol = c.GetBuffer(0, 0, 0);
					ConstBuffer& volat = c.GetBuffer(0, 1, 0);
					ConstLabelSignal& vol_bool = c.GetLabelSignal(0, 0, 0);
					ConstLabelSignal& volat_bool = c.GetLabelSignal(0, 1, 0);
					
					int size = vol.GetCount();
					int count = min(max(0, req_count), size);
					out.Put32(count);
					for(int i = 0; i < count; i++) {
						int pos = size - count + i;
						double v = vol.Get(pos);
						double va = volat.Get(pos);
						bool vb = vol_bool.enabled.Get(pos);
						bool vab = volat_bool.enabled.Get(pos);
						out.Put(&v, sizeof(double));
						out.Put(&va, sizeof(double));
						out.Put(&vb, sizeof(bool));
						out.Put(&vab, sizeof(bool));
					}
				}
				else out.Put32(0);
			}
			else out.Put32(0);
		}
		else out.Put32(0);
	}
	else if (key == "status") {
		MetaTrader& mt = GetMetaTrader();
		double balance = mt.AccountBalance();
		double equity = mt.AccountEquity();
		double freemargin = mt.AccountFreeMargin();
		out.Put(&balance, sizeof(double));
		out.Put(&equity, sizeof(double));
		out.Put(&freemargin, sizeof(double));
	}
	else if (key == "openorders") {
		MetaTrader& mt = GetMetaTrader();
		mt.DataEnter();
		
		const Vector<Order>& orders = mt.GetOpenOrders();
		
		out.Put32(orders.GetCount());
		for(int i = 0; i < orders.GetCount(); i++) {
			const Order& o = orders[i];
			const Symbol& sym = mt.GetSymbols()[o.symbol];
			out.Put32(o.ticket);
			out.Put32(o.begin.Get() - Time(1970,1,1).Get());
			out.Put32(o.end.Get() - Time(1970,1,1).Get());
			out.Put32(o.type);
			out.Put(&o.volume, sizeof(double));
			out.Put32(sym.name.GetCount());
			out.Put(sym.name);
			out.Put(&o.open, sizeof(double));
			out.Put(&o.stoploss, sizeof(double));
			out.Put(&o.takeprofit, sizeof(double));
			out.Put(&o.close, sizeof(double));
			out.Put(&o.commission, sizeof(double));
			out.Put(&o.swap, sizeof(double));
			out.Put(&o.profit, sizeof(double));
		}
		
		mt.DataLeave();
	}
	else if (key == "historyorders") {
		MetaTrader& mt = GetMetaTrader();
		mt.DataEnter();
		
		const Vector<Order>& orders = mt.GetHistoryOrders();
		
		out.Put32(orders.GetCount());
		for(int i = 0; i < orders.GetCount(); i++) {
			const Order& o = orders[i];
			const Symbol& sym = mt.GetSymbols()[o.symbol];
			out.Put32(o.ticket);
			out.Put32(o.begin.Get() - Time(1970,1,1).Get());
			out.Put32(o.end.Get() - Time(1970,1,1).Get());
			out.Put32(o.type);
			out.Put(&o.volume, sizeof(double));
			out.Put32(sym.name.GetCount());
			out.Put(sym.name);
			out.Put(&o.open, sizeof(double));
			out.Put(&o.stoploss, sizeof(double));
			out.Put(&o.takeprofit, sizeof(double));
			out.Put(&o.close, sizeof(double));
			out.Put(&o.commission, sizeof(double));
			out.Put(&o.swap, sizeof(double));
			out.Put(&o.profit, sizeof(double));
		}
		
		mt.DataLeave();
	}
	else if (key == "calendar") {
		CalendarCommon& cal = GetCalendar();
		
		Time now = GetUtcTime();
		Time begin = now - 24*60*60;
		Time end = now + 6*60*60;
		Vector<CalEvent> evs;
		for(int i = cal.GetCount()-1; i >= 0; i--) {
			const CalEvent& ev = cal.GetEvent(i);
			if (ev.timestamp > end) continue;
			if (ev.timestamp < begin) break;
			evs.Add(ev);
		}
		
		out.Put32(evs.GetCount());
		for(int i = 0; i < evs.GetCount(); i++) {
			const CalEvent& ev = evs[i];
			out.Put32(ev.id);
			out.Put32(ev.timestamp.Get() - Time(1970,1,1).Get());
			out.Put32(ev.impact);
			out.Put32(ev.direction);
			out.Put32(ev.title.GetCount());		out.Put(ev.title);
			out.Put32(ev.unit.GetCount());		out.Put(ev.unit);
			out.Put32(ev.currency.GetCount());	out.Put(ev.currency);
			out.Put32(ev.forecast.GetCount());	out.Put(ev.forecast);
			out.Put32(ev.previous.GetCount());	out.Put(ev.previous);
			out.Put32(ev.actual.GetCount());	out.Put(ev.actual);
		}
	}
	else if (key == "speculation") {
		Vector<String> sym;
		sym.Add("EUR1");
		sym.Add("USD1");
		sym.Add("GBP1");
		sym.Add("JPY1");
		sym.Add("CHF1");
		sym.Add("CAD1");
		sym.Add("EURUSD" + sys.GetPostFix());
		sym.Add("EURGBP" + sys.GetPostFix());
		sym.Add("EURJPY" + sys.GetPostFix());
		sym.Add("EURCHF" + sys.GetPostFix());
		sym.Add("EURCAD" + sys.GetPostFix());
		sym.Add("GBPUSD" + sys.GetPostFix());
		sym.Add("USDJPY" + sys.GetPostFix());
		sym.Add("USDCHF" + sys.GetPostFix());
		sym.Add("USDCAD" + sys.GetPostFix());
		sym.Add("GBPJPY" + sys.GetPostFix());
		sym.Add("GBPCHF" + sys.GetPostFix());
		sym.Add("GBPCAD" + sys.GetPostFix());
		sym.Add("CHFJPY" + sys.GetPostFix());
		sym.Add("CADJPY" + sys.GetPostFix());
		sym.Add("CADCHF" + sys.GetPostFix());
		Vector<int> tfs;
		tfs.Add(0);
		tfs.Add(2);
		tfs.Add(4);
		tfs.Add(5);
		tfs.Add(6);
		for(int i = 0; i < sym.GetCount(); i++) {
			for(int j = 0; j < tfs.GetCount(); j++) {
				CoreList c;
				c.AddSymbol(sym[i]);
				c.AddTf(tfs[j]);
				c.AddIndi(System::Find<SpeculationOscillator>());
				c.Init();
				c.Refresh();
				ConstLabelSignal& sig = c.GetLabelSignal(0, 0, 0);
				bool b = sig.signal.Top();
				out.Put(&b, sizeof(bool));
				
				double sum = 0;
				for(int k = 0; k < 20; k++) {
					bool b = sig.signal.Get(sig.signal.GetCount() - 1 - k);
					if (b) sum += 1.0;
				}
				sum /= 20;
				b = sum > 0.5;
				out.Put(&b, sizeof(bool));
			}
		}
	}
	else if (key == "activity") {
		Vector<String> sym;
		sym.Add("EUR1");
		sym.Add("USD1");
		sym.Add("GBP1");
		sym.Add("JPY1");
		sym.Add("CHF1");
		sym.Add("CAD1");
		sym.Add("EURUSD" + sys.GetPostFix());
		sym.Add("EURGBP" + sys.GetPostFix());
		sym.Add("EURJPY" + sys.GetPostFix());
		sym.Add("EURCHF" + sys.GetPostFix());
		sym.Add("EURCAD" + sys.GetPostFix());
		sym.Add("GBPUSD" + sys.GetPostFix());
		sym.Add("USDJPY" + sys.GetPostFix());
		sym.Add("USDCHF" + sys.GetPostFix());
		sym.Add("USDCAD" + sys.GetPostFix());
		sym.Add("GBPJPY" + sys.GetPostFix());
		sym.Add("GBPCHF" + sys.GetPostFix());
		sym.Add("GBPCAD" + sys.GetPostFix());
		sym.Add("CHFJPY" + sys.GetPostFix());
		sym.Add("CADJPY" + sys.GetPostFix());
		sym.Add("CADCHF" + sys.GetPostFix());
		Vector<int> tfs;
		tfs.Add(0);
		tfs.Add(2);
		tfs.Add(4);
		tfs.Add(5);
		TimeStop ts;
		for(int i = 0; i < sym.GetCount(); i++) {
			for(int j = 0; j < tfs.GetCount(); j++) {
				CoreList c;
				c.AddSymbol(sym[i]);
				c.AddTf(tfs[j]);
				c.AddIndi(System::Find<VolumeSlots>());
				c.AddIndi(System::Find<VolatilitySlots>());
				c.Init();
				c.Refresh();
				ConstLabelSignal& vol_sig = c.GetLabelSignal(0, 0, 0);
				ConstLabelSignal& volat_sig = c.GetLabelSignal(0, 1, 0);
				bool vol_b = vol_sig.enabled.Top();
				//vol_b = Random(2);
				out.Put(&vol_b, sizeof(bool));
				bool volat_b = volat_sig.enabled.Top();
				//volat_b = Random(2);
				out.Put(&volat_b, sizeof(bool));
			}
		}
	}
	else if (key == "openorder") {
		MetaTrader& mt = GetMetaTrader();
		System& sys = GetSystem();
		
		if (args.GetCount() == 5) {
			int sym = ScanInt(args[0]);
			int sig = ScanInt(args[1]);
			double volume = ScanDouble(args[2]);
			int tp_count = ScanInt(args[3]);
			int sl_count = ScanInt(args[4]);
			
			const Symbol& symbol = mt.GetSymbol(sym);
			String s = symbol.name;
			double price, sl, tp;
			if (!sig) {
				price = mt.RealtimeAsk(sym);
				sl = price - sl_count * symbol.point;
				tp = price + tp_count * symbol.point;
			}
			else {
				price = mt.RealtimeBid(sym);
				sl = price + sl_count * symbol.point;
				tp = price - tp_count * symbol.point;
			}
				
			int r = mt.OrderSend(s, sig, volume, price, 5, sl, tp, "", 0);
			out.Put32(r);
		}
	}
	else if (key == "closeorders") {
		MetaTrader& mt = GetMetaTrader();
		System& sys = GetSystem();
		
		if (args.GetCount() == 1) {
			int sym = ScanInt(args[0]);
			
			const Symbol& symbol = mt.GetSymbol(sym);
			String s = symbol.name;
			const Vector<Order>& orders = mt.GetOpenOrders();
			
			mt.DataEnter();
			for(int i = 0; i < orders.GetCount(); i++) {
				const Order& o = orders[i];
				if (o.symbol == sym)
					mt.CloseOrder(o, o.volume);
			}
			mt.DataLeave();
			
			out.Put32(0);
		}
	}
	else if (key == "equityhistory") {
		
		int count = server->equity_history.GetCount();
		out.Put32(count);
		for(int i = 0; i < count; i++)
			out.Put(&server->equity_history[i], sizeof(double));
		
	}
	
	out.Seek(size_pos);
	out.Put32(out.GetSize() - size_pos - 4);
	out.SeekEnd();
	out.Put32(0);
}


void ActiveSession::Poll(Stream& in, Stream& out) {
	int user_id = LoginId(in);
	
	UserDatabase& db = GetDatabase(user_id);
	
	Vector<InboxMessage> tmp;
	db.lock.Enter();
	Swap(db.inbox, tmp);
	db.lock.Leave();
	
	int count = tmp.GetCount();
	out.Put32(count);
	for(int i = 0; i < count; i++) {
		const InboxMessage& im = tmp[i];
		
		int data_len = im.data.GetCount();
		out.Put32(data_len);
		if (data_len > 0)
			out.Put(im.data.Begin(), data_len);
	}
}

}
