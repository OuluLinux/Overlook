#include "FeedbackInfo.h"

extern int series_len;


FeedbackInfo::FeedbackInfo(int x, int y, int d)
{
	this->x=x;
	this->y=y;
	this->d=d;

	X = new double[x];
	
	Y = new double[y];
	
	D = new double[d];
	
}


FeedbackInfo::~FeedbackInfo(void)
{

	
	delete []X;

	delete [] Y;

	delete [] D;
}
