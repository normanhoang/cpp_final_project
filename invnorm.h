
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

//uniform random number generator
vector<double> ran(int seed, int n = 1) {
	const int IM1 = 2147483563;
	const int IM2 = 2147483399;
	double AM = (1.0 / IM1);
	int IMM1 = (IM1 - 1);
	const int IA1 = 40014;
	const int IA2 = 40692;
	const int IQ1 = 53668;
	const int IQ2 = 52774;
	const int IR1 = 12211;
	const int IR2 = 3791;
	const int NTAB = 32;
	double NDIV = (1 + IMM1 / NTAB);
	double EPS = 1.2e-7;
	double RNMX = (1.0 - EPS);
	int idum = -seed;
	int idum2 = 123456789;
	int iy = 0;
	vector<int> iv(NTAB);

	vector<double> random(n);
	int k, j;
	for (auto it = random.begin(); it != random.end(); ++it) {
		if (idum <= 0) {
			idum = max(-idum, 1);
			idum2 = idum;
			j = NTAB + 8;
			while (j>0) {
				k = int(idum / IQ1);
				idum = IA1*(idum - k*IQ1) - k*IR1;
				if (idum<0) { idum = idum + IM1; }
				if (j <= NTAB) { iv.push_back(idum); }
				j = j - 1;
			}
			iy = iv[0];
		}
		k = int(idum / IQ1);
		idum = IA1*(idum - k*IQ1) - k*IR1;
		if (idum<0) { idum = idum + IM1; }
		k = int(idum2 / IQ2);
		idum2 = IA2*(idum2 - k*IQ2) - k*IR2;
		if (idum2<0) { idum2 = idum2 + IM2; }
		j = int(iy / NDIV) + 1;
		iy = iv[j] - idum2;
		iv[j] = idum;
		if (iy<1) { iy = iy + IMM1; }
		if (AM*iy<RNMX) { *it = AM*iy; }
		else { *it = RNMX; }
	}
	return(random);
}


//normal random number generator
vector<double> invnor(vector<double> uni) {
	double a0 = 2.50662823884;
	double a1 = -18.61500062529;
	double a2 = 41.39119773534;
	double a3 = -25.44106049637;
	double b0 = -8.47351093090;
	double b1 = 23.08336743743;
	double b2 = -21.06224101826;
	double b3 = 3.13082909833;
	double c0 = 0.3374754822726147;
	double c1 = 0.9761690190917186;
	double c2 = 0.1607979714918209;
	double c3 = 0.0276438810333863;
	double c4 = 0.0038405729373609;
	double c5 = 0.0003951896511919;
	double c6 = 0.0000321767881768;
	double c7 = 0.0000002888167364;
	double c8 = 0.0000003960315187;

	vector<double> invnor(uni.size());
	double x;
	int max = invnor.size();
	for (int i = 0; i< max; ++i) {
		double y = uni[i] - 0.5;
		if (fabs(y)<0.42) {
			double r = y*y;
			x = y*(((a3*r + a2)*r + a1)*r + a0) / ((((b3*r + b2)*r + b1)*r + b0)*r + 1);
		}
		else {
			double r = uni[i];
			if (y>0) r = 1 - uni[i];
			r = log(-log(r));
			x = c0 + r*(c1 + r*(c2 + r*(c3 + r*(c4 + r*(c5 + r*(c6 + r*(c7 + r*c8)))))));
			if (y<0) x = -x;
		}
		invnor[i] = x;
	}
	return(invnor);
}

