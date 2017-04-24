#include "Overlook.h"

Loader::Loader() {
	Title("Overlook Loader");
	Icon(OverlookImg::icon());
	
	CtrlLayout(*this);
	exit = true;
	
	list.AddColumn("Filename");
	
	// Author's note: I ain't got no time for allowing bad combinations... contributors can commit custom options.
	
	bardata.Add("DataBridge");
	bardata.Add("Dummy data");
	bardata.SetIndex(0);
	bardata.Disable();
	
	rnn.Disable();
	lstm.Set(true);
	lstm.Disable();
	highway.Disable();
	narx.Set(true);
	narx.Disable();
	
	rl1.Set(true);
	rl1.Disable();
	rl2.Set(true);
	rl2.Disable();
	mona.Set(true);
	mona.Disable();
	
	rl2multi.Set(true);
	rl2multi.Disable();
	monamulti.Set(true);
	monamulti.Disable();
	
	server.SetData("192.168.0.5");
	port.SetData(42000);
	
	begin.SetData(Time(2017, 1, 1));
	Date now = GetSysTime();
	do {++now;}
	while (now.day != 15);
	end.SetData(Time(now.year, now.month, now.day));
	begin.Disable();
	end.Disable();
	
	tf2.Set(true);
	tf3.Set(true);
	tf4.Set(true);
	tf5.Set(true);
	tf6.Set(true);
	tf7.Set(true);
	tf0.Disable();
	tf1.Disable();
	tf2.Disable();
	tf3.Disable();
	tf4.Disable();
	tf5.Disable();
	tf6.Disable();
	tf7.Disable();
	
	trade_real.Set(true);
	trade_real.Disable();
	
	title.SetData("The Profile");
	
	license <<= THISBACK(ShowLicense);
	load <<= THISBACK(Load);
	create <<= THISBACK(Create);
	
	RefreshSessions();
}

void Loader::ShowLicense() {
	TopWindow tw;
	tw.Title("Overlook License");
	
	DocEdit doc;
	doc.SetData(
		"Copyright (c) 2017 Seppo Pakonen. All rights reserved." "\n"
		"\n"
		"Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:" "\n"
		"" "\n"
		"1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer." "\n"
		"" "\n"
		"2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the " "\n"
		"distribution." "\n"
		"" "\n"
		"3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission." "\n"
		"" "\n"
		"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR "
		"A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT "
		"NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, "
		"OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
	);
	tw.Add(doc.SizePos());
	tw.SetRect(0,0,640,480);
	tw.Run();
}

void Loader::RefreshSessions() {
	FindFile ff;
	String search_str = ConfigFile("*.ol");
	ff.Search(search_str);
	do {
		String title = GetFileTitle(ff.GetPath());
		list.Add(title);
	}
	while (ff.Next());
}

void Loader::Create() {
	
	OverlookSession ses;
	
	ses.begin = begin.GetData();
	ses.end = end.GetData();
	
	ses.addr = server.GetData();
	ses.port = port.GetData();
	
	// Add DataBridge
	ses.link_core.Add("/open", "/db?addr=\"" + (String)server.GetData() + "\"&port=" + IntStr((int)port.GetData()));
	
	
	// Add some always used utils
	ses.link_core.Add("/ma", "/ma");
	ses.link_core.Add("/spread", "/spread");
	ses.link_core.Add("/change", "/change");
	ses.link_core.Add("/whch", "/whch");
	ses.link_core.Add("/whstat", "/whstat");
	ses.link_core.Add("/whd", "/whd");
	ses.link_core.Add("/chp", "/chp");
	ses.link_core.Add("/eosc", "/eosc");
	
	// Add recurrent neural networks
	int recurrents = 0;
	if (rnn.Get()) {
		ses.link_core.Add("/rnn", "/rnn?type=\"rnn\"");
		recurrents++;
	}
	
	if (lstm.Get()) {
		ses.link_core.Add("/lstm", "/rnn?type=\"lstm\"");
		recurrents++;
	}
	
	if (highway.Get()) {
		ses.link_core.Add("/highway", "/rnn?type=\"highway\"");
		recurrents++;
	}
	
	if (narx.Get()) {
		ses.link_core.Add("/narx", "/narx");
		recurrents++;
	}
	
	
	// Add forecaster
	ASSERT(recurrents > 0);
	ses.link_core.Add("/forecaster", "/forecaster");
	
	
	// Add agents
	int agents = 0;
	if (rl1.Get()) {
		ses.link_core.Add("/rl", "/rl");
		agents++;
	}
	
	if (rl2.Get()) {
		ses.link_core.Add("/dqn", "/dqn");
		agents++;
	}
	
	if (mona.Get()) {
		ses.link_core.Add("/mona", "/mona");
		agents++;
	}
	
	// Add meta agent (multi-agent, multi-tf)
	ASSERT(agents > 0);
	ses.link_core.Add("/metamona", "/metamona");
	
	
	// Add multi agents
	int multiagents = 0;
	if (rl2multi.Get()) {
		ses.link_core.Add("/doubledqn", "/doubledqn");
		multiagents++;
	}
	
	if (monamulti.Get()) {
		ses.link_core.Add("/doublemona", "/doublemona");
		multiagents++;
	}
	
	ASSERT(multiagents > 0);
	
	// Add Timeframes
	if (tf0.Get()) ses.tfs.Add(1);
	if (tf1.Get()) ses.tfs.Add(5);
	if (tf2.Get()) ses.tfs.Add(15);
	if (tf3.Get()) ses.tfs.Add(30);
	if (tf4.Get()) ses.tfs.Add(60);
	if (tf5.Get()) ses.tfs.Add(240);
	if (tf6.Get()) ses.tfs.Add(1440);
	if (tf7.Get()) ses.tfs.Add(10080);
	
	
	
	// Add Ctrl objects
	ses.link_ctrl.Add("/broker", "/brokerctrl?broker=\"/open\"");
	ses.link_ctrl.Add("/notification", "/notification");
	
	
	String filename = (String)title.GetData() + ".ol";
	filename.Replace(" ", "_");
	ses.datadir = filename + ".d";
	
	FileOut out(ConfigFile(filename));
	out % ses;
	
	String dir = ConfigFile(ses.datadir);
	if (DirectoryExists(dir))
		DeleteFolderDeep(dir);
	RealizeDirectory(dir);
	
	LoadSession(ses);
}


void Loader::Load() {
	int cursor = list.GetCursor();
	if (cursor == -1) return;
	String filename = (String)list.Get(cursor, 0) + ".ol";
	String path = ConfigFile(filename);
	if (!FileExists(path))
		return;
	
	OverlookSession ses;
	FileIn in(path);
	in % ses;
	LoadSession(ses);
}

void Loader::LoadSession(OverlookSession& ses) {
	TimeVector& tv = GetTimeVector();
	PathResolver& res = *GetPathResolver();
	
	try {
		tv.EnableCache();
		tv.LimitMemory();
		
		// Init sym/tfs/time space
		MetaTrader mt;
		mt.Init(ses.addr, ses.port);
		
		// Add symbols
		Vector<Symbol> symbols;
		symbols <<= mt.GetSymbols();
		ASSERTEXC(!symbols.IsEmpty());
		VectorMap<String, int> currencies;
		for(int i = 0; i < symbols.GetCount(); i++) {
			Symbol& s = symbols[i];
			tv.AddSymbol(s.name);
			if (s.IsForex() && s.name.GetCount() == 6) {
				String a = s.name.Left(3);
				String b = s.name.Right(3);
				currencies.GetAdd(a, 0)++;
				currencies.GetAdd(b, 0)++;
			}
		}
		SortByValue(currencies, StdGreater<int>()); // sort by must pairs having currency
		for(int i = 0; i < currencies.GetCount(); i++) {
			const String& cur = currencies.GetKey(i);
			tv.AddSymbol(cur);
		}
		
		// TODO: store symbols to session file and check that mt supports them
		
		// Add periods
		ASSERT(mt.GetTimeframe(0) == 1);
		int base = 15; // mins
		tv.SetBasePeriod(60*base);
		Vector<int> tfs;
		for(int i = 0; i < mt.GetTimeframeCount(); i++) {
			int tf = mt.GetTimeframe(i);
			if (tf >= base) {
				tfs.Add(tf / base);
				tv.AddPeriod(tf * 60);
			}
		}
		
		
		// Init time range
		tv.SetBegin(ses.begin);
		tv.SetEnd(ses.end);
		MetaTime& mtime = res.GetTime();
		mtime.SetBegin(tv.GetBegin());
		mtime.SetEnd(tv.GetEnd());
		mtime.SetBasePeriod(tv.GetBasePeriod());
		
		
		
		// Link core
		Index<String> linkctrl_symtf;
		VectorMap<String, String> linkcustomctrl_symtf, linkcustomctrl_sym, linkcustomctrl_tf;
		for(int i = 0; i < ses.link_core.GetCount(); i++) {
			const String& key = ses.link_core.GetKey(i);
			String value = ses.link_core[i];
			tv.LinkPath(key, value);
			
			// Link ctrl
			SlotPtr slot = tv.FindLinkSlot(key);
			String ctrlkey = slot->GetCtrl();
			
			// By default, slotctrl is Container and it is added separately for every symbol
			// and timeframe.
			if (ctrlkey == "default") {
				linkctrl_symtf.Add(key);
			}
			// Otherwise, link custom ctrl to /slotctrl/ folder.
			else {
				int type = slot->GetCtrlType();
				String ctrl_dest = "/slotctrl/" + ctrlkey;
				String ctrl_src = "/" + ctrlkey + "?slot=\"" + key + "\"";
				if (type == SLOT_SYMTF) {
					linkcustomctrl_symtf.Add(ctrl_dest, ctrl_src);
				}
				else if (type == SLOT_SYM) {
					linkcustomctrl_sym.Add(ctrl_dest, ctrl_src);
				}
				else if (type == SLOT_TF) {
					linkcustomctrl_tf.Add(ctrl_dest, ctrl_src);
				}
				else if (type == SLOT_ONCE) {
					res.LinkPath(ctrl_dest, ctrl_src);
				}
				else Panic("Unknown slottype");
			}
		}
		
		
		// Link ctrls
		for(int i = 0; i < ses.link_ctrl.GetCount(); i++) {
			const String& key = ses.link_ctrl.GetKey(i);
			String value = ses.link_ctrl[i];
			res.LinkPath(key, value);
		}
		
		
		// Link symbols and timeframes
		for(int i = 0; i < symbols.GetCount(); i++) {
			for(int j = 0; j < tfs.GetCount(); j++) {
				
				// Add frontpage
				String fp_dest = "/name/" + symbols[i].name;
				String fp_src = "/fp?id=" + IntStr(i);
				res.LinkPath(fp_dest, fp_src);
				
				
				// Add by name
				String dest = "/name/" + symbols[i].name + "/tf" + IntStr(tfs[j]);
				String src = "/bardata?bar=\"/open\"&id=" + IntStr(i) + "&period=" + IntStr(tfs[j]);
				dest.Replace("#", "");
				res.LinkPath(dest, src);
				
				
				// Add default sym/tf ctrls, by name
				for(int k = 0; k < linkctrl_symtf.GetCount(); k++) {
					const String& key = linkctrl_symtf[k];
					String ctrl_dest = "/slotctrl" + key + "/" + symbols[i].name + "/tf" + IntStr(tfs[j]);
					ctrl_dest.Replace("#", "");
					String ctrl_src = dest + "/cont?slot=\"" + key + "\"&id=" + IntStr(i) + "&tf_id=" + IntStr(j);
					res.LinkPath(ctrl_dest, ctrl_src);
				}
				
				// Add custom sym/tf ctrls
				for(int k = 0; k < linkcustomctrl_symtf.GetCount(); k++) {
					const String& key = linkcustomctrl_symtf.GetKey(k);
					String value = linkcustomctrl_symtf[k];
					String ctrl_dest = key + "/" + symbols[i].name + "/tf" + IntStr(tfs[j]);
					ctrl_dest.Replace("#", "");
					String ctrl_src = value + "&id=" + IntStr(i) + "&tf_id=" + IntStr(j);
					res.LinkPath(ctrl_dest, ctrl_src);
				}
				
				// Add custom tf ctrls
				if (i == 0) {
					for(int k = 0; k < linkcustomctrl_tf.GetCount(); k++) {
						const String& key = linkcustomctrl_tf.GetKey(k);
						String value = linkcustomctrl_tf[k];
						String ctrl_dest = key + "/tf" + IntStr(tfs[j]);
						ctrl_dest.Replace("#", "");
						String ctrl_src = value + "&tf_id=" + IntStr(j);
						res.LinkPath(ctrl_dest, ctrl_src);
					}
				}
			}
			
			// Add custom sym ctrls
			for(int k = 0; k < linkcustomctrl_sym.GetCount(); k++) {
				const String& key = linkcustomctrl_sym.GetKey(k);
				String value = linkcustomctrl_sym[k];
				String ctrl_dest = key + "/" + symbols[i].name;
				ctrl_dest.Replace("#", "");
				String ctrl_src = value + "&id=" + IntStr(i);
				res.LinkPath(ctrl_dest, ctrl_src);
			}
		}
		
		
		// Link parameter configuration ctrls for all slots
		for(int i = 0; i < tv.GetCustomSlotCount(); i++) {
			const Slot& slot = tv.GetCustomSlot(i);
			String linkpath = slot.GetLinkPath();
			ASSERT(!linkpath.IsEmpty()); // sanity check
			String ctrl_dest = "/params" + linkpath;
			String ctrl_src = "/paramctrl?slot=\"" + linkpath + "\"";
			res.LinkPath(ctrl_dest, ctrl_src);
		}
		
		
		// Create or load cache base on linked slots (keep this 2. last)
		tv.RefreshData();
		
		// Link runner (also starts processing, so keep this last)
		res.LinkPath("/runner", "/runnerctrl");
		
		
		exit = false;
	}
	
	catch (...) {
		PromptOK("Load failed");
	}
	
	Close();
}
