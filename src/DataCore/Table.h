#ifndef _DataCore_Table_h_
#define _DataCore_Table_h_

#include <CoreUtils/CoreUtils.h>
#include <RefCore/RefCore.h>

namespace DataCore {
using namespace RefCore;



class Table : public MetaNode {
	
	Vector<String> columns;
	Vector<Vector<Value> > values;
	
	RWMutex lock;
	
protected:
	
	Table();
	
	void AddColumn(String title) {columns.Add(title);}
	void Set(int i, int j, const Value& val);
	
public:
	
	virtual void Serialize(Stream& s) {MetaNode::Serialize(s); s % columns % values;}
	
	// Visible main functions
	virtual String GetName() = 0;
	virtual String GetShortName() = 0;
	void Refresh();
	void ClearContent();
	
	int GetColumnCount() const {return columns.GetCount();}
	int GetCount() const {return values.GetCount();}
	String GetColumn(int i) const {return columns[i];}
	const Value& Get(int i, int j) const {return values[i][j];}
	
};




}

#endif
