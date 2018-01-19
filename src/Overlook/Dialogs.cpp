#include "Overlook.h"

namespace Overlook {

Callback NewOrderWindow::WhenOrdersChanged;

NewOrderWindow::NewOrderWindow() {
	Size dsz = diag.GetSize();
	Size msz = me.GetSize();
	
	SetRect(0,0, dsz.cx, dsz.cy + msz.cy);
	
	Add(diag.LeftPos(0, dsz.cx).TopPos(0, dsz.cy));
	Add(me  .LeftPos(0, msz.cx).TopPos(dsz.cy, msz.cy));
	Add(po  .LeftPos(0, msz.cx).TopPos(dsz.cy, msz.cy));
	Add(mod .LeftPos(0, msz.cx).TopPos(dsz.cy, msz.cy));
	Add(modp.LeftPos(0, msz.cx).TopPos(dsz.cy, msz.cy));
	Add(err .LeftPos(0, msz.cx).TopPos(dsz.cy, msz.cy));
	
	diag.symlist.WhenAction = THISBACK(Data);
	diag.type.WhenAction = THISBACK(SetTypeView);
	
	SetView(VIEW_MARKETEXECUTION);
	Title("Order");
}

void NewOrderWindow::Init(int id) {
	MetaTrader& mt = GetMetaTrader();
	
	for(int i = 0; i < mt.GetSymbolCount(); i++) {
		diag.symlist.Add(mt.GetSymbol(i).name + ", " + mt.GetSymbol(i).description);
	}
	diag.symlist.SetIndex(id);
	
	diag.volume.SetData(0.01);
	
	diag.stoploss.SetData(0);
	diag.takeprofit.SetData(0);
	
	diag.type.Add("Market Execution");
	diag.type.Add("Pending Order");
	
	diag.type.SetIndex(0);
	
	
	me.buy.WhenAction  = THISBACK1(NewOrder, 0);
	me.sell.WhenAction = THISBACK1(NewOrder, 1);
	
	po.limit.Add("Buy Limit");
	po.limit.Add("Sell Limit");
	po.limit.Add("Buy Stop");
	po.limit.Add("Sell Stop");
	po.limit.SetIndex(0);
	
	po.limitprice.SetData(0);
	
	po.place.WhenAction = THISBACK(PlaceOrder);
	
	
	err.ok.WhenAction = THISBACK(SetTypeView);
	
	RefreshBidAsk();
}

void NewOrderWindow::Data() {
	MetaTrader& mt = GetMetaTrader();
	int id = diag.symlist.GetIndex();
	
	const Price& p = mt.GetAskBid()[id];
	bid = p.bid;
	ask = p.ask;
	String pricestr;
	pricestr << bid << " / " << ask;
	me.price.SetLabel(pricestr);
	
	RefreshModifyPointCopyValues();
	Refresh();
}

void NewOrderWindow::RefreshModifyPointCopyValues() {
	if (modify_order.type != -1) {
		double tp_point = mod.tp_points.GetValue();
		double sl_point = mod.sl_points.GetValue();
		if ((modify_order.type % 2) == 0) {
			mod.copy_tp.SetLabel(DblStr(ask + mod_point * tp_point));
			mod.copy_sl.SetLabel(DblStr(bid - mod_point * sl_point));
		} else {
			mod.copy_sl.SetLabel(DblStr(ask + mod_point * sl_point));
			mod.copy_tp.SetLabel(DblStr(bid - mod_point * tp_point));
		}
	}
}

void NewOrderWindow::RefreshBidAsk() {
	Data();
	bidask_timer.Set(500, THISBACK(RefreshBidAsk));
}

void NewOrderWindow::Modify(const libmt::Order& o) {
	MetaTrader& mt = GetMetaTrader();
	
	modify_order = o;
	
	SetView(VIEW_MODIFY);
	
	mod_point = mt.GetSymbol(o.symbol).point;
	
	diag.type.Add("Modify Order");
	diag.type.SetIndex(2);
	
	
	// Opened orders
	mod.tp_points.Clear();
	mod.tp_points.Add(15);
	mod.tp_points.Add(20);
	mod.tp_points.Add(100);
	mod.tp_points.SetIndex(2);
	mod.tp_points.WhenAction = THISBACK(RefreshModifyPointCopyValues);
	
	mod.sl_points.Clear();
	mod.sl_points.Add(15);
	mod.sl_points.Add(20);
	mod.sl_points.Add(100);
	mod.sl_points.SetIndex(2);
	mod.sl_points.WhenAction = THISBACK(RefreshModifyPointCopyValues);
	
	mod.tp.SetData(o.takeprofit);
	mod.sl.SetData(o.stoploss);
	
	mod.copy_sl.WhenAction = THISBACK(CopyStopLoss);
	mod.copy_tp.WhenAction = THISBACK(CopyTakeProfit);
	
	String modstr;
	modstr	<< "Modify " << o.ticket << " " << o.GetTypeString() << " "
			<< " " << o.volume << " " << o.symbol << " sl: " << o.stoploss << " tp: " << o.takeprofit;
	mod.modify.SetLabel(modstr);
	mod.modify.WhenAction = THISBACK(DoModifying);
	
	// Pending orders
	modp.price.SetData(o.open);
	modp.sl.SetData(o.stoploss);
	modp.tp.SetData(o.takeprofit);
	modp.expiry.SetData(o.expiration);
	
	modp.modify.WhenAction = THISBACK(DoModifyingPending);
	modp.del.WhenAction = THISBACK(DoDeleting);
	
	// TODO: check minimum point difference
}

void NewOrderWindow::CopyStopLoss() {
	mod.sl.SetData(StrDbl(mod.copy_sl.GetLabel()));
	// TODO: check minimum point difference
}

void NewOrderWindow::CopyTakeProfit() {
	mod.tp.SetData(StrDbl(mod.copy_tp.GetLabel()));
	// TODO: check minimum point difference
}

void NewOrderWindow::DoModifying() {
	MetaTrader& mt = GetMetaTrader();
	
	double sl = mod.sl.GetData();
	double tp = mod.tp.GetData();
	
	bool r = mt.OrderModify(
		modify_order.ticket,
		modify_order.open,
		sl,
		tp,
		0);
	
	HandleReturnValue(r);
}

void NewOrderWindow::DoModifyingPending() {
	MetaTrader& mt = GetMetaTrader();
	
	double price = modp.price.GetData();
	double sl = modp.sl.GetData();
	double tp = modp.tp.GetData();
	int expiration = 0; //Timestamp(modp.expiry.GetData());
	
	bool r = mt.OrderModify(
		modify_order.ticket,
		price,
		sl,
		tp,
		expiration);
	
	HandleReturnValue(r);
}

void NewOrderWindow::DoDeleting() {
	Panic("TODO");
	//bool r = DeletePendingOrder(modify_order.ticket);
	//HandleReturnValue(r);
}

bool NewOrderWindow::Key(dword key, int count) {
	if (key == K_ESCAPE) {
		PostClose(); return true;
	}
	return false;
}

void NewOrderWindow::PlaceOrder() {
	int type = 2 + po.limit.GetIndex();
	NewOrder(type);
}

void NewOrderWindow::NewOrder(int type) {
	MetaTrader& mt = GetMetaTrader();

	int sym = diag.symlist.GetIndex();
	if (sym == -1) return;
	String symstr = mt.GetSymbol(sym).name;
	
	double volume = diag.volume.GetData();
	double price = 0;
	double stoploss = diag.stoploss.GetData();
	double takeprofit = diag.takeprofit.GetData();
	
	if (type == 0) {
		price = mt.RealtimeAsk(sym);
		if (stoploss == 0) stoploss = price * 0.95;
		if (takeprofit == 0) takeprofit = price * 1.05;
	}
	else if (type == 1) {
		price = mt.RealtimeBid(sym);
		if (stoploss == 0) stoploss = price * 1.05;
		if (takeprofit == 0) takeprofit = price * 0.95;
	}
	else if (type >= 2 && type < 6) {
		price = po.limitprice;
		if (type == 2 || type == 4) {
			if (stoploss == 0) stoploss = price * 0.95;
			if (takeprofit == 0) takeprofit = price * 1.05;
		} else {
			if (stoploss == 0) stoploss = price * 1.05;
			if (takeprofit == 0) takeprofit = price * 0.95;
		}
	}
	else return;
	
	int expiry = 0;
	
	if (type >= 2) {
		Time t = po.expiry.GetData();
		expiry = 0;//Timestamp(t);
	}
	
	int r = -1;
	
	r = mt.OrderSend(symstr, type, volume, price, 2000, stoploss, takeprofit, 0, expiry);
	
	HandleReturnValue(r >= 0);
}

void NewOrderWindow::HandleReturnValue(bool success) {
	MetaTrader& mt = GetMetaTrader();
	
	if (success) {
		mt.Data();
		WhenOrdersChanged();
		Close();
	}
	else {
		SetView(VIEW_ERROR);
		err.msg.SetLabel(mt._GetLastError());
	}
}


void NewOrderWindow::SetTypeView() {
	SetView(diag.type.GetIndex());
}

void NewOrderWindow::SetView(int i) {
	me.Hide();
	po.Hide();
	mod.Hide();
	modp.Hide();
	err.Hide();
	
	diag.symlist.Enable();
	diag.stoploss.Enable();
	diag.takeprofit.Enable();
	diag.volume.Enable();
	diag.comment.Enable();
	
	switch (i) {
		case VIEW_MARKETEXECUTION: me.Show(); break;
		case VIEW_PENDINGORDER:    po.Show(); break;
		case VIEW_MODIFY:
			diag.symlist.Disable();
			diag.stoploss.Disable();
			diag.takeprofit.Disable();
			diag.volume.Disable();
			diag.comment.Disable();

			diag.symlist.SetIndex(modify_order.symbol);
			diag.stoploss.SetData(modify_order.stoploss);
			diag.takeprofit.SetData(modify_order.takeprofit);
			diag.volume.SetData(modify_order.volume);
			diag.comment.SetData("");//modify_order.comment);

			if (modify_order.type < 2) {
				mod.Show(); 
			} else {
				modp.Show(); 
			}
			break;
			
		case VIEW_ERROR:           err.Show(); break;
	}
	Layout();
	Refresh();
}





















Specification::Specification() {
	CtrlLayout(*this);
	close.WhenAction = THISBACK(DoClose);
}

void Specification::Init(int id) {
	sym = id;
	
	list.AddColumn("Key");
	list.AddColumn("Value");
	list.Header(false);
	
	list.Add("Spread", "");
	list.Add("Digits", "");
	list.Add("Stops level", "");
	list.Add("Contract size", "");
	list.Add("Margin currency", "");
	list.Add("Profit calculation mode", "");
	list.Add("Margin calculation mode", "");
	list.Add("Margin hedge", "");
	list.Add("Margin percentage", "");
	list.Add("Trade", "");
	list.Add("Execution", "");
	list.Add("Minimal volume", "");
	list.Add("Maximal volume", "");
	list.Add("Volume step", "");
	list.Add("Swap type", "");
	list.Add("Swap long", "");
	list.Add("Swap short", "");
	list.Add("3-days swap", "");
	
	list.Add("", "");
	
	list.Add("Sessions", "Quotes, Trades");
	list.Add("Sunday", "");
	list.Add("Monday", "");
	list.Add("Tuesday", "");
	list.Add("Wednesday", "");
	list.Add("Thursday", "");
	list.Add("Friday", "");
	list.Add("Saturday", "");
	
	list.Add("", "");
	
	list.Add("Proxy-symbol", "");
	
	Data();
}

void Specification::Data() {
	MetaTrader& mt = GetMetaTrader();
	const Symbol& bs = mt.GetSymbol(sym);

	double margin_perc = -1;
	
	list.Set(0, 1, bs.spread);
	list.Set(1, 1, bs.digits);
	list.Set(2, 1, bs.stops_level);
	list.Set(3, 1, bs.contract_size);
	list.Set(4, 1, bs.currency_margin);
	list.Set(5, 1, bs.GetCalculationMode(bs.profit_calc_mode));
	list.Set(6, 1, bs.GetCalculationMode(bs.margin_calc_mode));
	list.Set(7, 1, bs.margin_hedged);
	list.Set(8, 1, DblStr(bs.margin_factor*100) + "\%");
	list.Set(9, 1, bs.GetTradeMode(bs.trade_mode));
	list.Set(10, 1, bs.GetExecutionMode( bs.execution_mode ));
	
	list.Set(11, 1, bs.volume_min);
	list.Set(12, 1, bs.volume_max);
	list.Set(13, 1, bs.volume_step);
	list.Set(14, 1, bs.GetSwapMode( bs.swap_mode ));
	list.Set(15, 1, bs.swap_long);
	list.Set(16, 1, bs.swap_short);
	list.Set(17, 1, bs.GetDayOfWeek( bs.swap_rollover3days ));
	
	list.Set(20, 1, bs.GetQuotesRange(0) + ", " + bs.GetTradesRange(0));
	list.Set(21, 1, bs.GetQuotesRange(1) + ", " + bs.GetTradesRange(1));
	list.Set(22, 1, bs.GetQuotesRange(2) + ", " + bs.GetTradesRange(2));
	list.Set(23, 1, bs.GetQuotesRange(3) + ", " + bs.GetTradesRange(3));
	list.Set(24, 1, bs.GetQuotesRange(4) + ", " + bs.GetTradesRange(4));
	list.Set(25, 1, bs.GetQuotesRange(5) + ", " + bs.GetTradesRange(5));
	list.Set(26, 1, bs.GetQuotesRange(6) + ", " + bs.GetTradesRange(6));
	
	
	list.Set(28, 1, bs.proxy_name);
	
}



HistoryCenter::HistoryCenter() {
	Title("History Center");
	CtrlLayout(*this);
	
	symbol_list.AddColumn("Id");
	symbol_list.NoHeader();
	
	tf_list.AddColumn("Id");
	tf_list.NoHeader();
	
	data_list.AddColumn("Time");
	data_list.AddColumn("Open");
	data_list.AddColumn("High");
	data_list.AddColumn("Low");
	data_list.AddColumn("Volume");
	data_list.AddColumn("");
	
	data_list.ColumnWidths("4 2 2 2 2 1");
	data_list.HideSb();
	
	data_list.AddFrame(sb);
	
	
	sb.WhenScroll = THISBACK(Data);
	
	close.WhenAction = THISBACK(DoClose);
}

bool HistoryCenter::Key(dword key, int count) {
	if (key == K_ESCAPE) {Close(); return true;}
	return false;
}
	
void HistoryCenter::Init(int id, int tf_id) {
	MetaTrader& mt = GetMetaTrader();
	Data();
	
	int sym_count = mt.GetSymbolCount();
	for(int i = 0; i < sym_count; i++) {
		symbol_list.Add(mt.GetSymbol(i).name);
	}
	symbol_list.SetCursor(id);
	symbol_list.WhenAction = THISBACK(Data);
	
	int tf_count = mt.GetTimeframeCount();
	for(int i = 0; i < tf_count; i++) {
		tf_list.Add(mt.GetTimeframeString(i));
	}
	tf_list.SetCursor(tf_id);
	tf_list.WhenAction = THISBACK(Data);
}

void HistoryCenter::Data() {
	int sym = symbol_list.GetCursor();
	if (sym < 0) return;
	
	int tf_id = tf_list.GetCursor();
	if (tf_id < 0) return;
	
	System& sys = GetSystem();
	Core* core = sys.CreateSingle(0, sym, tf_id);
	if (!core) return;
	
	Output& out = core->GetOutput(0);
	const Buffer& open  = out.buffers[0];
	const Buffer& low   = out.buffers[1];
	const Buffer& high  = out.buffers[2];
	const Buffer& vol   = out.buffers[3];
	
	int count = open.GetCount();
	data_list.Clear();
	
	int screen_count = data_list.GetSize().cy / data_list.GetLineCy(0) - 1;
	int shift = sb.Get();
	for(int i = 0; i < screen_count; i++) {
		int pos = shift + i;
		if (pos < 0 || pos >= count) continue;
		Time time = sys.GetTimeTf(sym, tf_id, pos);
		
		data_list.Set(i, 0, time);
		data_list.Set(i, 1, open.Get(pos));
		data_list.Set(i, 2, high.Get(pos));
		data_list.Set(i, 3, low.Get(pos));
		data_list.Set(i, 4, vol.Get(pos));
	}
	
	sb.SetTotal(count);
	sb.SetPage(screen_count);
}







SystemDataCtrl::SystemDataCtrl() {
	Add(split.SizePos());
	split.Horz();
	
	split << reglist << memlist << datactrl;
	split.SetPos(2500, 0);
	split.SetPos(5000, 1);
	
	datactrl.Add(slider.TopPos(0, 30).HSizePos());
	datactrl.Add(datadraw.VSizePos(30).HSizePos());
	slider.MinMax(0, 1);
	slider.SetData(1);
	slider << THISBACK(Data);
	
	reglist.AddColumn("Reg Key");
	reglist.AddColumn("Value");
	reglist.ColumnWidths("2 1");
	
	memlist.AddColumn("Mem Key");
	memlist.AddColumn("Value");
	memlist.ColumnWidths("2 1");
	
	
}

void SystemDataCtrl::Data() {
	System& sys = GetSystem();
	
	for(int i = 0; i < System::REG_COUNT; i++) {
		reglist.Set(i, 0, sys.GetRegisterKey(i));
		reglist.Set(i, 1, sys.GetRegisterValue(i, sys.main_reg[i]));
	}
	
	for(int i = 0; i < System::MEM_COUNT; i++) {
		memlist.Set(i, 0, sys.GetMemoryKey(i));
		memlist.Set(i, 1, sys.GetMemoryValue(i, sys.main_mem[i]));
	}
	
	int count = sys.GetCountMain(sys.FindPeriod(15));
	int cursor = slider.GetData();
	if (slider.GetMax() == 1) {
		slider.MinMax(0, count-1);
		slider.SetData(count-1);
	} else {
		slider.MinMax(0, count-1);
	}
	
	int cur = slider.GetData();
	if (cur != datadraw.cursor) {
		datadraw.cursor = cur;
		datadraw.Data();
	}
	datadraw.Refresh();
}

void SystemDataCtrl::DataDraw::Data() {
	System& sys = GetSystem();
	int H = sys.main_sym_ids.GetCount() * sys.main_tf_ids.GetCount();
	int W = System::BIT_COUNT;
	Size sz(W, H);
	
	int64 count = sys.main_data.GetCount();
	if (!count) return;
	
	if (cursor < 0 || cursor >= sys.main_mem[System::MEM_INDIBARS])
		return;
	
	ImageBuffer id(sz);
	RGBA* cur = id.Begin();
	for(int i = 0; i < sys.main_tf_ids.GetCount(); i++) {
		for(int j = 0; j < sys.main_sym_ids.GetCount(); j++) {
			for (int x = 0; x < System::BIT_COUNT; x++) {
				int64 pos = sys.GetMainDataPos(cursor, j, i, x);
				if (pos < count) {
					bool b = sys.main_data.Get(pos);
					uint8 gray = b ? 0 : 255;
					cur->r = gray;
					cur->g = gray;
					cur->b = gray;
					cur->a = 255;
				}
				cur++;
			}
		}
	}
	
	img_sz = sz;
	img = id;
	scaled_img_sz = Size(0,0);
}

void SystemDataCtrl::DataDraw::Paint(Draw& w) {
	Size sz = GetSize();
	w.DrawRect(sz, White());
	if (img_sz.cx == 0 && img_sz.cy == 0) return;
	if (scaled_img_sz != sz) {
		scaled_img_sz = sz;
		scaled_img = RescaleFilter(img, scaled_img_sz, FILTER_NEAREST);
	}
	w.DrawImage(0, 0, scaled_img);
}





SystemJobCtrl::SystemJobCtrl() {
	Add(split.SizePos());
	split.Horz();
	
	
	split << joblist << draw;
	split.SetPos(2500);
	
	joblist.AddIndex(job_id);
	joblist.AddColumn("Common #");
	joblist.AddColumn("Level #");
	joblist.AddColumn("Progress");
	joblist.ColumnWidths("1 1 5");
	joblist << THISBACK(Data);
	
}

void SystemJobCtrl::Data() {
	System& sys = GetSystem();
	READLOCK(sys.main_lock) {
		for(int i = 0; i < sys.main_jobs.GetCount(); i++) {
			const System::MainJob& job = sys.main_jobs[i];
			joblist.Set(i, 0, i);
			joblist.Set(i, 1, job.common_pos);
			joblist.Set(i, 2, job.level);
			joblist.Set(i, 3, job.total > 0 ? job.actual * 100 / job.total : 0);
			joblist.SetDisplay(i, 2, Single<JobProgressDislay>());
			joblist.SetDisplay(i, 3, Single<JobProgressDislay>());
		}
		joblist.SetCount(sys.main_jobs.GetCount());
	}
	int cursor = joblist.GetCursor();
	if (cursor >= 0 && cursor < joblist.GetCount()) {
		draw.job_id = joblist.Get(cursor, 0);
		draw.Refresh();
	}
}

void SystemJobCtrl::TrainingCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	System& sys = GetSystem();
	READLOCK(sys.main_lock) {
		if (job_id >= 0 && job_id < sys.main_jobs.GetCount()) {
			const System::MainJob& job = sys.main_jobs[job_id];
			DrawVectorPolyline(id, sz, job.training_pts, polyline);
		}
	}
	
	w.DrawImage(0, 0, id);
}

}
