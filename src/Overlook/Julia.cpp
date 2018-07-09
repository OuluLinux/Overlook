#include "Overlook.h"

namespace Overlook {

Julia::Julia() {
	
}

void Julia::InitEA() {
	last_open_pos = 0;
}

void Julia::StartEA(int pos) {
	int li_8;
	
	if (MMoney) {
		BLot = NormalizeDouble(AccountBalance() * AccountLeverage() / 2000000.0, 1);
		if (BLot < 0.01) BLot = 0.01;
	}
	else
		BLot = 0.01;
		
	if (pos >= Bars*0.5) {
		pos = pos;
	}
	int l_ord_total_4 = OrdersTotal();
	
	g_count_236 = 0;
	int open_count = 0;
	for (int l_pos_0 = 0; l_pos_0 < l_ord_total_4; l_pos_0++) {
		if (OrderSelect(l_pos_0, SELECT_BY_POS)) {
			if (OrderSymbol() == Symbol())
				g_count_236++;
			if (OrderType() <= OP_SELL)
				open_count++;
		}
	}
	
	if (open_count > 0) {
		last_open_pos = pos;
	}
	int not_open_bars = pos - last_open_pos;
	if (not_open_bars >= idle_bars_avoid) {
		for (int i = OrdersTotal() - 1; i >= 0; i--) {
			OrderSelect(i, SELECT_BY_POS);
			OrderDelete(OrderTicket());
		}
	}
	
	if (g_count_236 == 0) {
		RefreshRates();
		g_price_132 = NormalizeDouble(Ask, 4);
		g_price_140 = NormalizeDouble(g_price_132 + 0.0034 * Rate, 4);
		g_price_148 = NormalizeDouble(g_price_132 + 0.0089 * Rate, 4);
		g_price_156 = NormalizeDouble(g_price_132 + 0.0144 * Rate, 4);
		g_price_164 = NormalizeDouble(g_price_132 + 0.0233 * Rate, 4);
		g_price_172 = NormalizeDouble(g_price_132 - 0.0034 * Rate, 4);
		g_price_180 = NormalizeDouble(g_price_132 - 0.0089 * Rate, 4);
		g_price_188 = NormalizeDouble(g_price_132 - 0.0144 * Rate, 4);
		g_price_196 = NormalizeDouble(g_price_132 - 0.0233 * Rate, 4);
		g_ticket_204 = OrderSend(Symbol(), OP_BUYSTOP, BLot, g_price_140, 0, g_price_132, g_price_148);
		
		if (g_ticket_204 < 0) {
			return;
		}
		
		g_ticket_208 = OrderSend(Symbol(), OP_BUYSTOP, BLot, g_price_140, 0, g_price_132, g_price_156);
		
		if (g_ticket_208 < 0) {
			return;
		}
		
		g_ticket_212 = OrderSend(Symbol(), OP_BUYSTOP, BLot, g_price_140, 0, g_price_132, g_price_164);
		
		if (g_ticket_212 < 0) {
			return;
		}
		
		g_ticket_216 = OrderSend(Symbol(), OP_SELLSTOP, BLot, g_price_172, 0, g_price_132, g_price_180);
		
		if (g_ticket_216 < 0) {
			return;
		}
		
		g_ticket_220 = OrderSend(Symbol(), OP_SELLSTOP, BLot, g_price_172, 0, g_price_132, g_price_188);
		
		if (g_ticket_220 < 0) {
			return;
		}
		
		g_ticket_224 = OrderSend(Symbol(), OP_SELLSTOP, BLot, g_price_172, 0, g_price_132, g_price_196);
		
		if (g_ticket_224 < 0) {
			return;
		}
		
		gi_104 = true;
	}
	
	else {
		if (gi_104) {
			for (int l_pos_0 = 0; l_pos_0 < l_ord_total_4; l_pos_0++) {
				if (OrderSelect(l_pos_0, SELECT_BY_POS)) {
					if (OrderSymbol() == Symbol() && OrderType() <= OP_SELL) {
						if (OrderTicket() == g_ticket_204 || OrderTicket() == g_ticket_208 || OrderTicket() == g_ticket_212) {
							if (!OrderModify(g_ticket_216, g_price_132, g_price_140, g_price_172))
								;
								
							if (!OrderModify(g_ticket_220, g_price_132, g_price_140, g_price_180))
								;
								
							if (!OrderModify(g_ticket_224, g_price_132, g_price_140, g_price_188))
								;
								
							if (Martin) {
								g_ticket_228 = OrderSend(Symbol(), OP_SELLSTOP, BLot * NLot, g_price_132, 0, g_price_140, g_price_172);
								
							}
							
							gi_112 = true;
							
							gi_104 = false;
							gi_108 = false;
							gi_116 = true;
							gi_120 = false;
							gi_124 = false;
							gi_128 = false;
							break;
						}
					}
				}
			}
			
			for (int l_pos_0 = 0; l_pos_0 < l_ord_total_4; l_pos_0++) {
				if (OrderSelect(l_pos_0, SELECT_BY_POS)) {
					if (OrderSymbol() == Symbol() && OrderType() <= OP_SELL) {
						if (OrderTicket() == g_ticket_216 || OrderTicket() == g_ticket_220 || OrderTicket() == g_ticket_224) {
							if (!OrderModify(g_ticket_204, g_price_132, g_price_172, g_price_140))
								;
								
							if (!OrderModify(g_ticket_208, g_price_132, g_price_172, g_price_148))
								;
								
							if (!OrderModify(g_ticket_212, g_price_132, g_price_172, g_price_156))
								;
								
							if (Martin) {
								g_ticket_228 = OrderSend(Symbol(), OP_BUYSTOP, BLot * NLot, g_price_132, 0, g_price_172, g_price_140);
								
							}
							
							gi_112 = false;
							
							gi_104 = false;
							gi_108 = false;
							gi_124 = true;
							gi_128 = false;
							gi_116 = false;
							gi_120 = false;
							break;
						}
					}
				}
			}
		}
		
		else {
			if (gi_112) {
				if (gi_116) {
					if (OrderSelect(g_ticket_204, SELECT_BY_TICKET) == true) {
						if (OrderType() == OP_BUY) {
							if (OrderClosePrice() > OrderStopLoss()) {
								if (!OrderModify(g_ticket_208, g_price_140, g_price_140, g_price_156))
									;
									
								if (!OrderModify(g_ticket_212, g_price_140, g_price_140, g_price_164))
									;
									
								if (!OrderDelete(g_ticket_216))
									;
									
								if (!OrderDelete(g_ticket_220))
									;
									
								if (!OrderDelete(g_ticket_224))
									;
									
								if (Martin)
									if (!OrderDelete(g_ticket_228))
										;
										
								gi_120 = true;
							}
							
							else {
								if (Martin) {
									g_ticket_232 = g_ticket_228;
									g_ticket_228 = OrderSend(Symbol(), OP_BUYSTOP, 4.0 * (BLot * NLot), g_price_140, 0, g_price_132, g_price_148);
									
									if (g_ticket_228 < 0)
										Print("Îøèáêà #"+ GetLastError() + " ïðè îòêðûòèè îðäåðà õ4");
								}
								
								gi_124 = true;
							}
							
							gi_116 = false;
						}
					}
				}
				
				if (gi_120) {
					if (OrderSelect(g_ticket_208, SELECT_BY_TICKET) == true) {
						if (OrderType() == OP_BUY) {
							if (OrderClosePrice() > OrderStopLoss()) {
								if (!OrderModify(g_ticket_212, g_price_140, g_price_148, g_price_164))
									Print("Îøèáêà #"+ GetLastError() + " ïðè èçìåíåíèè îðäåðà Buy_3");
									
								gi_120 = false;
							}
						}
					}
				}
				
				if (gi_124) {
					if (OrderSelect(g_ticket_216, SELECT_BY_TICKET) == true) {
						if (OrderType() == OP_SELL) {
							if (OrderClosePrice() < OrderStopLoss()) {
								if (!OrderModify(g_ticket_220, g_price_132, g_price_132, g_price_180))
									Print("Îøèáêà #"+ GetLastError() + " ïðè èçìåíåíèè îðäåðà Sell_2");
									
								if (!OrderModify(g_ticket_224, g_price_132, g_price_132, g_price_188))
									Print("Îøèáêà #"+ GetLastError() + " ïðè èçìåíåíèè îðäåðà Sell_3");
									
								if (Martin)
									if (!OrderDelete(g_ticket_228))
										Print("Îøèáêà #"+ GetLastError() + " ïðè óäàëåíèè îðäåðà õ4");
										
								gi_124 = false;
								
								gi_128 = true;
							}
							
							else
								if (Martin)
									gi_108 = true;
						}
					}
				}
				
				if (gi_128) {
					if (OrderSelect(g_ticket_220, SELECT_BY_TICKET) == true) {
						if (OrderType() == OP_SELL) {
							if (OrderClosePrice() < OrderStopLoss()) {
								if (!OrderModify(g_ticket_224, g_price_132, g_price_172, g_price_188))
									Print("Îøèáêà #"+ GetLastError() + " ïðè èçìåíåíèè îðäåðà Sell_3");
									
								gi_128 = false;
							}
						}
					}
				}
			}
			
			else {
				if (gi_124) {
					if (OrderSelect(g_ticket_216, SELECT_BY_TICKET) == true) {
						if (OrderType() == OP_SELL) {
							if (OrderClosePrice() < OrderStopLoss()) {
								if (!OrderModify(g_ticket_220, g_price_172, g_price_172, g_price_188))
									Print("Îøèáêà #"+ GetLastError() + " ïðè èçìåíåíèè îðäåðà Sell_2");
									
								if (!OrderModify(g_ticket_224, g_price_172, g_price_172, g_price_196))
									Print("Îøèáêà #"+ GetLastError() + " ïðè èçìåíåíèè îðäåðà Sell_3");
									
								if (!OrderDelete(g_ticket_204))
									Print("Îøèáêà #"+ GetLastError() + " ïðè óäàëåíèè îðäåðà Buy_1");
									
								if (!OrderDelete(g_ticket_208))
									Print("Îøèáêà #"+ GetLastError() + " ïðè óäàëåíèè îðäåðà Buy_2");
									
								if (!OrderDelete(g_ticket_212))
									Print("Îøèáêà #"+ GetLastError() + " ïðè óäàëåíèè îðäåðà Buy_3");
									
								if (Martin)
									if (!OrderDelete(g_ticket_228))
										Print("Îøèáêà #"+ GetLastError() + " ïðè óäàëåíèè îðäåðà Next");
										
								gi_128 = true;
							}
							
							else {
								if (Martin) {
									g_ticket_232 = g_ticket_228;
									g_ticket_228 = OrderSend(Symbol(), OP_SELLSTOP, 4.0 * (BLot * NLot), g_price_172, 0, g_price_132, g_price_180);
									
									if (g_ticket_228 < 0)
										Print("Îøèáêà #"+ GetLastError() + " ïðè îòêðûòèè îðäåðà õ4");
								}
								
								gi_116 = true;
							}
							
							gi_124 = false;
						}
					}
				}
			}
			
			if (gi_128) {
				if (OrderSelect(g_ticket_220, SELECT_BY_TICKET) == true) {
					if (OrderType() == OP_SELL) {
						if (OrderClosePrice() < OrderStopLoss()) {
							if (!OrderModify(g_ticket_224, g_price_172, g_price_180, g_price_196))
								Print("Îøèáêà #"+ GetLastError() + " ïðè èçìåíåíèè îðäåðà Sell_3");
								
							gi_128 = false;
						}
					}
				}
			}
			
			if (gi_116) {
				if (OrderSelect(g_ticket_204, SELECT_BY_TICKET) == true) {
					if (OrderType() == OP_BUY) {
						if (OrderClosePrice() > OrderStopLoss()) {
							if (!OrderModify(g_ticket_208, g_price_132, g_price_132, g_price_148))
								Print("Îøèáêà #"+ GetLastError() + " ïðè èçìåíåíèè îðäåðà Buy_2");
								
							if (!OrderModify(g_ticket_212, g_price_132, g_price_132, g_price_156))
								Print("Îøèáêà #"+ GetLastError() + " ïðè èçìåíåíèè îðäåðà Buy_3");
								
							if (Martin)
								if (!OrderDelete(g_ticket_228))
									Print("Îøèáêà #"+ GetLastError() + " ïðè óäàëåíèè îðäåðà õ4");
									
							gi_116 = false;
							
							gi_120 = true;
						}
						
						else
							if (Martin)
								gi_108 = true;
					}
				}
			}
			
			if (gi_120) {
				if (OrderSelect(g_ticket_208, SELECT_BY_TICKET) == true) {
					if (OrderType() == OP_BUY) {
						if (OrderClosePrice() > OrderStopLoss()) {
							if (!OrderModify(g_ticket_212, g_price_132, g_price_140, g_price_156))
								Print("Îøèáêà #"+ GetLastError() + " ïðè èçìåíåíèè îðäåðà Buy_3");
								
							gi_120 = false;
						}
					}
				}
			}
		}
		
		if (gi_108) {
			if (OrderSelect(g_ticket_232, SELECT_BY_TICKET) == true) {
				if (OrderType() == OP_BUY || OrderType() == OP_SELL) {
					if (true) {
						if ((OrderClosePrice() <= OrderStopLoss() && OrderType() == OP_BUY) || (OrderClosePrice() >= OrderStopLoss() && OrderType() == OP_SELL)) {
							g_ticket_232 = g_ticket_228;
							
							if (OrderLots() == NormalizeDouble(BLot * NLot, 2))
								li_8 = 2;
							else
								li_8 = 1;
								
								
							g_ticket_228 = OrderSend(Symbol(), OrderType() + 4, 4.0 * (OrderLots() * li_8), OrderOpenPrice(), 0, OrderStopLoss(), OrderTakeProfit());
							
							if (g_ticket_228 < 0) {
								Print("Îøèáêà #"+ GetLastError() + " ïðè îòêðûòèè îðäåðà x4");
								return;
							}
						}
						
						else {
							if (!OrderDelete(g_ticket_228))
								Print("Îøèáêà "+ GetLastError() + " ïðè óäàëåíèè îðäåðà");
								
							gi_108 = false;
						}
					}
				}
			}
		}
	}
	
}

}
