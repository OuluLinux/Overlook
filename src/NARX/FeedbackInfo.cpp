#include "NARX.h"

extern int series_len;


FeedbackInfo::FeedbackInfo() {
	this->x = 0;
	this->y = 0;
	this->d = 0;
}

void FeedbackInfo::Init(int x, int y, int d) {
	this->x = x;
	this->y = y;
	this->d = d;
	X.SetCount(x, 0);
	Y.SetCount(y, 0);
	D.SetCount(d, 0);
}


FeedbackInfo::~FeedbackInfo() {
	
}
