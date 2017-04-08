#pragma once

class FeedbackInfo : Moveable<FeedbackInfo> {
public:
	Vector<double> Y;
	Vector<double> X;
	Vector<double> D;

	int x, y, d;

public:
	FeedbackInfo();
	~FeedbackInfo();
	
	void Init(int x, int y, int d);
};

