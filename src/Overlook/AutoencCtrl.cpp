#include "Overlook.h"

namespace Overlook {

AutoencCtrl::AutoencCtrl() {
	Add(hsplit.SizePos());
	hsplit.Horz();
	hsplit << tasksplit << leftsplit << rightsplit;
	hsplit.SetPos(1500, 0);
	hsplit.SetPos(1500+2500, 1);
	
	tasksplit.Vert();
	tasksplit << threadlist << tasklist;
	tasksplit.SetPos(2500);
	
	leftsplit.Vert();
	leftsplit << settings << graph << status;
	
	rightsplit.Vert();
	rightsplit << aenc_view << layer_view;
	rightsplit.SetPos(6400);
	
	threadlist.AddColumn("Thread name");
	tasklist.AddColumn("Task name");
	tasklist.AddColumn("Level");
	tasklist.ColumnWidths("3 1");
	
	aenc_view.SetSession(ses);
	layer_view.HideGradients();
	
	lrate.SetLabel("Learning rate:");
	lmom.SetLabel("Momentum:");
	lbatch.SetLabel("Batch size:");
	ldecay.SetLabel("Weight decay:");
	apply.SetLabel("Apply");
	save_net.SetLabel("Save network");
	load_net.SetLabel("Load network");
	apply <<= THISBACK(ApplySettings);
	save_net <<= THISBACK(SaveFile);
	load_net <<= THISBACK(OpenFile);
	int row = 20;
	settings.Add(lrate.HSizePos(4,4).TopPos(0,row));
	settings.Add(rate.HSizePos(4,4).TopPos(1*row,row));
	settings.Add(lmom.HSizePos(4,4).TopPos(2*row,row));
	settings.Add(mom.HSizePos(4,4).TopPos(3*row,row));
	settings.Add(lbatch.HSizePos(4,4).TopPos(4*row,row));
	settings.Add(batch.HSizePos(4,4).TopPos(5*row,row));
	settings.Add(ldecay.HSizePos(4,4).TopPos(6*row,row));
	settings.Add(decay.HSizePos(4,4).TopPos(7*row,row));
	settings.Add(apply.HSizePos(4,4).TopPos(8*row,row));
	settings.Add(save_net.HSizePos(4,4).TopPos(9*row,row));
	settings.Add(load_net.HSizePos(4,4).TopPos(10*row,row));
	rate.SetData(0.01);
	mom.SetData(0.9);
	batch.SetData(20);
	decay.SetData(0.001);
	
	
	
	
	layer_view.SetSession(ses);
	
	graph.SetSession(ses);
	graph.SetModeLoss();
}

AutoencCtrl::~AutoencCtrl() {
	
	ses.StopTraining();
	
}

void AutoencCtrl::UpdateNetParamDisplay() {
	TrainerBase* t = ses.GetTrainer();
	if (!t) return;
	TrainerBase& trainer = *t;
	rate.SetData(trainer.GetLearningRate());
	mom.SetData(trainer.GetMomentum());
	batch.SetData(trainer.GetBatchSize());
	decay.SetData(trainer.GetL2Decay());
}

void AutoencCtrl::ApplySettings() {
	TrainerBase* t = ses.GetTrainer();
	if (!t) return;
	TrainerBase& trainer = *t;
	trainer.SetLearningRate(rate.GetData());
	trainer.SetMomentum(mom.GetData());
	trainer.SetBatchSize(batch.GetData());
	trainer.SetL2Decay(decay.GetData());
}

void AutoencCtrl::OpenFile() {
	String file = SelectFileOpen("JSON files\t*.json\nAll files\t*.*");
	if (file.IsEmpty()) return;
	
	if (!FileExists(file)) {
		PromptOK("File does not exists");
		return;
	}
	
	// Load json
	String json = LoadFile(file);
	if (json.IsEmpty()) {
		PromptOK("File is empty");
		return;
	}
	
	ses.StopTraining();
	
	ticking_lock.Enter();
	bool res = ses.LoadJSON(json);
	
	ticking_lock.Leave();
	
	if (!res) {
		PromptOK("Loading failed.");
		return;
	}
	
	ResetAll();
}

void AutoencCtrl::SaveFile() {
	String file = SelectFileSaveAs("JSON files\t*.json\nAll files\t*.*");
	if (file.IsEmpty()) return;
	
	// Save json
	String json;
	if (!ses.StoreJSON(json)) {
		PromptOK("Error: Getting JSON failed");
		return;
	}
	
	FileOut fout(file);
	if (!fout.IsOpen()) {
		PromptOK("Error: could not open file " + file);
		return;
	}
	fout << json;
}

void AutoencCtrl::Reload() {
	ses.StopTraining();
	
	String net_str =
		"[\n"
		"\t{\"type\":\"input\", \"input_width\":28, \"input_height\":28, \"input_depth\":1},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":50, \"activation\": \"tanh\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":50, \"activation\": \"tanh\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":2},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":50, \"activation\": \"tanh\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":50, \"activation\": \"tanh\"},\n"
		"\t{\"type\":\"regression\", \"neuron_count\":784},\n" // 24*24=576, 28*28=784
		"\t{\"type\":\"adadelta\", \"learning_rate\":1, \"batch_size\":50, \"l1_decay\":0.001, \"l2_decay\":0.001}\n"
		"]\n";
	
	ticking_lock.Enter();
	
	bool success = ses.MakeLayers(net_str);
	
	ticking_lock.Leave();
	
	ResetAll();
	layer_view.Layout();
	
	if (success) {
		ses.StartTraining();
	}
}

void AutoencCtrl::RefreshStatus() {
	String s;
	s << "   Forward time per example: " << ses.GetForwardTime() << "\n";
	s << "   Backprop time per example: " << ses.GetBackwardTime() << "\n";
	s << "   Regression loss: " << ses.GetLossAverage() << "\n";
	s << "   L2 Weight decay loss: " << ses.GetL2DecayLossAverage() << "\n";
	s << "   L1 Weight decay loss: " << ses.GetL1DecayLossAverage() << "\n";
	s << "   Examples seen: " << ses.GetStepCount();
	status.SetLabel(s);
}

void AutoencCtrl::Refresher() {
	layer_view.Refresh();
	aenc_view.Refresh();
		
	graph.RefreshData();
	RefreshStatus();
	
	PostCallback(THISBACK(Refresher));
}

void AutoencCtrl::ResetAll() {
	UpdateNetParamDisplay();
	graph.Clear();
}




}
