#ifndef _DataCore_Slot_h_
#define _DataCore_Slot_h_

#include <Core/Core.h>
#include <ConvNet/ConvNet.h>
#undef ASSERTEXC

namespace DataCore {
using namespace Upp;

class TimeVector;

struct DataExc : public Exc {
	DataExc();
	DataExc(String msg);
};
#define ASSERTEXC(x) if (!(x)) throw DataExc(#x);
#define ASSERTEXC_(x, msg) if (!(x)) throw DataExc(msg);

// Helper macros for indicator short names
#define SHORTNAME0(x) x
#define SHORTNAME1(x, a1) x "(" + DblStr(a1) + ")"
#define SHORTNAME2(x, a1, a2) x "(" + DblStr(a1) + "," + DblStr(a2) + ")"
#define SHORTNAME3(x, a1, a2, a3) x "(" + DblStr(a1) + "," + DblStr(a2) + "," + DblStr(a3) + ")"
#define SHORTNAME4(x, a1, a2, a3, a4) x "(" + DblStr(a1) + "," + DblStr(a2) + "," + DblStr(a3) + "," + DblStr(a4) + ")"
#define SHORTNAME5(x, a1, a2, a3, a4, a5) x "(" + DblStr(a1) + "," + DblStr(a2) + "," + DblStr(a3) + "," + DblStr(a4) + "," + DblStr(a5) + ")"
#define SHORTNAME6(x, a1, a2, a3, a4, a5, a6) x "(" + DblStr(a1) + "," + DblStr(a2) + "," + DblStr(a3) + "," + DblStr(a4) + "," + DblStr(a5) + "," + DblStr(a6) +  ")"
#define SHORTNAME7(x, a1, a2, a3, a4, a5, a6, a7) x "(" + DblStr(a1) + "," + DblStr(a2) + "," + DblStr(a3) + "," + DblStr(a4) + "," + DblStr(a5) + "," + DblStr(a6) + "," + DblStr(a7) + ")"
#define SHORTNAME8(x, a1, a2, a3, a4, a5, a6, a7, a8) x "(" + DblStr(a1) + "," + DblStr(a2) + "," + DblStr(a3) + "," + DblStr(a4) + "," + DblStr(a5) + "," + DblStr(a6) + "," + DblStr(a7) + "," + DblStr(a8) + ")"

typedef Vector<byte> SlotData;
typedef Ptr<TimeVector> TimeVectorPtr;
class Slot;

enum {SLOT_SYMTF, SLOT_SYM, SLOT_TF, SLOT_ONCE};

struct SlotProcessAttributes : Moveable<SlotProcessAttributes> {
	int sym_id;								// id of symbol
	int sym_count;							// count of symbol ids
	int tf_id;								// timeframe-id from shorter to longer
	int tf_count;							// count of timeframe ids
	int64 time;								// time of current position
	int pos[16];							// time-position for all timeframes
	int bars[16];							// count of time-positions for all timeframes
	int periods[16];						// periods for all timeframes
	int slot_bytes;							// reserved memory in bytes for current slot
	int slot_pos;							// position of the current slot in the sym/tf/pos vector
	Slot* slot;								// pointer to the current slot
	TimeVector* tv;							// pointer to the parent TimeVector
	
	int GetBars() const {return bars[tf_id];}
	int GetCounted() const {return pos[tf_id];}
	int GetPeriod() const {return periods[tf_id];}
};


class Slot : public Pte<Slot> {
	
protected:
	friend class TimeVector;
	typedef Ptr<Slot> SlotPtr;
	
	void SetWithoutData(bool b=true) {forced_without_data = b;}
	
	Vector<Vector<Vector<Vector<byte> > > > data;
	Vector<Vector<Vector<byte> > > ready;
	VectorMap<String, SlotPtr> dependencies;
	String linkpath, path, style, filedir;
	SlotPtr source;
	TimeVector* vector;
	
	struct SlotValue : Moveable<SlotValue> {
		int bytes;
		String name, description;
	};
	Vector<SlotValue> values;
	int id;
	bool forced_without_data;
	
public:
	Slot();
	virtual ~Slot() {}
	
	virtual void Serialize(Stream& s) {}
	virtual void SerializeCache(Stream& s, int sym_id, int tf_id) {}
	void LoadCache(int sym_id, int tf_id);
	void StoreCache(int sym_id, int tf_id);
	
	SlotPtr FindLinkSlot(const String& path);
	SlotPtr ResolvePath(const String& path);
	void LinkPath(String dest, String src);
	void AddValue(uint16 bytes, String name="", String description="");
	
	template <class T>
	void AddValue(String name="", String description="") {AddValue(sizeof(T), name, description);}
	void AddDependency(String slot_path, bool other_symbols, bool other_timeframes);
	
	bool IsWithoutData() const {return forced_without_data;}
	int GetCount() const {return values.GetCount();}
	const SlotValue& operator[] (int i) const {return values[i];}
	bool IsReady(int pos, const SlotProcessAttributes& attr);
	bool IsReady(const SlotProcessAttributes& attr) {return IsReady(attr.GetCounted(), attr);}
	String GetPath() const {return path;}
	String GetLinkPath() const {return linkpath;}
	String GetFileDir() const {ASSERT(!filedir.IsEmpty()); return filedir;}
	const String& GetStyle() const {return style;}
	const Slot& GetDependency(int i);
	String GetDependencyPath(int i) const {return dependencies.GetKey(i);}
	int GetDependencyCount() const {return dependencies.GetCount();}
	int GetId() const {return id;}
	
	template <class T>
	T* GetValue(int i, const SlotProcessAttributes& attr) const {
		const Vector<byte>& row = data[attr.sym_id][attr.tf_id][i];
		int pos = sizeof(T) * attr.pos[attr.tf_id];
		if (pos < 0 || pos + sizeof(T) > row.GetCount())
			return NULL;
		return (T*)&row[pos];
	}
	template <class T>
	T* GetValue(int i, int shift, const SlotProcessAttributes& attr) const {
		const Vector<byte>& row = data[attr.sym_id][attr.tf_id][i];
		int pos = sizeof(T) * (attr.pos[attr.tf_id] - shift);
		if (pos < 0 || pos + sizeof(T) > row.GetCount())
			return NULL;
		return (T*)&row[pos];
	}
	template <class T>
	T* GetValue(int i, int tf_id, int shift, const SlotProcessAttributes& attr) const {
		const Vector<byte>& row = data[attr.sym_id][tf_id][i];
		int pos = sizeof(T) * (attr.pos[tf_id] - shift);
		if (pos < 0 || pos + sizeof(T) > row.GetCount())
			return NULL;
		return (T*)&row[pos];
	}
	template <class T>
	T*  GetValue(int i, int sym_id, int tf_id, int shift, const SlotProcessAttributes& attr) const {
		const Vector<byte>& row = data[sym_id][tf_id][i];
		int pos = sizeof(T) * (attr.pos[tf_id] - shift);
		if (pos < 0 || pos + sizeof(T) > row.GetCount())
			return NULL;
		return (T*)&row[pos];
	}
	template <class T>
	T* GetValuePos(int i, int sym_id, int tf_id, int pos) const {
		const Vector<byte>& row = data[sym_id][tf_id][i];
		pos *= sizeof(T);
		if (pos < 0 || pos + sizeof(T) > row.GetCount())
			return NULL;
		return (T*)&row[pos];
	}
	
	void SetReady(int sym, int tf, int pos, const SlotProcessAttributes& attr, bool ready=true);
	void SetReady(int pos, const SlotProcessAttributes& attr, bool ready=true);
	void SetReady(const SlotProcessAttributes& attr, bool ready=true) {SetReady(attr.GetCounted(), attr, ready);}
	void SetPath(String p) {ASSERT(path.IsEmpty()); path = p;}
	void SetLinkPath(String p) {ASSERT(linkpath.IsEmpty()); linkpath = p;}
	void SetSource(SlotPtr sp) {source = sp;}
	void SetTimeVector(TimeVector* vector) {this->vector = vector;}
	void SetStyle(const String& json) {style = json;}
	void SetFileDirectory(String s) {filedir = s;}
	
	virtual String GetKey() const {return "slot";}
	virtual String GetName() {return "name";}
	virtual String GetShortName() const {return "shortname";}
	virtual String GetCtrl() const {return "default";}
	virtual int GetCtrlType() const {return SLOT_SYMTF;}	// new ctrl for every sym and tf
	virtual int GetType() const {return SLOT_SYMTF;}		// processing separately for every sym and tf
	virtual void SetArguments(const VectorMap<String, Value>& args) {}
	virtual void Init() {}
	virtual void Start() {}
	virtual bool Process(const SlotProcessAttributes& attr) {return 1;}
	
	
};

typedef Ptr<Slot> SlotPtr;



}

#endif
