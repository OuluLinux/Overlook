#include "AsmProto.h"


Generator::Generator() {
	
}

void Generator::AddRandomPressure() {
	PricePressure& p = active_pressures.Add();
	Randomize(p, price, iter);
}

void Generator::Randomize(PricePressure& p, double price, int iter) {
	double price_point = price + (-10 + Random(21)) * step;
	p.low  = price_point - (1 + Random(10)) * step;
	p.high = price_point + (1 + Random(10)) * step;
	p.min = 1 + Random(10);
	p.max = p.min + 1 + Random(10);
	p.action = Random(2);
	p.size = 1 + Random(1000);
	p.iter = iter;
}

void Generator::GenerateData(Vector<double>& data, bool add_random, int count) {
	
	price = 1.0;
	active_pressures.Clear();
	
	a.iter = 0;
	a.Sort();
	
	iter = 0;
	if (add_random) {
		for(int i = 0; i < 100; i++)
			AddRandomPressure();
	}
	
	
	if (count <= 0)
		count = data_count;
	
	data.SetCount(data_count, price);
	
	for(iter = 0; iter < count; iter++) {
		
		while (a.iter < a.src.GetCount()) {
			PricePressure& p = a.src[a.iter];
			ASSERT(p.iter != -1);
			if (p.iter > iter)
				break;
			active_pressures.Add(p);
			a.iter++;
		}
		
		double pres;
		Index<double> visited_prices;
		
		while (true) {
			
			double inc_price = price + step;
			double inc_buy_pres, inc_sell_pres;
			GetPricePressure(inc_price, inc_buy_pres, inc_sell_pres);
			double inc_min = min(inc_buy_pres, inc_sell_pres);
			
			
			double dec_price = price - step;
			double dec_buy_pres, dec_sell_pres;
			GetPricePressure(dec_price, dec_buy_pres, dec_sell_pres);
			double dec_min = min(dec_buy_pres, dec_sell_pres);
			
			
			if (inc_min > dec_min) {
				price = inc_price;
				pres = min(inc_buy_pres, inc_sell_pres);
			}
			else if (inc_min < dec_min) {
				price = dec_price;
				pres = min(dec_buy_pres, dec_sell_pres);
			}
			else {
				//price = inc_price;// 
				if (add_random)
					price = Random(2) ? inc_price : dec_price;
				else
					price = inc_price;
				break;
			}
			
			
			if (visited_prices.Find(price) != -1) break;
			visited_prices.Add(price);
		}
		
		
		SimpleReducePressure(100);
		if (add_random)
			while (active_pressures.GetCount() < 100)
				AddRandomPressure();
		
		
		data[iter] = price;
	}
	
}

void Generator::GetPricePressure(double price, double& buy_pres, double& sell_pres) {
	buy_pres = 0;
	sell_pres = 0;
	for (PricePressure& p : active_pressures) {
		if (p.low <= price && price <= p.high) {
			double range = p.high - p.low;
			double diff = price - p.low;
			double factor = diff / range;
			if (p.action) factor = 1.0 - factor;
			double prange = p.max - p.min;
			double pres = p.max - factor * prange; // buy price at low -> factor 0 -> max
			if (!p.action)	buy_pres += pres;
			else			sell_pres += pres;
		}
	}
}

void Generator::SimpleReducePressure(double amount) {
	int sell_count = 0, buy_count = 0;
	for (PricePressure& p : active_pressures) {
		if (p.low <= price && price <= p.high) {
			if (!p.action)	buy_count++;
			else			sell_count++;
		}
	}
	
	double sell_amount = amount, buy_amount = amount;
	double av_buy = buy_amount / buy_count;
	double av_sell = sell_amount / sell_count;
	for (int i = 0; i < active_pressures.GetCount(); i++) {
		PricePressure& p = active_pressures[i];
		if (p.low <= price && price <= p.high) {
			if (p.action == 0) {
				p.size -= av_buy;
				if (p.size < 0) {
					active_pressures.Remove(i);
					i--;
				}
			} else {
				p.size -= av_sell;
				if (p.size < 0) {
					active_pressures.Remove(i);
					i--;
				}
			}
		}
	}
}
