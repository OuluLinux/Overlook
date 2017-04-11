#include "NARX.h"

namespace Narx {

extern int series_len;

FeedbackInfo::FeedbackInfo() {
	this->xx = 0;
	this->xy = 0;
	this->yx = 0;
	this->yy = 0;
	this->dx = 0;
	this->dy = 0;
}

void FeedbackInfo::Init(int xx, int xy, int yx, int yy, int dx, int dy) {
	this->xx = xx;
	this->xy = xy;
	this->yx = yx;
	this->yy = yy;
	this->dx = dx;
	this->dy = dy;
	
	X.SetCount(xx);
	for(int i = 0; i < xx; i++)
		X[i].SetCount(xy,0);
	
	Y.SetCount(yx);
	for(int i = 0; i < yx; i++)
		Y[i].SetCount(yy,0);
	
	D.SetCount(dx);
	for(int i = 0; i < dx; i++)
		D[i].SetCount(dy,0);
}


FeedbackInfo::~FeedbackInfo() {
	
}

}
