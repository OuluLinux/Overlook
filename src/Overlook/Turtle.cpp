#include "Overlook.h"

// turbo profit

namespace Overlook {

Turtle::Turtle() {

}

void Turtle::InitEA() {
	gi_644 = 1;
	
	if (Digits == 5 || Digits == 3)
		gi_644 = 10;
		
	for (gi_552 = 1; gi_552 < 8; gi_552++) {
		gia_568[gi_552] = 0;
		gia_572[gi_552] = 0;
		gia_584[gi_552] = 0;
		gia_588[gi_552] = 0;
		gia_592[gi_552] = 0;
		gia_596[gi_552] = 0;
		gia_600[gi_552] = 0;
		gia_604[gi_552] = 0;
	}
	}

void Turtle::StartEA(int pos) {
	int l_stoplevel_8;
	int l_spread_12;
	double l_point_16;
	double l_bid_24;
	double l_ask_32;
	int li_unused_40;
	double l_minlot_44;
	double l_lotstep_52;
	double ld_60;
	int li_68;
	int li_72;
	int li_76;
	int li_80;
	int li_84;
	int li_88;
	bool li_92;
	bool li_96;
	bool li_100;
	bool li_104;
	double ld_108;
	double ld_116;
	String ls_0 = Symbol();
	
	if (gd_296 < 100.0 && AccountFreeMargin() < AccountBalance() * gd_296 / 100.0) {
		return;
	}
	
	gia_408[1] = 0;
	
	gia_408[2] = 0;
	g_index_400 = 0;
	gsa_412[g_index_400] = ls_0;
	gia_432[g_index_400] = Magic + g_index_400;
	N();
	g_index_400 = 1;
	gsa_412[g_index_400] = ls_0;
	gia_432[g_index_400] = Magic + g_index_400;
	N();
	g_index_400 = 2;
	gsa_412[g_index_400] = ls_0;
	gia_432[g_index_400] = Magic + g_index_400;
	N();
	
	if (gia_508[0] > N_enable_Sloy || gia_512[0] > N_enable_Sloy && TorgSloy > 1 && TorgSloy < 4)
		gia_408[1] = 1;
		
	if (gia_508[1] > 0 || gia_512[1] > 0)
		gia_408[1] = 1;
		
	if (gia_508[0] > N_enable_Sloy || gia_512[0] > N_enable_Sloy && gia_508[1] > N_enable_Sloy || gia_512[1] > N_enable_Sloy && TorgSloy > 2 && TorgSloy < 4)
		gia_408[2] = 1;
		
	if (gia_508[2] > 0 || gia_512[2] > 0)
		gia_408[2] = 1;
		
	for (g_index_400 = 0; g_index_400 < gi_404; g_index_400++) {
		Sleep(100);
		
		if (gia_408[g_index_400] == 1) {
			gia_432[g_index_400] = Magic + g_index_400;
			l_stoplevel_8 = MarketInfo(gsa_412[g_index_400], MODE_STOPLEVEL);
			l_spread_12 = MarketInfo(gsa_412[g_index_400], MODE_SPREAD);
			l_point_16 = MarketInfo(gsa_412[g_index_400], MODE_POINT);
			l_bid_24 = MarketInfo(gsa_412[g_index_400], MODE_BID);
			l_ask_32 = MarketInfo(gsa_412[g_index_400], MODE_ASK);
			li_unused_40 = MarketInfo(gsa_412[g_index_400], MODE_DIGITS);
			
			if (g_maxlot_148 == 0.0)
				g_maxlot_148 = MarketInfo(gsa_412[g_index_400], MODE_MAXLOT);
				
			l_minlot_44 = MarketInfo(gsa_412[g_index_400], MODE_MINLOT);
			
			l_lotstep_52 = MarketInfo(gsa_412[g_index_400], MODE_LOTSTEP);
			
			ld_60 = AccountBalance();
			
			if (l_minlot_44 == 0.01)
				li_68 = 2;
				
			if (l_minlot_44 == 0.1)
				li_68 = 1;
				
			if (l_minlot_44 >= 1.0)
				li_68 = 0;
				
			if (l_lotstep_52 == 0.01)
				li_72 = 2;
				
			if (l_lotstep_52 == 0.1)
				li_72 = 1;
				
			if (l_lotstep_52 >= 1.0)
				li_72 = 0;
				
			if (((!IsOptimization()) && !IsTesting() && (!IsVisualMode())) || (ShowTableOnTesting && IsTesting() && (!IsOptimization()))) {

			}
			
			if (gi_232 * gi_644 < l_stoplevel_8)
				gia_664[g_index_400] = l_stoplevel_8 + 5 * gi_644;
			else
				gia_664[g_index_400] = gi_232 * gi_644;
				
			if (gi_232 * gi_644 < l_stoplevel_8)
				gia_668[g_index_400] = l_stoplevel_8 + 5 * gi_644;
			else
				gia_668[g_index_400] = gi_232 * gi_644;
				
			gda_648[g_index_400] = gi_212 * gi_644;
			
			gda_652[g_index_400] = gi_212 * gi_644;
			
			gda_656[g_index_400] = gi_216 * gi_644;
			
			gda_660[g_index_400] = gi_216 * gi_644;
			
			gda_676[g_index_400] = hSETKY * gi_644;
			
			gda_684[g_index_400] = hSETKY * gi_644;
			
			gda_696[g_index_400] = ProtectionTP * gi_644;
			
			gda_700[g_index_400] = ProtectionTP * gi_644;
			
			gda_768[g_index_400] = gi_304 * gi_644;
			
			gda_772[g_index_400] = gi_304 * gi_644;
			
			g_slippage_316 *= gi_644;
			
			if (gi_96) {
				gda_688[g_index_400] = gd_100;
				gda_692[g_index_400] = gd_100;
			}
			
			else {
				gda_688[g_index_400] = NormalizeDouble(ld_60 * RiskPercent / 1000000.0, li_68);
				gda_692[g_index_400] = NormalizeDouble(ld_60 * RiskPercent / 1000000.0, li_68);
			}
			
			if (gda_688[g_index_400] == 0.0)
				gda_688[g_index_400] = l_minlot_44;
				
			if (gda_692[g_index_400] == 0.0)
				gda_692[g_index_400] = l_minlot_44;
				
			N();
			
			if (gia_508[g_index_400] > 0) {
				li_76 = MAX(0, gia_508[g_index_400]);
				li_80 = MIN(0, gia_508[g_index_400]);
			}
			
			if (gia_512[g_index_400] > 0) {
				li_84 = MAX(1, gia_512[g_index_400]);
				li_88 = MIN(1, gia_512[g_index_400]);
			}
			
			if (gia_508[g_index_400] > 1) {
				li_92 = NOV(0, gia_508[g_index_400], l_ask_32);
				li_96 = NON(0, gia_508[g_index_400], l_ask_32);
			}
			
			else {
				li_92 = false;
				li_96 = false;
			}
			
			if (gia_512[g_index_400] > 1) {
				li_100 = NOV(1, gia_512[g_index_400], l_bid_24);
				li_104 = NON(1, gia_512[g_index_400], l_bid_24);
			}
			
			else {
				li_100 = false;
				li_104 = false;
			}
			
			gda_672[g_index_400] = gda_676[g_index_400] * l_point_16;
			
			gda_680[g_index_400] = gda_684[g_index_400] * l_point_16;
			
			if (Uvel_hSETKY == 0) {
				gda_672[g_index_400] = gda_676[g_index_400] * l_point_16;
				gda_680[g_index_400] = gda_684[g_index_400] * l_point_16;
			}
			
			if (Uvel_hSETKY == 1) {
				if (gia_508[g_index_400] < 2)
					gda_672[g_index_400] = gda_676[g_index_400] * l_point_16;
					
				if (gia_512[g_index_400] < 2)
					gda_680[g_index_400] = gda_684[g_index_400] * l_point_16;
					
				if (gia_508[g_index_400] > 1)
					gda_672[g_index_400] = (gda_676[g_index_400] + ShagUvel_hSETKY * gia_508[g_index_400] * gi_644) * l_point_16;
					
				if (gia_512[g_index_400] > 1)
					gda_680[g_index_400] = (gda_684[g_index_400] + ShagUvel_hSETKY * gia_512[g_index_400] * gi_644) * l_point_16;
			}
			
			if (Uvel_hSETKY == 2) {
				if (gia_508[g_index_400] < 2)
					gda_672[g_index_400] = gda_676[g_index_400] * l_point_16;
					
				if (gia_512[g_index_400] < 2)
					gda_680[g_index_400] = gda_684[g_index_400] * l_point_16;
					
				if (gia_508[g_index_400] > 1)
					gda_672[g_index_400] = (gda_676[g_index_400] - ShagUvel_hSETKY * gia_508[g_index_400] * gi_644) * l_point_16;
					
				if (gia_512[g_index_400] > 1)
					gda_680[g_index_400] = (gda_684[g_index_400] - ShagUvel_hSETKY * gia_512[g_index_400] * gi_644) * l_point_16;
			}
			
			if (gda_672[g_index_400] < 10 * gi_644 * l_point_16)
				gda_672[g_index_400] = 10 * gi_644 * l_point_16;
				
			if (gda_680[g_index_400] < 10 * gi_644 * l_point_16)
				gda_680[g_index_400] = 10 * gi_644 * l_point_16;
				
			gia_608[g_index_400] = 0;
			
			gia_616[g_index_400] = 0;
			
			gia_612[g_index_400] = 0;
			
			gia_620[g_index_400] = 0;
			
			if (gia_508[g_index_400] == 0)
				gia_608[g_index_400] = 1;
				
			if (gia_512[g_index_400] == 0)
				gia_612[g_index_400] = 1;
				
			if (gia_508[g_index_400] > 0 && Ask < gda_472[g_index_400][0][li_80] - gda_672[g_index_400])
				gia_608[g_index_400] = 1;
				
			if (gia_508[g_index_400] > 0 && Ask > gda_472[g_index_400][0][li_76] + gda_672[g_index_400])
				gia_608[g_index_400] = 1;
				
			if (gia_512[g_index_400] > 0 && Bid > gda_472[g_index_400][1][li_84] + gda_680[g_index_400])
				gia_612[g_index_400] = 1;
				
			if (gia_512[g_index_400] > 0 && Bid < gda_472[g_index_400][1][li_88] - gda_680[g_index_400])
				gia_612[g_index_400] = 1;
				
			if (gia_508[g_index_400] > 0 && gia_508[g_index_400] > gia_512[g_index_400] && Ask < gda_472[g_index_400][0][li_80] - gda_672[g_index_400])
				gia_608[g_index_400] = 1;
				
			if (gia_508[g_index_400] > 0 && gia_508[g_index_400] > gia_512[g_index_400] && Ask > gda_472[g_index_400][0][li_76] + gda_672[g_index_400])
				gia_608[g_index_400] = 1;
				
			if (gia_512[g_index_400] > 0 && gia_512[g_index_400] > gia_508[g_index_400] && Bid > gda_472[g_index_400][1][li_84] + gda_680[g_index_400])
				gia_612[g_index_400] = 1;
				
			if (gia_512[g_index_400] > 0 && gia_512[g_index_400] > gia_508[g_index_400] && Bid < gda_472[g_index_400][1][li_88] - gda_680[g_index_400])
				gia_612[g_index_400] = 1;
				
			gia_632[g_index_400] = 0;
			
			gia_636[g_index_400] = 0;
			
			if (gia_608[g_index_400] > 0 || gia_612[g_index_400] > 0) {
				if (gia_508[g_index_400] == 0 && gia_512[g_index_400] == 0) {
					gia_632[g_index_400] = 1;
					gia_636[g_index_400] = 1;
				}
				
				if (gia_508[g_index_400] == 1 && gia_512[g_index_400] == 1 && gia_608[g_index_400] == 1)
					gia_632[g_index_400] = 1;
					
				if (gia_508[g_index_400] == 1 && gia_512[g_index_400] == 1 && gia_612[g_index_400] == 1)
					gia_636[g_index_400] = 1;
					
				if (gia_508[g_index_400] > 1 && gia_512[g_index_400] > 0 && l_bid_24 < gda_472[g_index_400][0][li_80] - gda_672[g_index_400])
					gia_632[g_index_400] = 1;
					
				if (gia_508[g_index_400] > 0 && gia_512[g_index_400] > 1 && l_bid_24 > gda_472[g_index_400][1][li_84] + gda_680[g_index_400])
					gia_636[g_index_400] = 1;
					
				if (gia_508[g_index_400] > 0 && gia_512[g_index_400] == 0 && l_bid_24 < gda_472[g_index_400][0][li_80] - gda_680[g_index_400])
					gia_632[g_index_400] = 1;
					
				if (gia_508[g_index_400] == 0 && gia_512[g_index_400] > 0 && l_bid_24 > gda_472[g_index_400][1][li_84] + gda_672[g_index_400])
					gia_636[g_index_400] = 1;
			}
			

			gi_unused_704 = 0;
			
			gi_unused_708 = 0;
			gi_712 = false;
			gi_716 = false;
			
			if (gia_608[g_index_400] == 1 && gia_632[g_index_400] == 1)
				gi_712 = true;
				
			if (gia_612[g_index_400] == 1 && gia_636[g_index_400] == 1)
				gi_716 = true;
				
			if (gi_712 == true || gi_716 == true || gia_568[g_index_400] == 1) {
				if (gia_508[g_index_400] == 0 && gia_512[g_index_400] == 0)
					gda_492[g_index_400] = NormalizeDouble(gda_688[g_index_400], li_68);
					
				if (gia_508[g_index_400] > 0 && gi_712 == true)
					gda_492[g_index_400] = NormalizeDouble(gda_468[g_index_400][0][li_80] * LotMultiplicator, li_72);
					
				if (gia_508[g_index_400] > 0 && gi_716 == true)
					gda_492[g_index_400] = NormalizeDouble(gda_468[g_index_400][1][li_84] * LotMultiplicator, li_72);
					
				if (gia_508[g_index_400] > 0 && gia_512[g_index_400] == 0) {
					gda_492[g_index_400] = NormalizeDouble(gda_468[g_index_400][0][li_80] * LotMultiplicator, li_72);
					gda_496[g_index_400] = gda_492[g_index_400];
				}
				
				if (gda_492[g_index_400] < l_minlot_44)
					gda_492[g_index_400] = l_minlot_44;
					
				if (gda_492[g_index_400] > g_maxlot_148)
					gda_492[g_index_400] = g_maxlot_148;
					
				g_ask_344 = MarketInfo(gsa_412[g_index_400], MODE_ASK);
				
				gd_320 = 0;
				
				gd_328 = gia_392[g_index_400][0];
				
				gd_368 = g_ask_344 - gda_656[g_index_400] * l_point_16;
				
				if (gda_656[g_index_400] == 0.0)
					gd_368 = 0;
					
				gd_360 = g_ask_344 + gda_648[g_index_400] * l_point_16;
				
				if (gda_648[g_index_400] == 0.0 || gia_508[g_index_400] > 0)
					gd_360 = 0;
					
				gi_536 = OpOrd(gsa_412[g_index_400], gd_320, gda_492[g_index_400], g_ask_344, gd_368, gd_360, gia_432[g_index_400]);
				
				if (gi_536 > 0) {
					gia_480[g_index_400][0][gia_508[g_index_400] + 1] = gi_536;
					gia_568[g_index_400] = 0;
				}
				
				if (gi_536 < 0)
					gia_568[g_index_400] = 1;
					
				gia_376[g_index_400] += gia_568[g_index_400];
				
				gia_568[g_index_400] = 0;
			}
			
			if (gi_712 == true || gi_716 == true || gia_572[g_index_400] == 1) {
				if (gia_508[g_index_400] == 0 && gia_512[g_index_400] == 0)
					gda_496[g_index_400] = NormalizeDouble(gda_692[g_index_400], li_68);
					
				if (gia_512[g_index_400] > 0 && gi_712 == true)
					gda_496[g_index_400] = NormalizeDouble(gda_468[g_index_400][0][li_80] * LotMultiplicator, li_72);
					
				if (gia_512[g_index_400] > 0 && gi_716 == true)
					gda_496[g_index_400] = NormalizeDouble(gda_468[g_index_400][1][li_84] * LotMultiplicator, li_72);
					
				if (gia_508[g_index_400] == 0 && gia_512[g_index_400] > 0) {
					gda_496[g_index_400] = NormalizeDouble(gda_468[g_index_400][1][li_84] * LotMultiplicator, li_72);
					gda_492[g_index_400] = gda_496[g_index_400];
				}
				
				if (gda_496[g_index_400] < l_minlot_44)
					gda_496[g_index_400] = l_minlot_44;
					
				if (gda_496[g_index_400] > g_maxlot_148)
					gda_496[g_index_400] = g_maxlot_148;
					
				if (gi_712 == true || gi_716 == true)
					g_bid_352 = MarketInfo(gsa_412[g_index_400], MODE_BID);
					
				gd_320 = 1;
				
				gd_328 = gia_392[g_index_400][1];
				
				gd_368 = g_bid_352 + gda_660[g_index_400] * l_point_16;
				
				if (gda_660[g_index_400] == 0.0)
					gd_368 = 0;
					
				gd_360 = g_bid_352 - gda_652[g_index_400] * l_point_16;
				
				if (gda_652[g_index_400] == 0.0 || gia_512[g_index_400] > 0)
					gd_360 = 0;
					
				gi_536 = OpOrd(gsa_412[g_index_400], gd_320, gda_496[g_index_400], g_bid_352, gd_368, gd_360, gia_432[g_index_400]);
				
				if (gi_536 > 0)
					gia_480[g_index_400][1][gia_512[g_index_400] + 1] = gi_536;
					
				if (gi_536 < 0)
					gia_572[g_index_400] = 1;
					
				gia_376[g_index_400] += gia_572[g_index_400];
				
				gia_572[g_index_400] = 0;
			}
			
			Sleep(500);
			
			N();
			
			if (gia_508[g_index_400] > 1 && gia_512[g_index_400] > 1 && Ask > gda_472[g_index_400][0][li_76] + gda_672[g_index_400])
				gia_616[g_index_400] = 1;
				
			if (gia_512[g_index_400] > 1 && gia_508[g_index_400] > 1 && Bid < gda_472[g_index_400][1][li_88] - gda_680[g_index_400])
				gia_620[g_index_400] = 1;
				
			gd_384 = 0;
			
			g_price_736 = 0;
			
			g_price_744 = 0;
			
			if (gia_616[g_index_400] == 1 || gia_576[g_index_400] == 1) {
				for (gi_556 = 1; gi_556 < gia_508[g_index_400]; gi_556++) {
					OrderSelect(gia_480[g_index_400][0][gi_556], SELECT_BY_TICKET, MODE_TRADES);
					gi_536 = OrderClose(OrderTicket(), OrderLots(), MarketInfo(gsa_412[g_index_400], MODE_BID), g_slippage_316);
					
					if (gi_536 == 1 && gia_576[g_index_400] != 1)
						gia_576[g_index_400] = 0;
						
					if (gi_536 == false)
						gia_576[g_index_400] = 1;
						
					gia_376[g_index_400] += gia_576[g_index_400];
				}
				
				gia_576[g_index_400] = 0;
			}
			
			N();
			
			gd_384 = U0(gia_508[g_index_400], gia_512[g_index_400]);
			
			if (gia_616[g_index_400] == 1 || gia_584[g_index_400] == 1) {
				for (gi_552 = 1; gi_552 <= gia_508[g_index_400]; gi_552++) {
					OrderSelect(gia_480[g_index_400][0][gi_552], SELECT_BY_TICKET, MODE_TRADES);
					
					if (OrderType() == OP_BUY && OrderMagicNumber() == gia_432[g_index_400] && OrderSymbol() == gsa_412[g_index_400]) {
						if (gd_384 - gda_700[g_index_400] * l_point_16 != OrderStopLoss() || OrderStopLoss() == 0.0) {
							gi_536 = OrderModify(OrderTicket(), OrderOpenPrice(), gd_384 - gda_700[g_index_400] * l_point_16, g_price_736);
							
							if (gi_536 == false)
								gia_584[g_index_400] = 1;
								
							if (gi_536 == 1 && gia_584[g_index_400] != 1)
								gia_584[g_index_400] = 0;
						}
					}
				}
				
				gia_376[g_index_400] += gia_584[g_index_400];
				
				gia_584[g_index_400] = 0;
			}
			
			if (gia_616[g_index_400] == 1 || gia_588[g_index_400] == 1) {
				for (gi_552 = 1; gi_552 <= gia_512[g_index_400]; gi_552++) {
					OrderSelect(gia_480[g_index_400][1][gi_552], SELECT_BY_TICKET, MODE_TRADES);
					
					if (OrderType() == OP_SELL && OrderMagicNumber() == gia_432[g_index_400] && OrderSymbol() == gsa_412[g_index_400]) {
						if (gd_384 - (gda_700[g_index_400] - l_spread_12) * l_point_16 != OrderTakeProfit() || OrderTakeProfit() == 0.0) {
							gi_536 = OrderModify(OrderTicket(), OrderOpenPrice(), g_price_744, gd_384 - (gda_700[g_index_400] - l_spread_12) * l_point_16);
							
							if (gi_536 == false)
								gia_588[g_index_400] = 1;
								
							if (gi_536 == 1 && gia_588[g_index_400] == 0)
								gia_588[g_index_400] = 0;
						}
					}
				}
				
				gia_376[g_index_400] += gia_588[g_index_400];
				
				gia_588[g_index_400] = 0;
			}
			
			if (gia_620[g_index_400] == 1 || gia_580[g_index_400] == 1) {
				for (gi_556 = 1; gi_556 < gia_512[g_index_400]; gi_556++) {
					OrderSelect(gia_480[g_index_400][1][gi_556], SELECT_BY_TICKET, MODE_TRADES);
					gi_536 = OrderClose(OrderTicket(), OrderLots(), MarketInfo(gsa_412[g_index_400], MODE_ASK), g_slippage_316);
					
					if (gi_536 == 1 && gia_580[g_index_400] != 1)
						gia_580[g_index_400] = 0;
						
					if (gi_536 == false)
						gia_580[g_index_400] = 1;
						
					gia_376[g_index_400] += gia_580[g_index_400];
				}
				
				gia_580[g_index_400] = 0;
			}
			
			N();
			
			gd_384 = U0(gia_508[g_index_400], gia_512[g_index_400]);
			
			if (gia_620[g_index_400] == 1 || gia_592[g_index_400] == 1) {
				for (gi_552 = 1; gi_552 <= gia_508[g_index_400]; gi_552++) {
					OrderSelect(gia_480[g_index_400][0][gi_552], SELECT_BY_TICKET, MODE_TRADES);
					
					if (OrderType() == OP_BUY && OrderMagicNumber() == gia_432[g_index_400] && OrderSymbol() == gsa_412[g_index_400]) {
						if (gd_384 + gda_696[g_index_400] * l_point_16 != OrderTakeProfit() || OrderTakeProfit() == 0.0) {
							gi_536 = OrderModify(OrderTicket(), OrderOpenPrice(), g_price_736, gd_384 + gda_696[g_index_400] * l_point_16);
							
							if (gi_536 == false)
								gia_592[g_index_400] = 1;
								
							if (gi_536 == 1 && gia_592[g_index_400] != 1)
								gia_592[g_index_400] = 0;
						}
					}
				}
				
				gia_376[g_index_400] += gia_592[g_index_400];
				
				gia_592[g_index_400] = 0;
			}
			
			if (gia_620[g_index_400] == 1 || gia_596[g_index_400] == 1) {
				for (gi_552 = 1; gi_552 <= gia_512[g_index_400]; gi_552++) {
					OrderSelect(gia_480[g_index_400][1][gi_552], SELECT_BY_TICKET, MODE_TRADES);
					
					if (OrderType() == OP_SELL && OrderMagicNumber() == gia_432[g_index_400] && OrderSymbol() == gsa_412[g_index_400]) {
						if (gd_384 + (gda_696[g_index_400] + l_spread_12) * l_point_16 != OrderStopLoss() || OrderStopLoss() == 0.0) {
							gi_536 = OrderModify(OrderTicket(), OrderOpenPrice(), gd_384 + (gda_696[g_index_400] + l_spread_12) * l_point_16, g_price_744);
							
							if (gi_536 == false)
								gia_596[g_index_400] = 1;
								
							if (gi_536 == 1 && gia_596[g_index_400] != 1)
								gia_596[g_index_400] = 0;
						}
					}
				}
				
				gia_376[g_index_400] += gia_596[g_index_400];
				
				gia_596[g_index_400] = 0;
			}
			
			gsa_88[g_index_400] = "Îñíîâíàÿ. ÒÑ - ÍÎÐÌÀ";
			
			N();
			
			if ((gia_508[g_index_400] > 1 && gia_512[g_index_400] > 1) || (gia_508[g_index_400] > 1 && gia_512[g_index_400] > 0 && gda_484[g_index_400][1][gia_512[g_index_400]] == 0.0 &&
					gda_488[g_index_400][1][gia_512[g_index_400]] == 0.0) || (gia_508[g_index_400] > 0 && gia_512[g_index_400] > 1 && gda_484[g_index_400][0][gia_508[g_index_400]] == 0.0 && gda_488[g_index_400][0][gia_508[g_index_400]] == 0.0)) {
				ld_108 = UB(gia_508[g_index_400], 0) + gda_768[g_index_400] * l_point_16;
				ld_116 = UB(gia_512[g_index_400], 1) - gda_772[g_index_400] * l_point_16;
				gsa_88[g_index_400] = "Âñïîì., îæèäàíèå âêë. ÒÐÀËËà";
			}
			
			if ((gia_508[g_index_400] > 0 && gia_512[g_index_400] == 0) || (gia_508[g_index_400] > 1 && gia_512[g_index_400] > 1 && Bid > ld_108)) {
				g_price_752 = U0(gia_508[g_index_400], gia_512[g_index_400]) + gda_768[g_index_400] * l_point_16;
				g_price_736 = 0;
				
				if (gia_508[g_index_400] > 1 && gia_512[g_index_400] > 1) {
					g_price_752 = ld_108;
					g_price_736 = 0;
				}
				
				gsa_88[g_index_400] = "Âñïîìîãàòåëüíàÿ. Òðàëë BUY";
				
				if (gia_508[g_index_400] > 0 && g_price_752 < l_bid_24 - gia_664[g_index_400] * l_point_16) {
					for (gi_552 = 1; gi_552 <= gia_508[g_index_400]; gi_552++) {
						OrderSelect(gia_480[g_index_400][0][gi_552], SELECT_BY_TICKET, MODE_TRADES);
						
						if (OrderType() == OP_BUY && OrderMagicNumber() == gia_432[g_index_400] && OrderSymbol() == gsa_412[g_index_400])
							if (l_bid_24 - gia_664[g_index_400] * l_point_16 > OrderStopLoss() || OrderStopLoss() == 0.0)
								gi_536 = OrderModify(gia_480[g_index_400][0][gi_552], OrderOpenPrice(), l_bid_24 - gia_664[g_index_400] * l_point_16, g_price_736);
					}
				}
			}
			
			
			if ((gia_508[g_index_400] == 0 && gia_512[g_index_400] > 0) || (gia_508[g_index_400] > 1 && gia_512[g_index_400] > 1 && Ask < ld_116)) {
				g_price_760 = U0(gia_508[g_index_400], gia_512[g_index_400]) - gda_772[g_index_400] * l_point_16;
				g_price_744 = 0;
				
				if (gia_508[g_index_400] > 1 && gia_512[g_index_400] > 1) {
					g_price_760 = ld_116;
					g_price_744 = 0;
				}
				
				gsa_88[g_index_400] = "Âñïîìîãàòåëüíàÿ. Òðàëë SELL";
				
				
				if (gia_512[g_index_400] > 0 && g_price_760 > l_ask_32 + gia_668[g_index_400] * l_point_16) {
					for (gi_552 = 1; gi_552 <= gia_512[g_index_400]; gi_552++) {
						OrderSelect(gia_480[g_index_400][1][gi_552], SELECT_BY_TICKET, MODE_TRADES);
						
						if (OrderType() == OP_SELL && OrderMagicNumber() == gia_432[g_index_400] && OrderSymbol() == gsa_412[g_index_400])
							if (l_ask_32 + gia_668[g_index_400] * l_point_16 < OrderStopLoss() || OrderStopLoss() == 0.0)
								gi_536 = OrderModify(gia_480[g_index_400][1][gi_552], OrderOpenPrice(), l_ask_32 + gia_668[g_index_400] * l_point_16, g_price_744);
					}
				}
			}
			
			
			gia_576[g_index_400] = 0;
			
			gia_580[g_index_400] = 0;
			gia_584[g_index_400] = 0;
			gia_588[g_index_400] = 0;
			gia_592[g_index_400] = 0;
			gia_596[g_index_400] = 0;
			
			if (gia_508[g_index_400] == 1 && gia_512[g_index_400] > 1) {
				for (gi_552 = 1; gi_552 <= gia_512[g_index_400]; gi_552++) {
					if (gda_484[g_index_400][1][gi_552] == 0.0 && gda_488[g_index_400][1][gi_552] == 0.0) {
						gia_584[g_index_400] = 1;
						gia_588[g_index_400] = 1;
					}
				}
				
				if (gda_484[g_index_400][0][gia_508[g_index_400]] == 0.0 && gda_488[g_index_400][0][gia_508[g_index_400]] == 0.0) {
					gia_584[g_index_400] = 1;
					gia_588[g_index_400] = 1;
				}
			}
			
			if (gia_508[g_index_400] > 1 && gia_512[g_index_400] == 1) {
				for (gi_552 = 1; gi_552 <= gia_508[g_index_400]; gi_552++) {
					if (gda_484[g_index_400][0][gi_552] == 0.0 && gda_488[g_index_400][0][gi_552] == 0.0) {
						gia_592[g_index_400] = 1;
						gia_596[g_index_400] = 1;
					}
				}
				
				if (gda_484[g_index_400][1][gia_512[g_index_400]] == 0.0 && gda_488[g_index_400][1][gia_512[g_index_400]] == 0.0) {
					gia_592[g_index_400] = 1;
					gia_596[g_index_400] = 1;
				}
			}
		}
	}
	
}

int Turtle::OpOrd(String a_symbol_0, int a_cmd_8, double a_lots_12, double a_price_20, double a_price_28, double a_price_36, int a_magic_44) {
	int l_ticket_52;
	l_ticket_52 = OrderSend(a_symbol_0, a_cmd_8, a_lots_12, a_price_20, 6, a_price_28, a_price_36, "" + g_index_400, a_magic_44);
	return (l_ticket_52);
}

double Turtle::UB(int ai_0, int ai_4) {
	double ld_8 = 0;
	double ld_16 = 0;
	
	for (gi_552 = 1; gi_552 <= ai_0; gi_552++) {
		ld_8 += gda_472[g_index_400][ai_4][gi_552] * gda_468[g_index_400][ai_4][gi_552];
		ld_16 += gda_468[g_index_400][ai_4][gi_552];
	}
	
	if (ld_16 == 0.0)
		return (0);
		
	return (ld_8 / ld_16);
}

double Turtle::U0(int ai_0, int ai_4) {
	double ld_8 = 0;
	double ld_16 = 0;
	double ld_24 = 0;
	double ld_32 = 0;
	
	for (gi_552 = 1; gi_552 <= ai_0; gi_552++) {
		ld_8 += gda_472[g_index_400][0][gi_552] * gda_468[g_index_400][0][gi_552];
		ld_16 += gda_468[g_index_400][0][gi_552];
	}
	
	for (gi_552 = 1; gi_552 <= ai_4; gi_552++) {
		ld_24 += gda_472[g_index_400][1][gi_552] * gda_468[g_index_400][1][gi_552];
		ld_32 += gda_468[g_index_400][1][gi_552];
	}
	
	if (ld_16 - ld_32 == 0.0)
		return (0);
		
	return ((ld_8 - ld_24) / (ld_16 - ld_32));
}

int Turtle::MIN(int ai_0, int ai_4) {
	int li_ret_16;
	double ld_8 = 1000;
	
	for (gi_552 = 1; gi_552 <= ai_4; gi_552++) {
		if (ld_8 >= gda_472[g_index_400][ai_0][gi_552]) {
			ld_8 = gda_472[g_index_400][ai_0][gi_552];
			li_ret_16 = gi_552;
		}
	}
	
	return (li_ret_16);
}

int Turtle::MAX(int ai_0, int ai_4) {
	int li_ret_16;
	double ld_8 = 0;
	
	for (gi_552 = 1; gi_552 <= ai_4; gi_552++) {
		if (ld_8 <= gda_472[g_index_400][ai_0][gi_552]) {
			ld_8 = gda_472[g_index_400][ai_0][gi_552];
			li_ret_16 = gi_552;
		}
	}
	
	return (li_ret_16);
}

int Turtle::NOV(int ai_0, int ai_4, double ad_8) {
	int li_ret_24;
	double ld_16 = 1000;
	
	for (gi_552 = 1; gi_552 <= ai_4; gi_552++) {
		if (ld_16 >= gda_472[g_index_400][ai_0][gi_552] && ad_8 <= gda_472[g_index_400][ai_0][gi_552]) {
			ld_16 = gda_472[g_index_400][ai_0][gi_552];
			li_ret_24 = gi_552;
		}
	}
	
	return (li_ret_24);
}

int Turtle::NON(int ai_0, int ai_4, double ad_8) {
	int li_ret_24;
	double ld_16 = 0;
	
	for (gi_552 = 1; gi_552 <= ai_4; gi_552++) {
		if (ld_16 <= gda_472[g_index_400][ai_0][gi_552] && ad_8 >= gda_472[g_index_400][ai_0][gi_552]) {
			ld_16 = gda_472[g_index_400][ai_0][gi_552];
			li_ret_24 = gi_552;
		}
	}
	
	return (li_ret_24);
}

void Turtle::N() {
	gia_508[g_index_400] = 0;
	gia_512[g_index_400] = 0;
	gia_516[g_index_400] = 0;
	gia_520[g_index_400] = 0;
	int l_count_0 = 0;
	int l_count_4 = 0;
	int l_count_8 = 0;
	int l_count_12 = 0;
	int l_count_16 = 0;
	int l_count_20 = 0;
	gda_500[g_index_400][0] = 0;
	gda_500[g_index_400][1] = 0;
	gda_500[g_index_400][2] = 0;
	gda_500[g_index_400][3] = 0;
	gda_504[g_index_400][0] = 0;
	gda_504[g_index_400][1] = 0;
	g_ord_total_540 = OrdersTotal();
	
	for (g_pos_532 = 0; g_pos_532 < g_ord_total_540; g_pos_532++) {
		OrderSelect(g_pos_532, SELECT_BY_POS, MODE_TRADES);
		
		if (OrderType() == OP_BUY && OrderMagicNumber() == gia_432[g_index_400] && OrderSymbol() == gsa_412[g_index_400]) {
			l_count_0++;
			gia_480[g_index_400][0][l_count_0] = OrderTicket();
			gda_472[g_index_400][0][l_count_0] = OrderOpenPrice();
			gda_468[g_index_400][0][l_count_0] = OrderLots();
			gda_476[g_index_400][0][l_count_0] = OrderProfit();
			gda_484[g_index_400][0][l_count_0] = OrderTakeProfit();
			gda_488[g_index_400][0][l_count_0] = OrderStopLoss();
			gda_504[g_index_400][0] += gda_476[g_index_400][0][l_count_0];
			gda_500[g_index_400][0] += gda_468[g_index_400][0][l_count_0];
			
			if (gda_488[g_index_400][0][l_count_0] > 0.0)
				l_count_8++;
		}
		
		if (OrderType() == OP_SELL && OrderMagicNumber() == gia_432[g_index_400] && OrderSymbol() == gsa_412[g_index_400]) {
			l_count_4++;
			gia_480[g_index_400][1][l_count_4] = OrderTicket();
			gda_472[g_index_400][1][l_count_4] = OrderOpenPrice();
			gda_468[g_index_400][1][l_count_4] = OrderLots();
			gda_476[g_index_400][1][l_count_4] = OrderProfit();
			gda_484[g_index_400][1][l_count_4] = OrderTakeProfit();
			gda_488[g_index_400][1][l_count_4] = OrderStopLoss();
			gda_504[g_index_400][1] += gda_476[g_index_400][1][l_count_4];
			gda_500[g_index_400][1] += gda_468[g_index_400][1][l_count_4];
			
			if (gda_488[g_index_400][1][l_count_4] > 0.0)
				l_count_12++;
		}
		
		if (OrderType() == OP_BUYSTOP && OrderMagicNumber() == gia_432[g_index_400] && OrderSymbol() == gsa_412[g_index_400]) {
			l_count_16++;
			gia_480[g_index_400][2][l_count_16] = OrderTicket();
			gda_472[g_index_400][2][l_count_16] = OrderOpenPrice();
			gda_484[g_index_400][2][l_count_16] = OrderTakeProfit();
			gda_488[g_index_400][2][l_count_16] = OrderStopLoss();
			gda_468[g_index_400][2][l_count_16] = OrderLots();
			gda_500[g_index_400][2] += gda_468[g_index_400][2][l_count_16];
		}
		
		if (OrderType() == OP_SELLSTOP && OrderMagicNumber() == gia_432[g_index_400] && OrderSymbol() == gsa_412[g_index_400]) {
			l_count_20++;
			gda_472[g_index_400][3][l_count_20] = OrderOpenPrice();
			gia_480[g_index_400][3][l_count_20] = OrderTicket();
			gda_484[g_index_400][3][l_count_20] = OrderTakeProfit();
			gda_488[g_index_400][3][l_count_20] = OrderStopLoss();
			gda_468[g_index_400][3][l_count_20] = OrderLots();
			gda_500[g_index_400][3] += gda_468[g_index_400][3][l_count_20];
		}
	}
	
	gia_508[g_index_400] = l_count_0;
	
	gia_512[g_index_400] = l_count_4;
	gia_524[g_index_400] = l_count_8;
	gia_528[g_index_400] = l_count_12;
	gia_516[g_index_400] = l_count_16;
	gia_520[g_index_400] = l_count_20;
}

double Turtle::GetProfitForDay(int ai_0) {
	double ld_ret_4 = 0;
	
	Time today = Now;
	today.hour = 0;
	today.minute = 0;
	today.second = 0;
	
	for (int l_pos_12 = 0; l_pos_12 < OrdersHistoryTotal(); l_pos_12++) {
		if (!(OrderSelect(l_pos_12, SELECT_BY_POS, MODE_HISTORY)))
			break;
			
		if (OrderSymbol() == gsa_412[g_index_400] && OrderMagicNumber() == gia_432[0] || OrderMagicNumber() == gia_432[1] || OrderMagicNumber() == gia_432[2])
			if (OrderCloseTime() >= today && OrderCloseTime() < today + 86400)
				ld_ret_4 = ld_ret_4 + OrderProfit() + OrderCommission() + OrderSwap();
	}
	
	return (ld_ret_4);
}

}
