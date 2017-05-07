#include "Overlook.h"

Loader::Loader() {
	Title("Overlook Loader");
	Icon(OverlookImg::icon());
	
	CtrlLayout(*this);
	exit = true;
	autostart = false;
	
	list.AddColumn("Filename");
	
	// Author's note: I ain't got no time for allowing bad combinations... contributors can commit custom options.
	
	bardata.Add("DataBridge");
	bardata.Add("Dummy data");
	bardata.SetIndex(0);
	bardata.Disable();
	
	autoencoder.Set(true);
	autoencoder.Disable();
	rnn.Disable();
	lstm.Set(true);
	lstm.Disable();
	highway.Disable();
	narx.Set(true);
	narx.Disable();
	
	classagent.Set(true);
	classagent.Disable();
	rl1.Set(true);
	rl1.Disable();
	rl2.Set(true);
	rl2.Disable();
	mona.Set(true);
	mona.Disable();
	
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
	//tf7.Set(true); // too little data usually
	tf0.Disable();
	tf1.Disable();
	tf2.Disable();
	tf3.Disable();
	tf4.Disable();
	tf5.Disable();
	tf6.Disable();
	tf7.Disable();
	
	trade_real.Set(false);
	
	title.SetData("The Profile");
	
	license <<= THISBACK(ShowLicense);
	load <<= THISBACK(Load);
	create <<= THISBACK(Create);
	
	PostCallback(THISBACK(RefreshSessions));
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
	String search_str = AppendFileName(ConfigFile("profiles"), "*");
	ff.Search(search_str);
	do {
		if (ff.IsDirectory()) {
			String title = GetFileTitle(ff.GetPath());
			if (title.GetCount() >= 3)
				list.Add(title);
		}
	}
	while (ff.Next());
	
	if (autostart && list.GetCount()) {
		list.SetCursor(0);
		PostCallback(THISBACK(Load));
	}
}

void Loader::Create() {
	DataCore::Session& ses = GetSession();
	
	bool trade_real = this->trade_real;
	
	// Set settings
	ses.SetName(title.GetData());
	ses.SetBegin(begin.GetData());
	ses.SetEnd(end.GetData());
	ses.SetAddress(server.GetData());
	ses.SetPort(port.GetData());
	
	
	// Add DataBridge
	ses.LinkCore("/open", "/db?addr=\"" + (String)server.GetData() + "\"&port=" + IntStr((int)port.GetData()));
	
	
	// Add some always used utils
	ses.LinkCore("/ma", "/ma");
	ses.LinkCore("/spread", "/spread");
	ses.LinkCore("/ideal", "/ideal");
	ses.LinkCore("/change", "/change");
	ses.LinkCore("/whstat_fast", "/whstat?period=10");
	ses.LinkCore("/whstat_slow", "/whstat?period=20");
	ses.LinkCore("/whdiff", "/whdiff");
	ses.LinkCore("/chp", "/chp");
	ses.LinkCore("/eosc", "/eosc");
	
	
	// Add forecaster components (autoencoder and recurrent neural networks)
	int forecasters = 0;
	if (autoencoder.Get()) {
		ses.LinkCore("/aenc", "/aenc");
		forecasters++;
	}
	if (rnn.Get()) {
		ses.LinkCore("/rnn", "/rnn?type=\"rnn\"");
		forecasters++;
	}
	
	if (lstm.Get()) {
		ses.LinkCore("/lstm", "/rnn?type=\"lstm\"");
		forecasters++;
	}
	
	if (highway.Get()) {
		ses.LinkCore("/highway", "/rnn?type=\"highway\"");
		forecasters++;
	}
	
	if (narx.Get()) {
		ses.LinkCore("/narx", "/narx");
		forecasters++;
	}
	
	
	// Add forecaster
	ASSERT(forecasters > 0);
	ses.LinkCore("/forecaster", "/forecaster");
	
	
	// Add agents
	int agents = 0;
	if (classagent.Get()) {
		ses.LinkCore("/classagent", "/classagent");
		agents++;
	}
	if (rl1.Get()) {
		ses.LinkCore("/rl", "/rl");
		agents++;
	}
	
	if (rl2.Get()) {
		ses.LinkCore("/dqn", "/dqn");
		agents++;
	}
	
	if (mona.Get()) {
		ses.LinkCore("/mona", "/mona");
		agents++;
	}
	
	// Add meta agent (multi-agent, multi-tf)
	ASSERT(agents > 0);
	ses.LinkCore("/metamona", "/metamona");
	
	
	// Add multi agents
	int multiagents = 0;
	if (rl2multi.Get()) {
		ses.LinkCore("/doubledqn", "/doubledqn");
		multiagents++;
	}
	
	if (monamulti.Get()) {
		ses.LinkCore("/doublemona", "/doublemona");
		multiagents++;
	}
	
	ASSERT(multiagents > 0);
	
	// Add Timeframes
	if (tf0.Get()) ses.AddTimeframe(1);
	if (tf1.Get()) ses.AddTimeframe(5);
	if (tf2.Get()) ses.AddTimeframe(15);
	if (tf3.Get()) ses.AddTimeframe(30);
	if (tf4.Get()) ses.AddTimeframe(60);
	if (tf5.Get()) ses.AddTimeframe(240);
	if (tf6.Get()) ses.AddTimeframe(1440);
	if (tf7.Get()) ses.AddTimeframe(10080);
	
	
	// Add Ctrl objects
	ses.LinkCtrl("/broker", "/brokerctrl?broker=\"/open\"");
	ses.LinkCtrl("/notification", "/notification");
	ses.LinkCtrl("/analyzerctrl", "/analyzerctrl");
	
	
	// Init
	ses.Init();
	
	exit = false;
	Close();
}


void Loader::Load() {
	int cursor = list.GetCursor();
	if (cursor == -1) return;
	
	DataCore::Session& ses = GetSession();
	ses.SetName(list.Get(cursor, 0));
	ses.LoadThis();
	
	exit = false;
	Close();
}
