#include "Overlook.h"

namespace Overlook {

void System::InitContent() {
	ASSERT(allowed_symbols.IsEmpty());
	
#ifndef flagHAVE_ALLSYM

	allowed_symbols.Add("EURUSD", 3); // 0.26
	allowed_symbols.Add("GBPUSD", 3); // 0.23
	allowed_symbols.Add("USDJPY", 3); // 0.27
	allowed_symbols.Add("USDCAD", 3); // 0.24
	allowed_symbols.Add("EURJPY", 3); // 0.23
	allowed_symbols.Add("EURCHF", 3); // 0.26
	
#else

	// also AUD,NZD,CHF included
	allowed_symbols.Add("EURUSD",  3); // lts
	allowed_symbols.Add("GBPUSD",  3); // lts
	allowed_symbols.Add("USDJPY",  3); // lts
	allowed_symbols.Add("USDCHF",  3); // lts
	allowed_symbols.Add("USDCAD",  3); // lts
	allowed_symbols.Add("AUDUSD",  3); // lts
	allowed_symbols.Add("NZDUSD",  3); // lts
	allowed_symbols.Add("EURJPY",  3); // lts
	allowed_symbols.Add("EURCHF",  3); // lts
	allowed_symbols.Add("EURGBP",  3); // lts
	allowed_symbols.Add("AUDCAD", 10); // lts
	allowed_symbols.Add("AUDJPY", 10); // lts
	allowed_symbols.Add("CADJPY", 10); // lts
	allowed_symbols.Add("CHFJPY", 10); // lts
	//allowed_symbols.Add("NZDCAD", 10);
	//allowed_symbols.Add("NZDCHF", 10);
	allowed_symbols.Add("EURAUD",  7); // lts
	allowed_symbols.Add("GBPCHF",  7); // lts
	allowed_symbols.Add("GBPJPY",  7); // lts
	allowed_symbols.Add("AUDNZD", 12); // lts
	allowed_symbols.Add("EURCAD", 12); // lts
	//allowed_symbols.Add("GBPAUD", 12);
	//allowed_symbols.Add("GBPCAD", 12);
	//allowed_symbols.Add("GBPNZD", 12);

#endif
	
	ASSERT(allowed_symbols.GetCount() == SYM_COUNT);
	
	
	MetaTrader& mt = GetMetaTrader();
	const Vector<Price>& askbid = mt._GetAskBid();
	String not_found;

	for (int j = 0; j < allowed_symbols.GetCount(); j++) {
		const String& allowed_sym = allowed_symbols.GetKey(j);
		bool found = false;

		for (int i = 0; i < mt.GetSymbolCount(); i++) {
			const Symbol& sym = mt.GetSymbol(i);

			if (sym.IsForex() && (sym.name.Left(6)) == allowed_sym) {
				double base_spread = 1000.0 * (askbid[i].ask / askbid[i].bid - 1.0);

				if (base_spread >= 0.5) {
					Cout() << "Warning! Too much spread: " << sym.name << " (" << base_spread << ")" << "\n";
				}

				sym_ids.Add(i);

				spread_points.Add(allowed_symbols[j] * sym.point);
				found = true;
				break;
			}
		}

		if (!found)
			not_found << allowed_sym << " ";
	}

	ASSERTUSER_(sym_ids.GetCount() == SYM_COUNT, "All required forex instruments weren't shown in the mt4: " + not_found);
	
	
	main_tf = FindPeriod(1);
	ASSERT(main_tf != -1);
	
	
	int stoch_id	= Find<StochasticOscillator>();
	int osma_id		= Find<OsMA>();
	int volav_id	= Find<VolatilityAverage>();
	int zz_id		= Find<ZigZag>();
	int ma_id		= Find<MovingAverage>();
	int mom_id		= Find<Momentum>();
	for(int i = 0; i < TF_COUNT; i++) {
		// tfs: 4, 8, 16, 32, 64
		int period = (1 << (3 + i));
		indi_ids.Add().Set(volav_id).AddArg(period);
		indi_ids.Add().Set(osma_id).AddArg(period).AddArg(period*2).AddArg(period);
		indi_ids.Add().Set(stoch_id).AddArg(period);
		
		label_indi_ids.Add().Set(zz_id).AddArg(period).AddArg(period);
		label_indi_ids.Add().Set(ma_id).AddArg(period).AddArg(-period/2);
		label_indi_ids.Add().Set(mom_id).AddArg(period).AddArg(-period/2);
	}
	
	ASSERT(indi_ids.GetCount() == TRUEINDI_COUNT * TF_COUNT);
	ASSERT(label_indi_ids.GetCount() == LABELINDI_COUNT * TF_COUNT);
}

void System::RefreshWorkQueue() {

	// Add proxy symbols to the queue if any
	Index<int> tf_ids, sym_ids;
	tf_ids.Add(main_tf);
	sym_ids <<= this->sym_ids;

	for (int i = 0; i < sym_ids.GetCount(); i++) {
		const Symbol& sym = GetMetaTrader().GetSymbol(sym_ids[i]);

		if (sym.proxy_id == -1)
			continue;

		sym_ids.FindAdd(sym.proxy_id);
	}

	GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);


	// Get DataBridge work queue
	Vector<FactoryDeclaration> db_indi_ids;
	db_indi_ids.Add().Set(Find<DataBridge>());
	GetCoreQueue(db_queue, sym_ids, tf_ids, db_indi_ids);


	// Get label indicator work queue
	GetCoreQueue(label_queue, sym_ids, tf_ids, label_indi_ids);
}


void System::ProcessWorkQueue() {
	WhenPushTask("Processing work queue");

	work_lock.Enter();

	for (int i = 0; i < work_queue.GetCount(); i++) {
		WhenSubProgress(i, work_queue.GetCount());
		Process(*work_queue[i]);
	}

	work_lock.Leave();
	
	// Update time buffers also
	int data_count = GetCountMain();
	time_buffers.SetCount(TIMEBUF_COUNT);
	for(int i = 0; i < time_buffers.GetCount(); i++) {
		Buffer& buf = time_buffers[i];
		buf.SetCount(data_count);
		if (i == TIMEBUF_WEEKTIME) {
			for(int j = 0; j < data_count; j++) {
				Time t = GetTimeTf(main_tf, j);
				int wday = DayOfWeek(t);
				double d = (((wday-1) * 24 + t.hour) * 60 + t.minute) / (5.0 * 24.0 * 60.0);
				buf.Set(j, d);
			}
		}
	}
	
	WhenPopTask();
}

void System::ProcessLabelQueue() {
	WhenPushTask("Processing label queue");

	work_lock.Enter();

	for (int i = 0; i < label_queue.GetCount(); i++) {
		WhenSubProgress(i, label_queue.GetCount());
		Process(*label_queue[i]);
	}

	work_lock.Leave();

	WhenPopTask();
}

void System::ProcessDataBridgeQueue() {
	WhenPushTask("Processing databridge work queue");

	work_lock.Enter();

	for (int i = 0; i < db_queue.GetCount(); i++) {
		WhenSubProgress(i, db_queue.GetCount());
		Process(*db_queue[i]);
	}

	work_lock.Leave();

	WhenPopTask();
}

void System::ResetValueBuffers() {
	
	// Get total count of output buffers in the indicator list
	VectorMap<unsigned, int> bufout_ids;
	int buf_id = 0;
	for (int i = 0; i < indi_ids.GetCount(); i++) {
		FactoryDeclaration& decl = indi_ids[i];
		const FactoryRegister& reg = GetRegs()[decl.factory];
		for (int j = decl.arg_count; j < reg.args.GetCount(); j++)
			decl.AddArg(reg.args[j].def);
		bufout_ids.Add(decl.GetHashValue(), buf_id);
		buf_id += reg.out[0].visible;
	}
	buf_count = buf_id;
	ASSERT(buf_count);
	
	
	// Get DataBridge core pointer for easy reading
	databridge_cores.SetCount(0);
	databridge_cores.SetCount(GetSymbolCount(), NULL);
	open_buffers.SetCount(0);
	open_buffers.SetCount(GetSymbolCount(), NULL);
	int factory = Find<DataBridge>();

	for (int i = 0; i < db_queue.GetCount(); i++) {
		CoreItem& ci = *db_queue[i];

		if (ci.factory != factory)
			continue;

		databridge_cores[ci.sym] = &*ci.core;
		open_buffers[ci.sym] = &ci.core->GetBuffer(0);
	}


	// Reserve zeroed memory for output buffer pointer vector
	value_buffers.Clear();
	value_buffers.SetCount(sym_ids.GetCount());
	for (int i = 0; i < sym_ids.GetCount(); i++) {
		value_buffers[i].SetCount(buf_count, NULL);
	}


	// Get output buffer pointer vector
	int bars = GetCountTf(main_tf);
	int total_bufs = 0;
	data_begin = 0;

	for (int i = 0; i < work_queue.GetCount(); i++) {
		CoreItem& ci = *work_queue[i];
		ASSERT(!ci.core.IsEmpty());
		const Core& core = *ci.core;
		const Output& output = core.GetOutput(0);

		int sym_id = sym_ids.Find(ci.sym);

		if (sym_id == -1)
			continue;

		if (ci.tf != main_tf)
			continue;

		DataBridge* db = dynamic_cast<DataBridge*>(&*ci.core);

		if (db) {
			int begin = db->GetDataBegin();
			int limit = bars - 10000;
			bool enough = begin < limit;
			ASSERTUSER_(enough, "Symbol " + GetMetaTrader().GetSymbol(ci.sym).name + " has no proper data.");

			if (begin > data_begin) {
				LOG("Limiting symbol " << GetMetaTrader().GetSymbol(ci.sym).name << " " << bars - begin);
				data_begin = begin;
			}
		}

		Vector<ConstBuffer*>& indi_buffers = value_buffers[sym_id];
		const FactoryRegister& reg = GetRegs()[ci.factory];

		FactoryDeclaration decl;
		decl.Set(ci.factory);

		for (int j = 0; j < ci.args.GetCount(); j++)
			decl.AddArg(ci.args[j]);

		for (int j = decl.arg_count; j < reg.args.GetCount(); j++)
			decl.AddArg(reg.args[j].def);

		unsigned hash = decl.GetHashValue();


		// Check that args match to declaration
#ifdef flagDEBUG
		ArgChanger ac;
		ac.SetLoading();
		ci.core->IO(ac);

		ASSERT(ac.args.GetCount() >= ci.args.GetCount());

		for (int i = 0; i < ci.args.GetCount(); i++) {
			int a = ac.args[i];
			int b = ci.args[i];
			if (a != b) {
				LOG(Format("%d != %d", a, b));
			}
			ASSERT(a == b);
		}

#endif


		int buf_begin_id = bufout_ids.Find(hash);

		if (buf_begin_id == -1)
			continue;

		int buf_begin = bufout_ids[buf_begin_id];

		//LOG(i << ": " << ci.factory << ", " << sym_id << ", " << (int64)hash << ", " << buf_begin);

		for (int l = 0; l < reg.out[0].visible; l++) {
			int buf_pos = buf_begin + l;
			ConstBuffer*& bufptr = indi_buffers[buf_pos];
			ASSERT_(bufptr == NULL, "Duplicate work item");
			bufptr = &output.buffers[l];
			total_bufs++;
		}
	}

	int expected_total = sym_ids.GetCount() * buf_count;

	ASSERT_(total_bufs == expected_total, "Some items are missing in the work queue");
}

void System::ResetLabelBuffers() {
	
	// Get total count of output buffers in the indicator list
	VectorMap<unsigned, int> bufout_ids;
	int buf_id = 0;
	for (int i = 0; i < label_indi_ids.GetCount(); i++) {
		FactoryDeclaration& decl = label_indi_ids[i];
		const FactoryRegister& reg = GetRegs()[decl.factory];
		for (int j = decl.arg_count; j < reg.args.GetCount(); j++)
			decl.AddArg(reg.args[j].def);
		bufout_ids.Add(decl.GetHashValue(), buf_id);
		buf_id += reg.out[0].visible;
	}
	label_buf_count = buf_id;
	ASSERT(label_buf_count);
	
	
	// Reserve zeroed memory for output buffer pointer vector
	label_value_buffers.Clear();
	label_value_buffers.SetCount(sym_ids.GetCount());
	for (int i = 0; i < sym_ids.GetCount(); i++)
		label_value_buffers[i].SetCount(label_buf_count, NULL);


	// Get output buffer pointer vector
	int bars = GetCountTf(main_tf);
	int total_bufs = 0;
	data_begin = 0;

	for (int i = 0; i < label_queue.GetCount(); i++) {
		CoreItem& ci = *label_queue[i];
		ASSERT(!ci.core.IsEmpty());
		const Core& core = *ci.core;
		const Output& output = core.GetOutput(0);

		int sym_id = sym_ids.Find(ci.sym);

		if (sym_id == -1)
			continue;

		if (ci.tf != main_tf)
			continue;
		
		Vector<ConstVectorBool*>& indi_buffers = label_value_buffers[sym_id];
		const FactoryRegister& reg = GetRegs()[ci.factory];

		FactoryDeclaration decl;
		decl.Set(ci.factory);

		for (int j = 0; j < ci.args.GetCount(); j++)
			decl.AddArg(ci.args[j]);

		for (int j = decl.arg_count; j < reg.args.GetCount(); j++)
			decl.AddArg(reg.args[j].def);

		unsigned hash = decl.GetHashValue();


		// Check that args match to declaration
#ifdef flagDEBUG
		ArgChanger ac;
		ac.SetLoading();
		ci.core->IO(ac);
		ASSERT(ac.args.GetCount() >= ci.args.GetCount());
		for (int i = 0; i < ci.args.GetCount(); i++) {
			int a = ac.args[i];
			int b = ci.args[i];
			if (a != b) {
				LOG(Format("%d != %d", a, b));
			}
			ASSERT(a == b);
		}
#endif

		int buf_begin_id = bufout_ids.Find(hash);
		if (buf_begin_id == -1)
			continue;

		int buf_begin = bufout_ids[buf_begin_id];
		//LOG(i << ": " << ci.factory << ", " << sym_id << ", " << (int64)hash << ", " << buf_begin);
		for (int l = 0; l < reg.out[0].visible; l++) {
			int buf_pos = buf_begin + l;
			ConstVectorBool*& vecptr = indi_buffers[buf_pos];
			ASSERT_(vecptr == NULL, "Duplicate work item");
			vecptr = &output.label;
			total_bufs++;
		}
	}

	int expected_total = sym_ids.GetCount() * label_buf_count;

	ASSERT_(total_bufs == expected_total, "Some items are missing in the work queue");
}

void System::InitBrokerValues() {
	MetaTrader& mt = GetMetaTrader();

	proxy_id.SetCount(SYM_COUNT, 0);
	proxy_base_mul.SetCount(SYM_COUNT, 0);

	for (int i = 0; i < SYM_COUNT; i++) {
		int sym = sym_ids[i];

		DataBridge* db = dynamic_cast<DataBridge*>(databridge_cores[sym]);

		const Symbol& symbol = mt.GetSymbol(sym);

		if (symbol.proxy_id != -1) {
			int k = sym_ids.Find(symbol.proxy_id);
			ASSERT(k != -1);
			proxy_id[i] = k;
			proxy_base_mul[i] = symbol.base_mul;
		}

		else {
			proxy_id[i] = -1;
			proxy_base_mul[i] = 0;
		}
	}
}

void System::SetFixedBroker(FixedSimBroker& broker, int sym_id) {
	for (int i = 0; i < SYM_COUNT; i++) {
		broker.spread_points[i]		= spread_points[i];
		broker.proxy_id[i]			= proxy_id[i];
		broker.proxy_base_mul[i]	= proxy_base_mul[i];
	}

	broker.begin_equity				= 10000.0;
	broker.leverage					= 1000;
	broker.free_margin_level		= FMLEVEL;
	broker.part_sym_id				= sym_id;
	broker.init						= true;
}

}
