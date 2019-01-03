#include "Overlook.h"

namespace Overlook {

void InitSessionDefault(ConvNet::Session& ses, int input_depth, int output_count) {
	String t = "[\n"
		"\t{\"type\":\"input\", \"input_width\":1, \"input_height\":1, \"input_depth\":" + IntStr(input_depth) + "},\n"
		"\t{\"type\":\"fc\", \"neuron_count\": 50, \"activation\":\"relu\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\": 50, \"activation\":\"relu\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\": 50, \"activation\":\"relu\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\": 50, \"activation\":\"relu\"},\n"
		"\t{\"type\":\"regression\", \"neuron_count\":" + IntStr(output_count) + "},\n"
		"\t{\"type\":\"sgd\", \"learning_rate\":0.001, \"momentum\":0.0, \"batch_size\":64, \"l2_decay\":0.01}\n"
		"]\n";
		
	if (ses.GetStepCount() == 0)
		ses.MakeLayers(t);
	
}

void LoadSymbol(CoreList& cl_sym, int symbol, int tf) {
	System& sys = GetSystem();
	
	System::NetSetting& net = sys.GetNet(0);
	cl_sym.AddSymbol(sys.GetSymbol(symbol));
	cl_sym.AddTf(tf);
	cl_sym.AddIndi(0);
	cl_sym.Init();
	cl_sym.Refresh();
}

void LoadNetSymbols(CoreList& cl_sym, int tf) {
	System& sys = GetSystem();
	
	System::NetSetting& net = sys.GetNet(0);
	for(int i = 0; i < net.symbols.GetCount(); i++) {
		cl_sym.AddSymbol(net.symbols.GetKey(i));
	}
	cl_sym.AddTf(tf);
	cl_sym.AddIndi(0);
	cl_sym.Init();
	cl_sym.Refresh();
}


void LoadNets(CoreList& cl_net, int tf) {
	System& sys = GetSystem();
	
	for(int i = 0; i < sys.GetNetCount(); i++) {
		cl_net.AddSymbol("Net" + IntStr(i));
	}
	cl_net.AddTf(tf);
	cl_net.AddIndi(0);
	cl_net.Init();
	cl_net.Refresh();
}

void LoadDataPriceInput(ConvNet::Session& ses, CoreList& cl_net, int begin, int count, int windowsize) {
	ConvNet::SessionData& d = ses.GetData();
	
	for(int i = 0; i < count; i++) {
		int pos = begin + i;
		
		int col = 0;
		for(int j = 0; j < cl_net.GetSymbolCount(); j++) {
			ConstBuffer& buf = cl_net.GetBuffer(j, 0, 0);
			double next = buf.Get(pos);
			for(int j = 0; j < windowsize; j++) {
				int pos2 = max(0, pos - j - 1);
				double cur = buf.Get(pos2);
				double ch = ((next / cur) - 1.0) * 1000;
				d.SetData(i, col++, ch);
				next = cur;
			}
		}
	}
}

void LoadVolumePriceInput(CoreList& cl_net, int pos, ConvNet::Volume& in, int windowsize) {
	int col = 0;
	for(int j = 0; j < cl_net.GetSymbolCount(); j++) {
		ConstBuffer& buf = cl_net.GetBuffer(j, 0, 0);
		double next = buf.Get(pos);
		for(int j = 0; j < windowsize; j++) {
			int pos2 = max(0, pos - j - 1);
			double cur = buf.Get(pos2);
			double ch = ((next / cur) - 1.0) * 1000;
			in.Set(col++, ch);
			next = cur;
		}
	}
}

void LoadDataPipOutput(ConvNet::Session& ses, CoreList& cl_net, int begin, int count, int postpips_count) {
	ConvNet::SessionData& d = ses.GetData();
	
	for(int i = 0; i < count; i++) {
		int pos = begin + i;
		
		for(int j = 0; j < cl_net.GetSymbolCount(); j++) {
			ConstBuffer& buf = cl_net.GetBuffer(j, 0, 0);
			ConstBuffer& lowbuf = cl_net.GetBuffer(j, 0, 1);
			ConstBuffer& highbuf = cl_net.GetBuffer(j, 0, 2);
			double point = cl_net.GetDataBridge(j)->GetPoint();
			double open = buf.Get(pos);
			double lo = open - postpips_count * point;
			double hi = open + postpips_count * point;
			
			bool result = false;
			
			for(int k = pos+1; k < buf.GetCount(); k++) {
				double low  = lowbuf.Get(k-1);
				double high = highbuf.Get(k-1);
				if (low <= lo) {
					result = true;
					break;
				}
				else if (high >= hi) {
					result = false;
					break;
				}
			}
			
			d.SetResult(i, j, result ? -1.0 : +1.0);
		}
	}
}

void LoadDataVolatOutput(ConvNet::Session& ses, CoreList& cl_net, int begin, int count, int ticks) {
	ConvNet::SessionData& d = ses.GetData();
	
	for(int i = 0; i < count; i++) {
		int pos = begin + i;
		
		for(int j = 0; j < cl_net.GetSymbolCount(); j++) {
			ConstBuffer& buf = cl_net.GetBuffer(j, 0, 0);
			
			double open = buf.Get(pos);
			double close = buf.Get(min(buf.GetCount()-1, pos + ticks));
			double ch = ((close / open) - 1.0) * 1000;
			ch = fabs(ch);
			
			d.SetResult(i, j, ch);
		}
	}
}

void TrainSession(ConvNet::Session& ses, int iterations, int& actual) {
	actual = ses.GetStepCount();
	if (actual < iterations) {
		ses.StartTraining();
		actual = ses.GetStepCount();
		while (actual < iterations && ses.IsTraining()) {
			Sleep(100);
			actual = ses.GetStepCount();
		}
		ses.StopTraining();
	}
}

String TestPriceInPipOut(ConvNet::Session& ses, CoreList& cl_net, int begin, int count, int windowsize, int postpips_count) {
	ConvNet::Net& net = ses.GetNetwork();
	ConvNet::Volume in;
	in.Init(1, 1, cl_net.GetSymbolCount() * windowsize);
	
	double sum = 0.0;
	Vector<double> sums;
	sums.Reserve(count);
	
	for(int i = 0; i < count; i++) {
		int pos = begin + i;
		
		LoadVolumePriceInput(cl_net, pos, in, windowsize);
		
		ConvNet::Volume& out = net.Forward(in);
		
		for(int j = 0; j < cl_net.GetSymbolCount(); j++) {
			ConstBuffer& buf = cl_net.GetBuffer(j, 0, 0);
			ConstBuffer& lowbuf = cl_net.GetBuffer(j, 0, 1);
			ConstBuffer& highbuf = cl_net.GetBuffer(j, 0, 2);
			double point = cl_net.GetDataBridge(j)->GetPoint();
			double open = buf.Get(pos);
			double lo = open - postpips_count * point;
			double hi = open + postpips_count * point;
			
			bool result = false;
			
			for(int k = pos+1; k < buf.GetCount(); k++) {
				double low  = lowbuf.Get(k-1);
				double high = highbuf.Get(k-1);
				if (low <= lo) {
					result = true;
					break;
				}
				else if (high >= hi) {
					result = false;
					break;
				}
			}
			
			double pred = out.Get(j);
			bool pred_result = pred <= 0.0;
			double ch = (result == pred_result ? +1 : -1);
			sum += ch;
		}
		
		if (i % 100 == 0)
			sums.Add(sum);
	}
	
	
	Size sz(800, 100);
	sz *= 4;
	DrawingDraw dw(sz);
	
	Vector<Point> cache;
	DrawVectorPolyline(dw, sz, sums, cache);
	
	static Array<QtfRichObject> objcache;
	QtfRichObject* pict = new QtfRichObject(CreateDrawingObject(dw.GetResult(), sz, sz));
	objcache.Add(pict);
	String qtf;
	qtf << *pict << DeQtf("\n");
	
	return qtf;
}

String TestPriceInVolatOut(ConvNet::Session& ses, CoreList& cl_net, int begin, int count, int windowsize, int ticks) {
	ConvNet::Net& net = ses.GetNetwork();
	ConvNet::Volume in;
	in.Init(1, 1, cl_net.GetSymbolCount() * windowsize);
	
	double sum = 0.0;
	Vector<double> sums;
	sums.Reserve(count);
	
	OnlineAverage1 av;
	for(int i = 0; i < count; i++) {
		int pos = begin + i;
		
		int col = 0;
		for(int j = 0; j < cl_net.GetSymbolCount(); j++) {
			ConstBuffer& buf = cl_net.GetBuffer(j, 0, 0);
			double next = buf.Get(pos);
			for(int j = 0; j < windowsize; j++) {
				int pos2 = max(0, pos - j - 1);
				double cur = buf.Get(pos2);
				double ch = ((next / cur) - 1.0) * 1000;
				in.Set(col++, ch);
				next = cur;
			}
		}
		
		ConvNet::Volume& out = net.Forward(in);
		
		for(int j = 0; j < cl_net.GetSymbolCount(); j++) {
			ConstBuffer& buf = cl_net.GetBuffer(j, 0, 0);
			double point = cl_net.GetDataBridge(j)->GetPoint();
			double open = buf.Get(pos);
			double close = buf.Get(min(buf.GetCount()-1, pos + ticks));
			double ch = ((close / open) - 1.0) * 1000;
			ch = fabs(ch);
			
			double pred = out.Get(j);
			av.Add(ch);
			
			double mean = av.GetMean();
			bool dir = mean < ch;
			bool pred_dir = mean < pred;
			
			double sumch = (pred_dir == dir ? +1 : -1) * fabs(mean - ch);
			sum += sumch;
		}
		
		if (i % 100 == 0)
			sums.Add(sum);
	}
	
	
	Size sz(800, 100);
	sz *= 4;
	DrawingDraw dw(sz);
	
	Vector<Point> cache;
	DrawVectorPolyline(dw, sz, sums, cache);
	
	static Array<QtfRichObject> objcache;
	QtfRichObject* pict = new QtfRichObject(CreateDrawingObject(dw.GetResult(), sz, sz));
	objcache.Add(pict);
	String qtf;
	qtf << *pict << DeQtf("\n");
	
	return qtf;
}

String TestTrade(int symbol, int postpips_count, LabelSignal& signal) {
	System& sys = GetSystem();
	CoreList cl;
	cl.AddSymbol(sys.GetSymbol(symbol));
	cl.AddTf(ScriptCore::fast_tf);
	cl.AddIndi(0);
	cl.Init();
	cl.Refresh();
	
	ConstBuffer& open_buf = cl.GetBuffer(0, 0, 0);
	ConstBuffer& low_buf = cl.GetBuffer(0, 0, 1);
	ConstBuffer& high_buf = cl.GetBuffer(0, 0, 2);
	double point = cl.GetDataBridge(0)->GetPoint();
	int spread_i = CommonSpreads().Find(sys.GetSymbol(symbol));
	int spread = spread_i != -1 ? CommonSpreads()[spread_i] : 6;
	
	Vector<double> sums;
	double sum = 0.0;
	
	OnlineAverage1 withoutspread, withspread, consecutivewins, consecutivelosses;
	int consecutivenow = 0;
	bool prev_succ = false;
	
	int count = min(open_buf.GetCount(), signal.signal.GetCount());
	for(int pos = 0; pos < count; pos++) {
		
		if (signal.enabled.Get(pos)) {
			bool sig = signal.signal.Get(pos);
			
			double open = open_buf.Get(pos);
			double lo = open - postpips_count * point;
			double hi = open + postpips_count * point;
			
			bool result = false;
			
			for(int k = pos+1; k < open_buf.GetCount(); k++) {
				double low  = low_buf.Get(k-1);
				double high = high_buf.Get(k-1);
				if (low <= lo) {
					result = true;
					break;
				}
				else if (high >= hi) {
					result = false;
					break;
				}
			}
			
			bool succ = result == sig;
			
			if (succ == prev_succ) {
				consecutivenow++;
			} else {
				if (prev_succ)
					consecutivewins.Add(consecutivenow);
				else
					consecutivelosses.Add(consecutivenow);
				consecutivenow = 0;
			}
			prev_succ = succ;
			
			withoutspread.Add(succ ? +postpips_count : -postpips_count);
			double ch = (succ ? +postpips_count - spread : -postpips_count - spread);
			withspread.Add(ch);
			sum += ch;
		}
		
		if (pos % 100 == 0 && sum != 0.0)
			sums.Add(sum);
	}
	
	
	Size sz(800, 100);
	sz *= 4;
	DrawingDraw dw(sz);
	
	Vector<Point> cache;
	DrawVectorPolyline(dw, sz, sums, cache);
	
	static Array<QtfRichObject> objcache;
	QtfRichObject* pict = new QtfRichObject(CreateDrawingObject(dw.GetResult(), sz, sz));
	objcache.Add(pict);
	String qtf;
	qtf << DeQtf("spread=" + IntStr(spread) + "\n");
	qtf << DeQtf("average without spread=" + DblStr(withoutspread.GetMean()) + "\n");
	qtf << DeQtf("average with spread=" + DblStr(withspread.GetMean()) + "\n");
	qtf << DeQtf("average consecutive wins=" + DblStr(consecutivewins.GetMean()) + "\n");
	qtf << DeQtf("average consecutive losses=" + DblStr(consecutivelosses.GetMean()) + "\n");
	qtf << *pict << DeQtf("\n");
	
	return qtf;
}















void ScriptList::Init() {
	System& sys = GetSystem();
	ScriptCommon& sc = GetScriptCommon();
	
	script_queue.Clear();
	
	for(int i = 0; i < script_ids.GetCount(); i++) {
		Vector<Ptr<ScriptCoreItem> > queue;
		sys.GetScriptCoreQueue(queue, script_ids[i]);
		ScriptCore& a = *queue[0]->core;
		bool exists = false;
		for(int j = 0; j < sc.process_queue.GetCount(); j++) {
			ScriptCore& b = *sc.process_queue[j]->core;
			if (&a == &b) {
				exists = true;
				break;
			}
		}
		if (!exists) {
			sc.process_queue.Append(queue);
		}
		script_queue.Append(queue);
	}
	
	
}







ScriptCommon::ScriptCommon() {
	
}

void ScriptCommon::Init() {
	
	Thread::Start(THISBACK(Process));
}

void ScriptCommon::Start() {
	
}

void ScriptCommon::Deinit() {
	
}

void ScriptCommon::Process() {
	sig.Start();
	
	while (sig.IsRunning()) {
		
		if (queue_cursor < process_queue.GetCount()) {
			ScriptCore& core = *process_queue[queue_cursor]->core;
			core.Run();
			core.Store();
			queue_cursor++;
		}
		
		Sleep(100);
	}
	
	sig.SetStopped();
}


ScriptCtrl::ScriptCtrl() {
	Add(hsplit.SizePos());
	hsplit.Horz();
	hsplit << scriptlist << result;
	hsplit.SetPos(2500);
	
	scriptlist.AddColumn("Title");
	scriptlist.AddColumn("Args");
	scriptlist.AddColumn("Progress");
	scriptlist <<= THISBACK(Data);
}

void ScriptCtrl::Data() {
	ScriptCommon& sc = GetScriptCommon();
	
	for(int i = 0; i < sc.process_queue.GetCount(); i++) {
		ScriptCore& core = *sc.process_queue[i]->core;
		scriptlist.Set(i, 0, core.GetTitle());
		
		ArgScript args;
		core.Arg(args);
		String s;
		for(int j = 0; j < args.titles.GetCount(); j++) {
			if (j) s << ", ";
			s << args.titles[j] << "=" << *args.args[j];
		}
		scriptlist.Set(i, 1, s);
		
		int prog = core.GetActual() * 1000 / core.GetTotal();
		scriptlist.Set(i, 2, prog);
		scriptlist.SetDisplay(i, 2, ProgressDisplay());
	}
	
	if (scriptlist.IsCursor()) {
		ScriptCore& core = *sc.process_queue[scriptlist.GetCursor()]->core;
		String result = core.GetTestResultQTF();
		if (result != prev_result) {
			this->result.SetQTF(result);
			prev_result = result;
		}
	}
}


}
