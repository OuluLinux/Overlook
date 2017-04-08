#pragma once
class FeedbackInfo
{
public:
	double *Y;
	double *X;
	double *D;

	int x, y, d;

public:
	FeedbackInfo(int x, int y, int d);
	~FeedbackInfo(void);
};

