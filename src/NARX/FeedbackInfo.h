#pragma once

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
};

