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
	const Buffer& tbuf  = out.buffers[4];
	
	int count = open.GetCount();
	data_list.Clear();
	
	int screen_count = data_list.GetSize().cy / data_list.GetLineCy(0) - 1;
	int shift = sb.Get();
	for(int i = 0; i < screen_count; i++) {
		int pos = shift + i;
		if (pos < 0 || pos >= count) continue;
		Time time = Time(1970,1,1) + tbuf.Get(pos);
		
		data_list.Set(i, 0, time);
		data_list.Set(i, 1, open.Get(pos));
		data_list.Set(i, 2, high.Get(pos));
		data_list.Set(i, 3, low.Get(pos));
		data_list.Set(i, 4, vol.Get(pos));
	}
	
	sb.SetTotal(count);
	sb.SetPage(screen_count);
}









RuleAnalyzer::RuleAnalyzer() {
	data_check.ra = this;
	datactrl_cursor << THISBACK(SetCursor);
	datactrl_cursor.MinMax(0, 1);
	
	CtrlLayout(*this, "Rule Analyzer :: Alpha version :: Long half-crashed loading at beginning");
	prog.Set(0, 1);
	analyze << THISBACK(Process);
	LoadThis();
	
	begin.AddColumn("Bit period #");
	begin.AddColumn("Bit type");
	begin.AddColumn("Prob. Av.");
	begin.AddColumn("Succ. idx");
	begin.AddColumn("Type");
	begin.ColumnWidths("1 3 2 2 1");
	
	sustain.AddColumn("Bit period #");
	sustain.AddColumn("Bit type");
	sustain.AddColumn("Prob. Av.");
	sustain.AddColumn("Succ. idx");
	sustain.AddColumn("Type");
	sustain.ColumnWidths("1 3 2 2 1");
	
	end.AddColumn("Bit period #");
	end.AddColumn("Bit type");
	end.AddColumn("Prob. Av.");
	end.AddColumn("Succ. idx");
	end.AddColumn("Type");
	end.ColumnWidths("1 3 2 2 1");
	
	Data();
}

RuleAnalyzer::~RuleAnalyzer() {
	StoreThis();
}

void RuleAnalyzer::Prepare() {
	System& sys = GetSystem();
	for(int i = 0; i < sys.GetSymbolCount(); i++) sym_ids.Add(i);
	tf_ids.Add(0);
	
	FactoryDeclaration tmp;
	tmp.arg_count = 0;
	tmp.factory = System::Find<DataBridge>();							indi_ids.Add(tmp);
    /*tmp.factory = System::Find<MovingAverage>();						indi_ids.Add(tmp);
    tmp.factory = System::Find<MovingAverageConvergenceDivergence>();	indi_ids.Add(tmp);
    tmp.factory = System::Find<BollingerBands>();						indi_ids.Add(tmp);
    tmp.factory = System::Find<ParabolicSAR>();							indi_ids.Add(tmp);
    tmp.factory = System::Find<StandardDeviation>();					indi_ids.Add(tmp);
    tmp.factory = System::Find<AverageTrueRange>();						indi_ids.Add(tmp);
    tmp.factory = System::Find<BearsPower>();							indi_ids.Add(tmp);
    tmp.factory = System::Find<BullsPower>();							indi_ids.Add(tmp);
    tmp.factory = System::Find<CommodityChannelIndex>();				indi_ids.Add(tmp);
    tmp.factory = System::Find<DeMarker>();								indi_ids.Add(tmp);
    tmp.factory = System::Find<Momentum>();								indi_ids.Add(tmp);
    tmp.factory = System::Find<RelativeStrengthIndex>();				indi_ids.Add(tmp);
    tmp.factory = System::Find<RelativeVigorIndex>();					indi_ids.Add(tmp);
    tmp.factory = System::Find<StochasticOscillator>();					indi_ids.Add(tmp);
    tmp.factory = System::Find<AcceleratorOscillator>();				indi_ids.Add(tmp);
    tmp.factory = System::Find<AwesomeOscillator>();					indi_ids.Add(tmp);
    tmp.factory = System::Find<PeriodicalChange>();						indi_ids.Add(tmp);
    tmp.factory = System::Find<VolatilityAverage>();					indi_ids.Add(tmp);
    tmp.factory = System::Find<VolatilitySlots>();						indi_ids.Add(tmp);
    tmp.factory = System::Find<VolumeSlots>();							indi_ids.Add(tmp);
    tmp.factory = System::Find<ChannelOscillator>();					indi_ids.Add(tmp);
    tmp.factory = System::Find<ScissorChannelOscillator>();				indi_ids.Add(tmp);*/
    
    sys.System::GetCoreQueue(ci_queue, sym_ids, tf_ids, indi_ids);
}

void RADataCtrl::Paint(Draw& d) {
	Size sz = GetSize();
	ImageDraw id(sz);
	
	id.DrawRect(sz, White());
	
	int mem_pos = cursor * ra->row_size;
	
	if (symbol < ra->data.GetCount() && cursor >= 0 && mem_pos + ra->row_size <= ra->data[symbol].GetCount()) {
		const VectorBool& data = ra->data[symbol];
		
		double xstep = (double)sz.cx / ra->period_count;
		double ystep = (double)sz.cy / ra->COUNT;
		int w = xstep + 0.5;
		int h = ystep + 0.5;
		for(int i = 0; i < ra->period_count; i++) {
			int x = xstep * i;
			for(int j = 0; j < ra->COUNT; j++) {
				int y = ystep * j;
				
				bool bit = data.Get(mem_pos++);
				if (bit)
					id.DrawRect(x, y, w, h, Black());
			}
		}
	}
	
	
	d.DrawImage(0,0,id);
}

void RuleAnalyzer::ProcessData() {
	
	// Get reference values
	System& sys = GetSystem();
	VectorBool& data = this->data[data_cursor];
	DataBridge& db = dynamic_cast<DataBridge&>(*ci_queue[data_cursor]->core);
	ASSERT(db.GetSymbol() == data_cursor);
	double spread_point		= db.GetPoint();
	
	
	// Prepare maind data
	ConstBuffer& open_buf = db.GetBuffer(0);
	ConstBuffer& low_buf  = db.GetBuffer(1);
	ConstBuffer& high_buf = db.GetBuffer(2);
	int data_count = open_buf.GetCount();
	int bit_count = row_size * data_count;
	int begin = data.GetCount() / row_size;
	data.SetCount(bit_count);
	
	
	// Prepare Moving average
	Vector<OnlineAverageWindow1> av_wins;
	av_wins.SetCount(period_count);
	for(int i = 0; i < av_wins.GetCount(); i++)
		av_wins[i].SetPeriod(1 << (1+i));
	
	
	// Prepare VolatilityContext
	Vector<Vector<double> > volat_divs;
	for(int j = 0; j < period_count; j++) {
		int period = 1 << (1+j);
		volat_divs.Add();
		VectorMap<int,int> median_map;
		for(int cursor = period; cursor < data_count; cursor++) {
			double diff = fabs(open_buf.Get(cursor) - open_buf.Get(cursor - period));
			int step = (int)((diff + spread_point * 0.5) / spread_point);
			median_map.GetAdd(step, 0)++;
		}
		SortByKey(median_map, StdLess<int>());
		volat_divs[j].SetCount(0);
		int64 total = 0;
		for(int i = 0; i < median_map.GetCount(); i++)
			total += median_map[i];
		int64 count_div = total / volat_div;
		total = 0;
		int64 next_div = count_div;
		volat_divs[j].Add(median_map.GetKey(0) * spread_point);
		for(int i = 0; i < median_map.GetCount(); i++) {
			total += median_map[i];
			if (total >= next_div) {
				next_div += count_div;
				volat_divs[j].Add(median_map.GetKey(i) * spread_point);
			}
		}
		if (volat_divs[j].GetCount() < volat_div)
			volat_divs[j].Add(median_map.TopKey() * spread_point);
	}
	
	
	// Run main data filler
	for(int cursor = begin; cursor < data_count; cursor++) {
		#ifdef flagDEBUG
		if (cursor >= 100000)
			break;
		#endif
		int bit_pos = cursor * row_size;
		double open1 = open_buf.Get(cursor);
		
		int bit_begin = bit_pos;
		
		for(int k = 0; k < period_count; k++) {
			
			// OnlineMinimalLabel
			double cost	 = spread_point * (1 + k);
			const int count = 1;
			bool sigbuf[count];
			int begin = Upp::max(0, cursor - 200);
			int end = cursor + 1;
			OnlineMinimalLabel::GetMinimalSignal(cost, open_buf, begin, end, sigbuf, count);
			bool label = sigbuf[count - 1];
			data.Set(bit_pos++, label);
		
			
			// TrendIndex
			bool bit_value;
			int period = 1 << (1 + k);
			double err, av_change, buf_value;
			TrendIndex::Process(open_buf, cursor, period, 3, err, buf_value, av_change, bit_value);
			data.Set(bit_pos++, buf_value > 0.0);
			
			
			#ifndef flagDEBUG
			
			// VolatilityContext
			int lvl = -1;
			if (cursor >= period) {
				double diff = fabs(open_buf.Get(cursor) - open_buf.Get(cursor - period));
				for(int i = 0; i < volat_divs[k].GetCount(); i++) {
					if (diff < volat_divs[k][i]) {
						lvl = i - 1;
						break;
					}
				}
			}
			for(int i = 0; i < volat_div; i++)
				data.Set(bit_pos++,  lvl == i);
			
		
			// MovingAverage
			OnlineAverageWindow1& av_win = av_wins[k];
			double prev = av_win.GetMean();
			av_win.Add(open1);
			double curr = av_win.GetMean();
			label = open1 < prev;
			data.Set(bit_pos++, label);
			
			
			// Momentum
			begin = Upp::max(0, cursor - period);
			double open2 = open_buf.Get(begin);
			double value = open1 / open2 - 1.0;
			label = value < 0.0;
			data.Set(bit_pos++, label);
			
			
			// Open/Close trend
			period = 1 << k;
			int dir = 0;
			int len = 0;
			if (cursor >= period * 3) {
				for (int i = cursor-period; i >= 0; i -= period) {
					int idir = open_buf.Get(i+period) > open_buf.Get(i) ? +1 : -1;
					if (dir != 0 && idir != dir) break;
					dir = idir;
					len++;
				}
			}
			data.Set(bit_pos++, len > 2);
		
		
			// High break
			dir = 0;
			len = 0;
			if (cursor >= period * 3) {
				double hi = high_buf.Get(cursor-period);
				for (int i = cursor-1-period; i >= 0; i -= period) {
					int idir = hi > high_buf.Get(i) ? +1 : -1;
					if (dir != 0 && idir != +1) break;
					dir = idir;
					len++;
				}
			}
			data.Set(bit_pos++, len > 2);
			
			
			// Low break
			dir = 0;
			len = 0;
			if (cursor >= period * 3) {
				double lo = low_buf.Get(cursor-period);
				for (int i = cursor-1-period; i >= 0; i -= period) {
					int idir = lo < low_buf.Get(i) ? +1 : -1;
					if (dir != 0 && idir != +1) break;
					dir = idir;
					len++;
				}
			}
			data.Set(bit_pos++, len > 2);
			
			
			// Trend reversal
			int t0 = +1;
			int t1 = +1;
			int t2 = -1;
			if (cursor >= 4*period) {
				double t0_diff		= open_buf.Get(cursor-0*period) - open_buf.Get(cursor-1*period);
				double t1_diff		= open_buf.Get(cursor-1*period) - open_buf.Get(cursor-2*period);
				double t2_diff		= open_buf.Get(cursor-2*period) - open_buf.Get(cursor-3*period);
				t0 = t0_diff > 0 ? +1 : -1;
				t1 = t1_diff > 0 ? +1 : -1;
				t2 = t2_diff > 0 ? +1 : -1;
			}
			if (t0 * t1 == -1 && t1 * t2 == +1) {
				data.Set(bit_pos++, t0 == +1);
				data.Set(bit_pos++, t0 != +1);
			} else {
				data.Set(bit_pos++, false);
				data.Set(bit_pos++, false);
			}
			
			#endif
		}
		
		ASSERT(bit_pos - bit_begin == row_size);
	}
}

void RuleAnalyzer::Data() {
	if (this->data.IsEmpty()) return;
	RealizeStats();
	
	int begin_id = 0;
	
	if (begin_id >= 0 && begin_id < row_size) {
		auto& stats = this->stats[Upp::min(JOINLEVEL_COUNT-1, joinlevel)][begin_id];
		if (stats.begins.IsEmpty()) return;
		
		for(int i = 0; i < row_size; i++) {
			BitStats& stat = stats.begins[i];
			int period_id = i / COUNT;
			int type_id = i % COUNT;
			begin.Set(i, 0, period_id);
			begin.Set(i, 1, GetTypeString(type_id));
			begin.Set(i, 2, stat.prob_av);
			begin.Set(i, 3, stat.succ_idx);
			begin.Set(i, 4, stat.type ? "Short" : "Long");
		}
		begin.SetSortColumn(3, true);
		
		for(int i = 0; i < row_size; i++) {
			BitStats& stat = stats.sustains[i];
			int period_id = i / COUNT;
			int type_id = i % COUNT;
			sustain.Set(i, 0, period_id);
			sustain.Set(i, 1, GetTypeString(type_id));
			sustain.Set(i, 2, stat.prob_av);
			sustain.Set(i, 3, stat.succ_idx);
			sustain.Set(i, 4, stat.type ? "Short" : "Long");
		}
		sustain.SetSortColumn(3, true);
		
		for(int i = 0; i < row_size; i++) {
			BitStats& stat = stats.ends[i];
			int period_id = i / COUNT;
			int type_id = i % COUNT;
			end.Set(i, 0, period_id);
			end.Set(i, 1, GetTypeString(type_id));
			end.Set(i, 2, stat.prob_av);
			end.Set(i, 3, stat.succ_idx);
			end.Set(i, 4, stat.type ? "Short" : "Long");
		}
		end.SetSortColumn(3, true);
	}
	
	
	auto& data = this->data[0];
	int data_count = data.GetCount() / row_size;
	if (datactrl_cursor.GetMax() != data_count-1) {
		datactrl_cursor.MinMax(0, data_count-1);
		datactrl_cursor.SetData(data_count-1);
	}
}

void RuleAnalyzer::Process() {
	System& sys = GetSystem();
	TimeStop ts;
	int phase_total = row_size * row_size * LOOP_COUNT;
	
	if (!is_prepared) {
		Prepare();
		is_prepared = true;
	}
	
	data.SetCount(sys.GetSymbolCount());
	
	int current_in_loop = 0;
	while (ts.Elapsed() < 100 && phase < FINISHED && joinlevel < JOINLEVEL_COUNT) {
		
		if (processed_cursor < ci_queue.GetCount()) {
			sys.System::Process(*ci_queue[processed_cursor++], true);
			continue;
		}
		else if (data_cursor < sys.GetSymbolCount()) {
			ProcessData();
			StoreThis();
			data_cursor++;
			continue;
		}
		else if (phase >= BEGIN && phase <= END) {
			Iterate(phase - BEGIN);
		}
		
		
		current++;
		current_in_loop++;
		if (current >= phase_total) {
			current = 0;
			phase++;
			
			if (phase >= FINISHED) {
				phase = 0;
				joinlevel++;
			}
		}
	}
	
	int actual = phase_total * (phase + FINISHED * joinlevel) + current;
	int total = phase_total * FINISHED * JOINLEVEL_COUNT;
	prog.Set(actual, total);
	
	double speed = ((double)current_in_loop / (double)ts.Elapsed()) * 1000.0;
	if (speed < 1.0) speed = 1.0;
	int seconds = total / speed;
	int perc = actual * 100 / total;
	if (perc < 100) {
		int hours = seconds / 3600;		seconds = seconds % 3600;
		int minutes = seconds / 60;		seconds = seconds % 60;
		Title(Format("Rule Analyzer :: %d%% :: Time remaining %d hours %d minutes %d seconds", perc, hours, minutes, seconds));
	}
	else Title("Rule Analyzer :: Ready");
	
	Data();
	
	bool ready = phase == FINISHED && joinlevel == JOINLEVEL_COUNT;
	if (!ready) PostCallback(THISBACK(Process));
}

void RuleAnalyzer::RealizeStats() {
	if (!stats.IsEmpty()) return;
	stats.SetCount(JOINLEVEL_COUNT);
	for(int i = 0; i < stats.GetCount(); i++) {
		stats[i].SetCount(row_size);
		for(int j = 0; j < row_size; j++) {
			BeginStats& bs = stats[i][j];
			bs.begins.SetCount(row_size);
			bs.sustains.SetCount(row_size);
			bs.ends.SetCount(row_size);
			if (i == 0) bs.begin_bits.Add(j);
		}
	}
}

void RuleAnalyzer::Iterate(int type) {
	System& sys = GetSystem();
	RealizeStats();
	
	if (!type && !joinlevel) return; // no new begins for first joinlevel
	
	int bit_id = current / (row_size * row_size);
	int bit_opt = current % (row_size * row_size);
	int begin_id = bit_opt / row_size;
	int sub_id = bit_opt % row_size;
	
	// Getvalues for bits
	auto& stats = this->stats[joinlevel][begin_id];
	auto& sub_stats = type == 0 ? stats.begins : (type == 1 ? stats.sustains : stats.ends);
	BitStats& stat = sub_stats[sub_id];
	stat.prob_av = 0.0;
	for(int i = 0; i < sys.GetSymbolCount(); i++) {
		stat.prob_av += GetBitProbTest(i, begin_id, type, sub_id, stat.type);
	}
	stat.prob_av /= sys.GetSymbolCount();
	stat.succ_idx = fabs(stat.prob_av * 200.0 - 100.0);
	stat.type = stat.prob_av < 0.5;
	
	// When bit is added
	if (bit_opt+1 == row_size * row_size && current > 0) {
		for(int begin_id = 0; begin_id < row_size; begin_id++) {
			auto& stats = this->stats[joinlevel][begin_id];
			auto& sub_stats = type == 0 ? stats.begins : (type == 1 ? stats.sustains : stats.ends);
			auto& bits = (type == 0 ? stats.begin_bits : (type == 1 ? stats.sust_bits : stats.end_bits));
			double max_succ_idx = -DBL_MAX;
			int max_pos = -1;
			for(int i = 0; i < sub_stats.GetCount(); i++) {
				double d = sub_stats[i].succ_idx;
				if (d > max_succ_idx) {
					if (bits.Find(i)) continue;
					max_succ_idx = d;
					max_pos = i;
				}
			}
			// Append best bit
			if (max_pos != -1) {
				ASSERT(max_pos >= 0 && max_pos < row_size);
				bits.Add(max_pos);
			}
		}
	}
}

double RuleAnalyzer::GetBitProbBegin(int symbol, int begin_id) {
	VectorBool& data = this->data[symbol];
	ConstBuffer& open_buf = ci_queue[symbol]->core->GetBuffer(0);
	
	int bars = data.GetCount() / row_size - BEGIN_PEEK;
	
	int true_count = 0;
	int pos = begin_id;
	for(int i = 0; i < bars; i++) {
		bool pred_value = data.Get(pos);
		bool actual_value = open_buf.Get(i + BEGIN_PEEK) < open_buf.Get(i);
		pos += row_size;
		if (pred_value == actual_value) true_count++;
	}
	
	return (double)true_count / (double)bars;
}

double RuleAnalyzer::GetBitProbTest(int symbol, int begin_id, int type, int sub_id, bool inv_action) {
	VectorBool& data = this->data[symbol];
	ConstBuffer& open_buf = ci_queue[symbol]->core->GetBuffer(0);
	BeginStats& begin = this->stats[joinlevel][begin_id];
	
	int peek;
	bool test_begin = false, test_sust = false, test_end = false;
	switch (type) {
		case 0:
			test_begin = true;
			peek = BEGIN_PEEK;
			break;
		case 1:
			test_sust = true;
			peek = SUSTAIN_PEEK;
			break;
		case 2:
			test_end = true;
			peek = END_PEEK;
			break;
		default: Panic("Invalid caller");
	}
	
	int bars = data.GetCount() / row_size - peek;
	
	int open_left = 0;
	bool open_dir = false;
	
	int true_count = 0;
	int begin_pos = 0;
	int total = 0;
	for(int i = 0; i < bars; i++) {
		
		// Existing values
		bool begin_value = true;
		for(int j = 0; j < begin.begin_bits.GetCount(); j++)
			begin_value &= data.Get(begin_pos + begin.begin_bits[j]); // AND
		if (begin_value && open_left < peek)
			open_left = peek;
		
		bool sust_value = false;
		for(int j = 0; j < begin.sust_bits.GetCount(); j++)
			sust_value |= data.Get(begin_pos + begin.sust_bits[j]); // OR
		if (sust_value && open_left == 1)
			open_left++;
		
		bool end_value = false;
		for(int j = 0; j < begin.end_bits.GetCount(); j++)
			end_value |= data.Get(begin_pos + begin.end_bits[j]); // OR
		if (end_value)
			open_left = 0;
		
		// Test values
		if (test_begin && begin_value) {
			bool pred_value = data.Get(begin_pos + sub_id);
			bool actual_value = open_buf.Get(i + peek) < open_buf.Get(i);
			if (inv_action) actual_value = !actual_value;
			if (pred_value == actual_value) true_count++;
			total++;
		}
		if (test_sust && open_left > 0) {
			bool pred_value = data.Get(begin_pos + sub_id);
			bool actual_value = open_buf.Get(i + peek) < open_buf.Get(i);
			if (inv_action) actual_value = !actual_value;
			if (pred_value == actual_value) true_count++;
			total++;
		}
		if (test_end && open_left > 0) {
			bool pred_value = data.Get(begin_pos + sub_id);
			bool actual_value = open_buf.Get(i + peek) < open_buf.Get(i);
			if (inv_action) actual_value = !actual_value;
			if (pred_value != actual_value) true_count++;
			total++;
		}
		
		if (open_left > 0)
			open_left--;
		begin_pos += row_size;
	}
	
	return total > 0 ? (double)true_count / (double)total : 0.50;
}


void RuleAnalyzer::ProcessRealtime() {
	
	// Get reference values
	System& sys = GetSystem();
	VectorBool& data = this->data[data_cursor];
	DataBridge& db = dynamic_cast<DataBridge&>(*ci_queue[data_cursor]->core);
	ASSERT(db.GetSymbol() == data_cursor);
	double spread_point		= db.GetPoint();
	
	
	// Prepare maind data
	ConstBuffer& open_buf = db.GetBuffer(0);
	ConstBuffer& low_buf  = db.GetBuffer(1);
	ConstBuffer& high_buf = db.GetBuffer(2);
	int data_count = open_buf.GetCount();
	int bit_count = rt_row_size * data_count;
	int begin = data.GetCount() / rt_row_size;
	data.SetCount(bit_count);
	
	/*
	// Run main data filler
	for(int cursor = begin; cursor < data_count; cursor++) {
		if (cursor >= 100000)
			break;
		int bit_pos = cursor * rt_row_size;
		double open1 = open_buf.Get(cursor);
		
		// Existing values
		bool begin_value = true;
		for(int j = 0; j < begin.begin_bits.GetCount(); j++)
			begin_value &= data.Get(begin_pos + begin.begin_bits[j]); // AND
		if (begin_value && open_left < peek)
			open_left = peek;
		
		bool sust_value = false;
		for(int j = 0; j < begin.sust_bits.GetCount(); j++)
			sust_value |= data.Get(begin_pos + begin.sust_bits[j]); // OR
		if (sust_value && open_left == 1)
			open_left++;
		
		bool end_value = false;
		for(int j = 0; j < begin.end_bits.GetCount(); j++)
			end_value |= data.Get(begin_pos + begin.end_bits[j]); // OR
		if (end_value)
			open_left = 0;
		
	}*/
}

void RuleAnalyzer::Refresh() {
	if (joinlevel < JOINLEVEL_COUNT)
		return;
	ProcessData();
	ProcessRealtime();
}

}
