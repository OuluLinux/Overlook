#include "Overlook.h"

namespace Overlook {

AgentGroup::AgentGroup() {
	
}

void AgentGroup::Init() {
	Panic("TODO");
}

void AgentGroup::StoreThis() {
	ASSERT(!name.IsEmpty());
	String backup_dir = ConfigFile("backups");
	RealizeDirectory(backup_dir);
	Time t = GetSysTime();
	String file = ConfigFile(name + ".agrp");
	bool rem_bak = false;
	if (FileExists(file)) {
		MoveFile(file, file + ".bak");
		rem_bak = true;
	}
	StoreToFile(*this,	file);
	if (rem_bak)
		DeleteFile(file + ".bak");
}

void AgentGroup::LoadThis() {
	LoadFromFile(*this,	ConfigFile(name + ".agrp"));
}

void AgentGroup::Serialize(Stream& s) {
	s % agents % latest_signals % tf_ids % created % name;
}

int AgentGroup::GetSignalPos(int shift) {
	Panic("TODO");
	return 0;
}

int AgentGroup::GetSignalPos(int shift, int group_id) {
	Panic("TODO");
	return 0;
}









Manager::Manager() {
	
	
	
}

void Manager::Init() {
	FindFile ff(ConfigFile("*.agrp"));
	while (ff) {
		AgentGroup& ag = groups.Add();
		ag.name = GetFileTitle(ff.GetName());
		ag.LoadThis();
		ag.Init();
		ff.Next();
	}
}

void Manager::Start() {
	
	
	
}

void Manager::Stop() {
	
	
	
}

}
