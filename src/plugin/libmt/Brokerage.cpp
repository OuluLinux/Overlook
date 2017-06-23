#include "libmt.h"

namespace libmt {

void Brokerage::ForwardExposure() {
	
	String base = AccountCurrency();
	double leverage = AccountLeverage();
	double base_volume = AccountEquity() * leverage;
	
	//VectorMap<String, double> cur_volumes;
	//VectorMap<int, double> idx_volumes;
	
	int count = GetOpenOrderCount();
	int sym_count = GetSymbolCount();
	
	for(int i = 0; i < count; i++) {
		const Order& bo = GetOpenOrder(i);
		
		if (bo.sym < 0 || bo.sym >= GetSymbolCount())
			continue;
		
		const Symbol& sym = GetSymbol(bo.sym);
		
		if (sym.margin_calc_mode == Symbol::CALCMODE_FOREX) {
			if (sym.base_mul == +1) {
				// For example: base=USD, sym=USDCHF, cmd=buy, lots=0.01, a=USD, b=CHF
				double& cur_volume = cur_volumes[sym.base_cur0];
				if (bo.type == TYPE_BUY)
					cur_volume += -1 * bo.volume * sym.lotsize * GetAsk(bo.sym);
				else if (bo.type == TYPE_SELL)
					cur_volume += bo.volume * sym.lotsize * GetBid(bo.sym);
				else Panic("Type handling not implemented");
			}
			else if (sym.base_mul == -1) {
				// For example: base=USD, sym=AUDUSD, cmd=buy, lots=0.01, a=AUD, b=USD
				double& cur_volume = cur_volumes[sym.base_cur1];
				if (bo.type == TYPE_BUY)
					cur_volume += bo.volume * sym.lotsize;
				else if (bo.type == TYPE_SELL)
					cur_volume += -1 * bo.volume * sym.lotsize;
				else Panic("Type handling not implemented");
			}
			else {
				ASSERT(sym.proxy_id != -1);
				// For example: base=USD, sym=CHFJPY, cmd=buy, lots=0.01, s=USDCHF, a=CHF, b=JPY
				//  - CHF += 0.01 * 1000
				{
					double& cur_volume = cur_volumes[sym.base_cur0];
					if (bo.type == TYPE_BUY)
						cur_volume += bo.volume * sym.lotsize;
					else if (bo.type == TYPE_SELL)
						cur_volume += -1 * bo.volume * sym.lotsize;
					else Panic("Type handling not implemented");
				}
				//  - JPY += -1 * 0.01 * 1000 * open-price
				{
					double& cur_volume = cur_volumes[sym.base_cur1];
					if (bo.type == TYPE_BUY)
						cur_volume += -1 * bo.volume * sym.lotsize * bo.open;
					else if (bo.type == TYPE_SELL)
						cur_volume += bo.volume * sym.lotsize * bo.open;
					else Panic("Type handling not implemented");
				}
			}
		}
		else {
			double& idx_volume = idx_volumes[bo.sym];
			double value;
			if (bo.type == TYPE_BUY)
				value = bo.volume * sym.contract_size;
			else if (bo.type == TYPE_SELL)
				value = -1 * bo.volume * sym.contract_size;
			else Panic("Type handling not implemented");
			idx_volume += value;
		}
		
	}
	
	// Get values in base currency: currencies
	//VectorMap<String, double> cur_rates, cur_base_values;
	for(int i = 0; i < cur_volumes.GetCount(); i++) {
		bool found = false;
		double rate;
		for(int j = 0; j < sym_count; j++) {
			String s = GetSymbol(j).name;
			
			if (s.Left(3) == cur && s.Mid(3,3) == base) {
				rate = GetBid(j);
				cur_rates.Add(cur, rate);
				found = true;
				break;
			}
			else if (s.Mid(3,3) == cur && s.Left(3) == base) {
				rate = 1 / GetAsk(j);
				cur_rates.Add(cur, rate);
				found = true;
				break;
			}
		}
		
		if (!found) Panic("not found");
		
		double volume = cur_volumes[i];
		double base_value = volume / leverage * rate;
		cur_base_values.Add(cur, base_value);
	}
	
	// Get values in base currency: Indices and CDFs
	VectorMap<int, double> idx_rates, idx_base_values;
	for(int i = 0; i < idx_volumes.GetCount(); i++) {
		int id = idx_volumes.GetKey(i);
		Symbol bs = GetSymbol(id);
		String cur = bs.currency_base;
		
		double rate;
		double volume = idx_volumes[i];
		if (cur == base) {
			if (volume < 0)		rate = GetAsk(id);
			else				rate = GetBid(id);
			idx_rates.Add(id, rate);
			double base_value = volume * rate * fabs(volume / bs.contract_size);
			idx_base_values.Add(id, base_value);
		} else {
			bool found = false;
			for(int j = 0; j < sym_count; j++) {
				String s = GetSymbol(j).name;
				
				if (s.Left(3) == cur && s.Mid(3,3) == base) {
					rate = GetBid(j);
					if (volume < 0)		rate *= GetAsk(id);
					else				rate *= GetBid(id);
					idx_rates.Add(id, rate);
					found = true;
					break;
				}
				else if (s.Mid(3,3) == cur && s.Left(3) == base) {
					rate = 1 / GetAsk(j);
					if (volume < 0)		rate *= GetAsk(id);
					else				rate *= GetBid(id);
					idx_rates.Add(id, rate);
					found = true;
					break;
				}
			}
			
			if (!found) Panic("not found");
			
			double base_value = volume * rate * bs.margin_factor;// * abs(volume / bs.contract_size);
			idx_base_values.Add(id, base_value);
		}
		
		
	}
	
	
	// Display currencies
	for(int i = 0; i < cur_volumes.GetCount(); i++) {
		int j;
		const String& cur = cur_volumes.GetKey(i);
		double rate = 0;
		double base_value = 0;
		j = cur_rates.Find(cur);
		if (j != -1) rate = cur_rates[j];
		j = cur_base_values.Find(cur);
		if (j != -1) base_value = cur_base_values[j];
		
		Row& row = rows.Add();
		row.asset = cur_volumes.GetKey(i);
		row.volume = cur_volumes[i];
		row.rate = rate;
		row.base_value = base_value;
		
		if (base_value > 0)
			base_volume -= base_value * leverage * 2; // Freaky bit, but gives correct result. Checked many, many times.
		else
			base_volume -= base_value * leverage;
	}
	
	// Display indices and CDFs
	for(int i = 0; i < idx_volumes.GetCount(); i++) {
		int j;
		int id = idx_volumes.GetKey(i);
		double rate = 0;
		double base_value = 0;
		j = idx_rates.Find(id);
		if (j != -1) rate = idx_rates[j];
		j = idx_base_values.Find(id);
		if (j != -1) base_value = idx_base_values[j];
		Symbol bs = GetSymbol(idx_volumes.GetKey(i));
		
		Row& row = rows.Add();
		row.asset = bs.name;
		row.volume = idx_volumes[i];
		row.rate = rate;
		row.base_value = base_value;
		
		if (bs.currency_base != base) {
			if (base_value > 0)
				base_volume -= base_value * leverage;
			else
				base_volume += base_value * leverage;
		}
	}
	
	// Base currency
	Row& row = rows.Add();
	row.asset = base;
	row.volume = base_volume;
	row.rate = 1.00;
	row.base_value = base_volume / leverage;
	
		
	lock.Leave();
}

void Brokerage::BackwardExposure() {
	ASSERT(basket_begin != -1 && cur_begin != -1);
	System& sys = *this->sys;
	
	// Assert that time is going forward
	Time now = GetTime();
	ASSERT(now > prev_cycle_time);
	prev_cycle_time = now;
	
	
	// Create list of orders what should be opened according to signals
	Vector<int> queue;
	queue.SetCount(0);
	queue.SetCount(sys.GetBrokerSymbolCount(), 0);
	
	int total_sig = 0;
	for(int i = 0; i < signals.GetCount(); i++) {
		int sig = signals[i];
		if (!sig) continue;
		
		// Extend to symbols in correlation-basket
		if (i > basket_begin) {
			const Vector<int>& basket_syms = sys.basket_symbols[i - basket_begin];
			ASSERT(!basket_syms.IsEmpty());
			for(int j = 0; j < basket_syms.GetCount(); j++) {
				int& qsig = queue[basket_syms[j]];
				qsig += sig;
				total_sig += abs(sig);
			}
		}
		// Extend to symbols in currency-basket
		else if (i > cur_begin) {
			const Vector<int>& cur_syms = sys.currency_symbols[i - cur_begin];
			ASSERT(!cur_syms.IsEmpty());
			for(int j = 0; j < cur_syms.GetCount(); j++) {
				int& qsig = queue[cur_syms[j]];
				int cur_sym_id = cur_syms[j];
				const Symbol& sym = symbols[cur_sym_id];
				ASSERT_(sym.base_mul != 0 && (sym.base_mul == -1 || sym.base_mul == +1), "No symbols without base currency allowed currently");
				qsig += sig * sym.base_mul;
				total_sig += abs(sig);
			}
		}
		// Single symbols
		else {
			int& qsig = queue.GetAdd(i, 0);
			qsig += sig;
			total_sig += abs(sig);
		}
	}
	
	// Find minimum relative volume
	double min_vol = DBL_MAX;
	VectorMap<int, double> sym_sig_lots;
	for(int i = 0; i < queue.GetCount(); i++) {
		int& qsig = queue[i];
		if (!qsig) continue;
		
		// Get relative volume
		double vol = (double)abs(qsig) / (double)total_sig;
		if (vol < min_vol) min_vol = vol;
		
		// Assign long/short direction to the volume
		if (qsig < 0) vol *= -1.0;
		sym_sig_lots.Add(queue.GetKey(i), vol);
	}
	
	// Normalize volume to minimum lots
	double total_lots = 0;
	for(int i = 0; i < sym_sig_lots.GetCount(); i++) {
		double& qvol = sym_sig_lots[i];
		
		// Divide by minimum lots and round volume to lots
		bool neg = qvol < 0.0;
		int v = qvol / min_vol + (!neg ? +0.5 : -0.5);
		ASSERT(v != 0);
		qvol = v * 0.01;
		total_lots += qvol;
	}
	
	
	
	/*

	
	// Do some magic management for the minimum volume
	double max_total_lots;
	// TODO: use the "RefreshData": solve from "row.base_value" == equity * free_margin_level
	Panic("TODO");
	if (total_lots > max_total_lots) {
		// Oh-oh, we have a failed signal
		SetFailed();
		return;
	}
	double lot_multiplier = max_total_lots / total_lots;
	for(int i = 0; i < sym_sig_lots.GetCount(); i++) {
		double& qvol = sym_sig_lots[i];
		
		// Multiply and round volume to lots
		bool neg = qvol < 0.0;
		int v = qvol * lot_multiplier + (!neg ? +0.5 : -0.5);
		ASSERT(v != 0);
		qvol = v * 0.01;
	}
	
	
	// Find orders which exceeds the common and intersecting volume area
	//  - all to opposite direction
	//  - exceeding amount to same direction
	typedef Tuple2<int, double> Reduce;
	Vector<Reduce> reduce;
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		
		int j = sym_sig_lots.Find(o.sym);
		
		// If order's symbol is not in the open list, close the order.
		if (j == -1) {
			reduce.Add(Reduce(i, o.volume));
			continue;
		}
		
		double& lots = sym_sig_lots[j];
		bool dst_short = lots < 0.0;
		bool src_short = o.type == 1;
		ASSERT_(o.type < 2, "Only simple long/short is allowed currently");
		
		// If order is to opposite direction, close the order
		if (dst_short != src_short) {
			reduce.Add(Reduce(i, o.volume));
			continue;
		}
		
		// If order exceeds the lots to same direction, reduce the order lots
		Panic("TODO multiple orders with same sym/type");
		Panic("TODO multiple orders with opposing sym/type");
		double abs_lots = fabs(lots);
		int exceeding = (o.volume - abs_lots + 0.005) / 0.01;
		if (exceeding > 0) {
			reduce.Add(Reduce(i, exceeding * 0.01));
			continue;
		}
	}
	
	
	// Commit orders which wasn't in the common area
	Panic("TODO");
	*/
}

}
