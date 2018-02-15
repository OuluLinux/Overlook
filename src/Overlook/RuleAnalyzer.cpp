#include "Overlook.h"
#if 0

namespace Overlook {




RuleAnalyzer::RuleAnalyzer() {
	data_ctrl.ra = this;
	agentctrl.ra = this;
	data_slider << THISBACK(SetCursor);
	data_slider.MinMax(0, 1);
	
	CtrlLayout(*this, "Rule Analyzer");
	prog.Set(0, 1);
	analyze << THISBACK(PostStartProcess);
	LoadThis();
	
	
	data_tab.Add(data_ctrl.HSizePos(0,30).VSizePos());
	data_tab.Add(data_slider.BottomPos(0,30).HSizePos());
	agent_tab.Add(agentctrl.SizePos());
	
	enable.Set(is_enabled);
	enable.WhenAction << THISBACK(SetEnable);
	
	Data();
}

RuleAnalyzer::~RuleAnalyzer() {
	StopProcess();
	StoreThis();
}

void RuleAnalyzer::Refresh() {
	System& sys				= GetSystem();
	if (phase < FINISHED)
		return;
	for(data_cursor = 0; data_cursor < sys.GetSymbolCount(); data_cursor++) {
		sys.System::Process(*ci_queue[data_cursor], true);
		ProcessData();
		ProcessSignalPrediction();
	}
	if (is_enabled)
		RefreshReal();
}

void RuleAnalyzer::Data() {
	if (this->data_in.IsEmpty()) return;
	
	int begin_id = 0;
	
	auto& data_in = this->data_in[0];
	int data_count = data_in.GetCount() / row_size;
	if (data_slider.GetMax() != data_count-1) {
		data_slider.MinMax(0, data_count-1);
		data_slider.SetData(data_count-1);
	}
	
	int seconds = total / prev_speed / GetUsedCpuCores();
	if (perc < 100) {
		int hours = seconds / 3600;		seconds = seconds % 3600;
		int minutes = seconds / 60;		seconds = seconds % 60;
		Title(Format("Rule Analyzer :: %d%% :: Time remaining %d hours %d minutes %d seconds", perc, hours, minutes, seconds));
		prog.Set(actual, total);
	}
	else {
		Title("Rule Analyzer :: Ready");
		prog.Set(0, 1);
	}
	
	agentctrl.Refresh();
}
}



#endif
