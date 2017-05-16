#include "Overlook.h"

namespace Overlook {

PrioritizerCtrl::PrioritizerCtrl() {
	Add(tabs.SizePos());
	
	tabs.Add(current);
	tabs.Add(current, "Current");
	tabs.Add(result);
	tabs.Add(result, "Results");
	
	split.Vert();
	current.Add(split.SizePos());
	split << pipeline_queue << process_queue << job_queue;
	
	pipeline_queue.AddColumn("ID");
	pipeline_queue.AddColumn("Combination");
	pipeline_queue.AddColumn("Priority");
	
	process_queue.AddColumn("ID");
	process_queue.AddColumn("Factory-ID");
	process_queue.AddColumn("Factory-Name");
	process_queue.AddColumn("Combination");
	process_queue.AddColumn("Symbols");
	process_queue.AddColumn("Timeframes");
	process_queue.AddColumn("Priority");
	process_queue.ColumnWidths("1 1 3 5 1 1");
	
	job_queue.AddColumn("ID");
	job_queue.AddColumn("Factory-ID");
	job_queue.AddColumn("Factory-Name");
	job_queue.AddColumn("Symbol");
	job_queue.AddColumn("Timeframe");
	job_queue.AddColumn("Priority");
	job_queue.ColumnWidths("1 1 2 2 1 1");
	
	result.Add(resultlist.SizePos());
	resultlist.AddColumn("ID");
	resultlist.AddColumn("Combination");
	resultlist.AddColumn("Classifier Value");
	resultlist.AddColumn("Floating Value");
	
}
	
void PrioritizerCtrl::RefreshData() {
	BaseSystem& base = *core->Get<BaseSystem>();
	Prioritizer& prio = *dynamic_cast<Prioritizer*>(core);
	
	
	String s;
	int tab = tabs.Get();
	if (tab == 0) {
		prio.lock.Enter();
		
		for(int i = 0; i < prio.GetCorelineCount(); i++) {
			CorelineItem& q = prio.GetCoreline(i);
			pipeline_queue.Set(i, 0, i);
			pipeline_queue.Set(i, 1, HexVector(q.value));
			pipeline_queue.Set(i, 2, q.priority);
		}
		pipeline_queue.SetCount(prio.GetCorelineCount());
		
		for(int i = 0; i < prio.GetCoreCount(); i++) {
			CoreItem& q = prio.GetCore(i);
			process_queue.Set(i, 0, i);
			process_queue.Set(i, 1, q.factory);
			process_queue.Set(i, 2, Factory::GetCtrlFactories()[q.factory].a);
			process_queue.Set(i, 3, HexVector(q.value));
			
			s = "";
			for(int j = 0; j < q.symlist.GetCount(); j++) {
				if (j) s.Cat(',');
				const int& a = q.symlist.GetKey(j);
				const int& b = q.symlist[j];
				s << "(" << a << "," << b << ")";
			}
			process_queue.Set(i, 4, s);
			
			s = "";
			for(int j = 0; j < q.tflist.GetCount(); j++) {
				if (j) s.Cat(',');
				const int& a = q.tflist.GetKey(j);
				const int& b = q.tflist[j];
				s << "(" << a << "," << b << ")";
			}
			process_queue.Set(i, 5, s);
			
			process_queue.Set(i, 6, q.priority);
		}
		process_queue.SetCount(prio.GetCoreCount());
		
		for(int i = 0; i < prio.GetJobCount(); i++) {
			JobItem& j = prio.GetJob(i);
			
			job_queue.Set(i, 0, i);
			job_queue.Set(i, 1, j.factory);
			job_queue.Set(i, 2, Factory::GetCtrlFactories()[j.factory].a);
			job_queue.Set(i, 3, base.GetSymbol(j.sym));
			job_queue.Set(i, 4, base.GetPeriodString(j.tf));
			job_queue.Set(i, 5, j.priority);
		}
		job_queue.SetCount(prio.GetJobCount());
		
		prio.lock.Leave();
	}
	else {
		
	}
}

}
