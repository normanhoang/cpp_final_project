// Parameter estimation for GARCH(1,1) model
// Fitted to S&P500 index levels
// By Fabrice Douglas Rouah, 2009
// For Visual C++.Net Version 7.1

#ifndef GARCH_H
#define GARCH_H

#include <vector>
#include <algorithm>
#include <iomanip>


using namespace std;


vector<double> VMean(vector<vector<double> > X, int n)
{
	vector<double> meanX(n);
	for (int i = 0; i <= n - 1; i++)
	{
		meanX[i] = 0.0;
		for (int j = 0; j <= n - 1; j++)
		{
			meanX[i] += X[i][j];
		}
		meanX[i] = meanX[i] / n;
	}
	return meanX;
}

// Function to add two vectors together
vector<double> VAdd(vector<double> x, vector<double> y)
{
	int n = x.size();
	vector<double> z(n, 0.0);
	for (int j = 0; j <= n - 1; j++)
		z[j] = x[j] + y[j];
	return z;
}

// Function to subtract two vectors
vector<double> VSub(vector<double> x, vector<double> y)
{
	int n = x.size();
	vector<double> z(n, 0.0);
	for (int j = 0; j <= n - 1; j++)
		z[j] = x[j] - y[j];
	return z;
}

// Function to multiply a vector by a constant
vector<double> VMult(vector<double> x, double a)
{
	int n = x.size();
	vector<double> z(n, 0.0);
	for (int j = 0; j <= n - 1; j++)
		z[j] = a*x[j];
	return z;
}

// Sum of a vector
double VecSum(vector<double> x)
{
	int n = x.size();
	double Sum = 0.0;
	for (int i = 0; i <= n - 1; i++)
		Sum += x[i];
	return Sum;
}

// Calculates unbiased sample variance
double VecVar(vector<double> x)
{
	double n = x.size();
	double sumM = 0.0;
	for (int i = 0; i <= n - 1; i++)
		sumM += x[i];
	double mean = sumM / n;
	double sumV = 0.0;
	for (int i = 0; i <= n - 1; i++)
		sumV += (x[i] - mean)*(x[i] - mean);
	return sumV / (n - 1);
}

// Nelder Mead Algorithmf
vector<double> NelderMead(double(*f)(vector<double>, vector<double>), int N, double NumIters, double MaxIters,
	double Tolerance, vector<double> Price)
{
	vector<vector<double> > x(N, vector<double>(N + 1));
	// Vertice 0	Vertice 1		Vertice 2		Vertice 3		Vertice 4
	x[0][0] = 0.00002;	x[0][1] = 0.00001;	x[0][2] = 0.00000015;		x[0][3] = 0.0000005;
	x[1][0] = 0.10;		x[1][1] = 0.11;		x[1][2] = 0.09;			x[1][3] = 0.15;
	x[2][0] = 0.85;		x[2][1] = 0.87;		x[2][2] = 0.90;			x[2][3] = 0.83;

	int i, j;

	// Value of the function at the vertices
	vector<vector<double> > F(N + 1, vector<double>(2));

	// Step 0.  Ordering and Best and Worst points
	// Order according to the functional values, compute the best and worst points
step0:
	NumIters = NumIters + 1;
	for (j = 0; j <= N; j++) {
		vector<double> z(N, 0.0);	       			 // Create vector to contain
		for (i = 0; i <= N - 1; i++)
			z[i] = x[i][j];
		F[j][0] = f(z, Price);         				 // Function values
		F[j][1] = j;			        		 // Original index positions
	}
	sort(F.begin(), F.end());

	// New vertices order first N best initial vectors and
	// last (N+1)st vertice is the worst vector
	// y is the matrix of vertices, ordered so that the worst vertice is last
	vector<vector<double> > y(N, vector<double>(N + 1));
	for (j = 0; j <= N; j++) {
		for (i = 0; i <= N - 1; i++) {

			y[i][j] = x[i][(int)F[j][1]];
		}
	}

	//  First best vector y(1) and function value f1
	vector<double> x1(N, 0.0); for (i = 0; i <= N - 1; i++) x1[i] = y[i][0];
	double f1 = f(x1, Price);

	// Last best vector y(N) and function value fn
	vector<double> xn(N, 0.0); for (i = 0; i <= N - 1; i++) xn[i] = y[i][N - 1];
	double fn = f(xn, Price);

	// Worst vector y(N+1) and function value fn1
	vector<double> xn1(N, 0.0); for (i = 0; i <= N - 1; i++) xn1[i] = y[i][N];
	double fn1 = f(xn1, Price);

	// z is the first N vectors from y, excludes the worst y(N+1)
	vector<vector<double> > z(N, vector<double>(N));
	for (j = 0; j <= N - 1; j++) {
		for (i = 0; i <= N - 1; i++)
			z[i][j] = y[i][j];
	}

	// Mean of best N values and function value fm
	vector<double> xm(N, 0.0); xm = VMean(z, N);
	double fm = f(xm, Price);

	// Reflection point xr and function fr
	vector<double> xr(N, 0.0); xr = VSub(VAdd(xm, xm), xn1);
	double fr = f(xr, Price);

	// Expansion point xe and function fe
	vector<double> xe(N, 0.0); xe = VSub(VAdd(xr, xr), xm);
	double fe = f(xe, Price);

	// Outside contraction point and function foc
	vector<double> xoc(N, 0.0);	xoc = VAdd(VMult(xr, 0.5), VMult(xm, 0.5));
	double foc = f(xoc, Price);

	// Inside contraction point and function foc
	vector<double> xic(N, 0.0);	xic = VAdd(VMult(xm, 0.5), VMult(xn1, 0.5));
	double fic = f(xic, Price);

	while ((NumIters <= MaxIters) && (abs(f1 - fn1) >= Tolerance))
	{
		// Step 1. Reflection Rule
		if ((f1 <= fr) && (fr<fn)) {
			for (j = 0; j <= N - 1; j++) {
				for (i = 0; i <= N - 1; i++)  x[i][j] = y[i][j];
			}
			for (i = 0; i <= N - 1; i++)  x[i][N] = xr[i];
			goto step0;
		}

		// Step 2.  Expansion Rule
		if (fr<f1) {
			for (j = 0; j <= N - 1; j++) {
				for (i = 0; i <= N - 1; i++)  x[i][j] = y[i][j];
			}
			if (fe<fr)
				for (i = 0; i <= N - 1; i++)	x[i][N] = xe[i];
			else
				for (i = 0; i <= N - 1; i++)	x[i][N] = xr[i];
			goto step0;
		}

		// Step 3.  Outside contraction Rule
		if ((fn <= fr) && (fr<fn1) && (foc <= fr)) {
			for (j = 0; j <= N - 1; j++) {
				for (i = 0; i <= N - 1; i++)  x[i][j] = y[i][j];
			}
			for (i = 0; i <= N - 1; i++)  x[i][N] = xoc[i];
			goto step0;
		}

		// Step 4.  Inside contraction Rule
		if ((fr >= fn1) && (fic<fn1)) {
			for (j = 0; j <= N - 1; j++) {
				for (i = 0; i <= N - 1; i++)  x[i][j] = y[i][j];
			}
			for (i = 0; i <= N - 1; i++)  x[i][N] = xic[i];
			goto step0;
		}

		// Step 5. Shrink Step
		for (i = 0; i <= N - 1; i++)
			x[i][0] = y[i][0];
		for (i = 0; i <= N - 1; i++) {
			for (j = 1; j <= N; j++)
				x[i][j] = 0.5*(y[i][j] + x[i][0]);
		}
		goto step0;
	}

	// Output component
	vector<double> out(N + 2);
	for (i = 0; i <= N - 1; i++)
		out[i] = x1[i];
	out[N] = f1;
	out[N + 1] = NumIters;
	return out;

}
// Log Likelihood for GARCH(1,1)
// Returns the negative Log Likelihood, for minimization
// Variance equation is
// h(t) = B[0] + B[1]*ret2[i+1] + B[2]*h[i+1];
// where B[0]=omega, B[1]=alpha, B[2]=beta

double LogLikelihood(vector<double> B, vector<double> Price)
{
	int i = 0;
	int n = Price.size();
	vector<double> ret(n - 1);					// Return
	vector<double> ret2(n - 1);					// Return squared
	vector<double> GARCH(n - 1, 0.0);				// GARCH(1,1) variance
	vector<double> LogLike(n - 1, 0.0);

	// Penalty for non-permissible parameter values
	if ((B[0]<0.0) || (B[1]<0.0) || (B[2]<0.0) || (B[1] + B[2] >= 1))
		return 1e100;
	else								// Construct the log likelihood
		for (i = 0; i <= n - 2; i++) {
			ret[i] = log(Price[i] / Price[i + 1]);
			ret2[i] = ret[i] * ret[i];
		}
	GARCH[n - 2] = VecVar(ret);
	LogLike[n - 2] = -log(GARCH[n - 2]) - ret2[n - 2] / GARCH[n - 2];
	for (i = n - 3; i >= 0; i--) {
		GARCH[i] = B[0] + B[1] * ret2[i + 1] + B[2] * GARCH[i + 1];
		LogLike[i] = -log(GARCH[i]) - ret2[i] / GARCH[i];
	}
	return -VecSum(LogLike);
}

#endif