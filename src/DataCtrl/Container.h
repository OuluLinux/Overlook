#ifndef _DataCtrl_Container_h_
#define _DataCtrl_Container_h_

#include <CtrlLib/CtrlLib.h>
#include <DataCore/DataCore.h>
#include <RefCore/RefCore.h>
#include <RefCtrl/RefCtrl.h>


namespace DataCtrl {
using namespace RefCore;
using namespace RefCtrl;
using namespace DataCore;


// Visual setting enumerators
enum {DRAW_LINE, DRAW_SECTION, DRAW_HISTOGRAM, DRAW_ARROW, DRAW_ZIGZAG, DRAW_NONE};
enum {WINDOW_CHART, WINDOW_SEPARATE};
enum {STYLE_SOLID, STYLE_DASH, STYLE_DOT, STYLE_DASHDOT, STYLE_DASHDOTDOT};


// Class for vertical levels on containers
struct DataLevel : public Moveable<DataLevel> {
	int style, line_width;
	double value; // height
	Color clr;
	
	DataLevel() : value(0), style(0), line_width(1) {}
	DataLevel& operator=(const DataLevel& src) {value = src.value; clr = src.clr; style = src.style; line_width = src.line_width; return *this;}
	void Serialize(Stream& s) {s % style % line_width % value % clr;}
};


// Class for default visual settings for a single visible line of a container
struct DataBufferSettings : public Moveable<DataBufferSettings> {
	int buffer;
	String label;
	Color clr;
	int style, line_style, line_width, chr, begin, shift;
	
	DataBufferSettings() : clr(Black()), style(0), line_width(1), chr('^'), begin(0), shift(0), line_style(0) {}
	void Serialize(Stream& s) {s % label % clr % style % line_style % line_width % chr % begin % shift;}
};


class Container : public MetaNode {
	
protected:
	// Settings
	String short_name;
	int counted, period_id;
	Ptr<DataCore::Slot> slot;
	
	// Visual settings
	Vector<DataLevel> levels;
	Vector<DataBufferSettings> buffer_settings;
	Color levels_clr;
	RWMutex lock;
	double minimum, maximum;
	double point;
	int levels_style;
	int window_type;
	int bars, next_count;
	bool has_maximum, has_minimum;
	bool skip_setcount;
	bool locked;
	
public:
	typedef Container CLASSNAME;
	Container();
	
	// Get settings
	int GetBufferSettingsCount() const {return buffer_settings.GetCount();}
	const String& GetBufferLabel(int i) {return buffer_settings[i].label;}
	int GetBars() {return bars;}
	int GetWindowType() {return window_type;}
	int GetCounted() {return counted;}
	double GetPoint() const {return point;}
	int GetContainerLevelCount() const {return levels.GetCount();}
	int GetContainerLevelType(int i) const {return levels[i].style;}
	int GetContainerLevelLineWidth(int i) const {return levels[i].line_width;}
	int GetBufferStyle(int i) {return buffer_settings[i].style;}
	int GetBufferArrow(int i) {return buffer_settings[i].chr;}
	double GetMaximum() const {return maximum;}
	double GetMinimum() const {return minimum;}
	double GetContainerLevelValue(int i) const {return levels[i].value;}
	double GetContainerMinimum() {return minimum;}
	double GetContainerMaximum() {return maximum;}
	bool IsContainerSeparateWindow() {return window_type == WINDOW_SEPARATE;}
	bool HasMaximum() const {return has_maximum;}
	bool HasMinimum() const {return has_minimum;}
	int GetBufferCount() {return buffer_settings.GetCount();}
	//Data32f& GetBuffer(int buffer) {return *buffer_settings[buffer].buffer;}
	//const Data32f& GetBuffer(int buffer) const {return *buffer_settings[buffer].buffer;}
	//Data32f& GetIndex(int buffer) {return *indices[buffer];}
	const DataBufferSettings& GetBufferSettings(int buffer) {return buffer_settings[buffer];}
	double GetBufferValue(int i, int shift);// {return buffer_settings[i].buffer->Get(shift);}
	double GetBufferValue(int shift);// {return buffer_settings[0].buffer->Get(shift);}
	int GetBufferDataCount();
	Color GetBufferColor(int i) {return buffer_settings[i].clr;}
	int GetBufferLineWidth(int i) {return buffer_settings[i].line_width;}
	int GetBufferType(int i) {return buffer_settings[i].line_style;}
	//int GetIndexCount() {return indices.GetCount();}
	double GetIndexValue(int i, int shift);// {return indices[i]->Get(shift);}
	double GetIndexValue(int shift);// {return indices[0]->Get(shift);}
	
	
	// Set settings
	void SetWindowType(int i) {window_type = i;}
	void SetBufferCount(int count) {buffer_settings.SetCount(count);}
	void SetBufferColor(int i, Color c) {buffer_settings[i].clr = c;}
	void SetBufferLineWidth(int i, int line_width) {buffer_settings[i].line_width = line_width;}
	void SetBufferType(int i, int style) {buffer_settings[i].line_style = style;}
	//void SetContainerDigits(int digits) {this->digits = digits;}
	void SetPoint(double d);
	void SetContainerLevelCount(int count) {levels.SetCount(count);}
	void SetContainerLevel(int i, double value) {levels[i].value = value;}
	void SetContainerLevelType(int i, int style) {levels[i].style = style;}
	void SetContainerLevelLineWidth(int i, int line_width) {levels[i].line_width = line_width;}
	void SetContainerLevelsColor(Color clr) {levels_clr = clr;}
	void SetContainerLevelsStyle(int style) {levels_style = style;}
	void SetContainerMinimum(double value) {minimum = value; has_minimum = true;}
	void SetContainerMaximum(double value) {maximum = value; has_maximum = true;}
	void SetBufferLabel(int i, String label)  {buffer_settings[i].label = label;}
	void SetBufferStyle(int i, int style)     {buffer_settings[i].style = style;}
	void SetBufferArrow(int i, int chr)       {buffer_settings[i].chr   = chr;}
	void SetContainerChartWindow() {window_type = WINDOW_CHART;}
	void SetContainerSeparateWindow() {window_type = WINDOW_SEPARATE;}
	void SetBufferShift(int i, int shift) {buffer_settings[i].shift = shift;}
	void SetBufferBegin(int i, int begin) {buffer_settings[i].begin = begin;}
	//void SetIndexCount(int count) {indices.SetCount(count);}
	//void SetIndexBuffer(int i, Data32f& vec);
	void ForceSetCounted(int i) {counted = i; bars = i; next_count = i;}
	void SetSkipSetCount(bool b=true) {skip_setcount = b;}
	
	// Visible main functions
	virtual String GetKey() const {return "cont";}
	virtual String GetCtrlKey() const {return "graph";}
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	
	//virtual String GetName() = 0;
	//virtual String GetShortName() = 0;
	String GetShortName() {return slot ? slot->GetKey() : "unknown";}
	
	void Refresh();
	void ClearContent();
	
	void Enter() {DataCore::GetTimeVector().EnterCache(); locked = true;}
	void Leave() {DataCore::GetTimeVector().LeaveCache(); locked = false;}
	
};



class BarData : public Container {
	
	
public:
	BarData();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	virtual String GetKey() const {return "bardata";}
	virtual String GetCtrlKey() const {return "graph";}
};

}

#endif
