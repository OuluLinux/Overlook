#ifndef _Forecaster_Heatmap_h_
#define _Forecaster_Heatmap_h_

namespace Forecast {


class Heatmap {
	Vector<double> data;
	double low = 0, high = 0, height = 0, ystep = 0;
	int ysize = 0, xsize = 0, xstep = 0, div = 0;
	
	
	int Pos(int x, double y) const;
	
public:
	typedef Heatmap CLASSNAME;
	
	Heatmap();
	void Init(double low, double high, double min_ystep, double ystep, int length, int xstep);
	void Add(int x, double y, double value);
	double Get(int x, double y) const {int i = Pos(x, y); if (i < 0 || i >= data.GetCount()) return 0; return data[i] / div;}
	int GetWidth() const {return xsize;}
	void CopyRight(Heatmap& dst, int size);
	
	void operator=(const Heatmap& s) {
		data <<= s.data;
		low = s.low;
		high = s.high;
		height = s.height;
		ystep = s.ystep;
		ysize = s.ysize;
		xsize = s.xsize;
		xstep = s.xstep;
		div = s.div;
	}
	
	void Serialize(Stream& s) {s % data % low % high % height % ystep % ysize % xsize % xstep % div;}
};



}



#endif
