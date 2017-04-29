#pragma once

namespace Narx {

class FeedbackInfo : Moveable<FeedbackInfo> {
public:
	Vector<Vector<double> > Y;
	Vector<Vector<double> > X;
	Vector<Vector<double> > D;

	int xx, yx, dx;
	int xy, yy, dy;

public:
	FeedbackInfo();
	~FeedbackInfo();
	
	void Init(int xx, int xy, int yx, int yy, int dx, int dy);
	
	void Serialize(Stream& s) {s % Y % X % D % xx % yx % dx % xy % yy % dy;}
};

}
