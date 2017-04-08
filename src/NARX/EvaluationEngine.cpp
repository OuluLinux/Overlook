#include "NARX.h"


EvaluationEngine::EvaluationEngine(int slen)
{
	series_len = slen;
	series = new double[slen];
	predicted = new double[slen];


	curlen = 0;
}


EvaluationEngine::~EvaluationEngine(void)
{
	delete [] series;
	delete [] predicted;
}

void EvaluationEngine::insertvalue(double target, double pred)
{
	series[curlen]=target;
	predicted[curlen]=pred;
	FLOG(QString("series:%1,predicted:%2\n").arg(target).arg(pred).toStdString().c_str());
	curlen++;
}

double EvaluationEngine::F1()
{
	double ret = 0;
	
	for(int i=0;i<curlen;i++)
	{
		ret += qPow(series[i] - predicted[i], 2);
		//FLOG(QString(":%1,%2\n").arg(series[i]).arg(predicted[i]).toStdString().c_str());
	}

	return (ret);
}

double EvaluationEngine::F2()
{
	return qSqrt(F1() / (curlen));
}

double EvaluationEngine::F3()
{
	double ret = 0;
	double sum = 0;
	for(int i=0;i<curlen;i++)
	{
         sum += qAbs(series[i]);
	}

	return F2() / sum * curlen;
}

double EvaluationEngine::F4()
{
	double s1 = 0;
	double s2 = 0;
	double avg = 0;

	for(int i=0;i<curlen;i++)
		avg += qAbs(series[i]);
	avg /= curlen;

	for(int i=0;i<curlen;i++)
	{
         s1 += qPow(series[i] - predicted[i], 2);
		 s2 += qPow(series[i] - avg, 2);
	}
	//char test[256];
	//	sprintf(test, "aa %f %f\n", s1, s2);
		//FLOG(test);

	return qSqrt(s1/s2);
}


double EvaluationEngine::Fd(double val)
{
	double c=0;
	for(int i=0;i<curlen;i++)
		if(series[i]<=val) c++;
	return c/curlen;
}

double EvaluationEngine::Fy(double val)
{
	double c=0;
	for(int i=0;i<curlen;i++)
		if(predicted[i]<=val) c++;
	return c/curlen;
}

double EvaluationEngine::KS1()
{
	double max = 0;
	for(int i=0;i<curlen;i++)
	{
			double t = qAbs(Fd(series[i]) - Fy(series[i]));
			if(max < t) max = t;
	}

	return max;
}

double EvaluationEngine::KS2()
{
	double max = 0;
	for(int i=0;i<curlen;i++)
	{
			double t = qAbs(Fd(predicted[i]) - Fy(predicted[i]));
			if(max < t) max = t;
	}
	return max;
}

double EvaluationEngine::KS12()
{
	double max = 0;
	for(int i=0;i<curlen;i++)
	{
			double t = qAbs(Fd(series[i]) - Fy(predicted[i]));
			if(max < t) max = t;
	}
	return max;
}

double EvaluationEngine::DA()
{
	double ret=0;
	for(int i=0;i<curlen;i++) {
		if(Fd(predicted[i])!=0 && Fd(predicted[curlen-1-i])!=0) 
		ret+=(2*i+1)/curlen * ( qLn(Fd(predicted[i])) + qLn(Fd(predicted[curlen-1-i])));
		//char test[256];
		//sprintf(test, "aa %f %f %f\n", Fd(predicted[i]),(2*i+1)/curlen * ( qLn(Fd(predicted[i])) + qLn(Fd(predicted[curlen-1-i])))
		//	,ret
		//	);
		//FLOG(test);
	}
	return - curlen - ret;
}

void EvaluationEngine::reset()
{
	//for( int i=0;i<series_len;i++)
	//	series[i]=predicted[i]=0;
	//FLOG(QString("curlen:%1\n").arg(curlen).toStdString().c_str());
	curlen = 0;
}