#define _USE_MATH_DEFINES

#include <iostream>
#include <string>
#include <curl/curl.h>
#include <sstream>
#include <vector>
#include <map>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <cmath>

using namespace std;

#ifndef BS_H
#define BS_H
class BS {
private:
	double S, K, v, r, d, T;
	string type;

public:
	BS(const double& S_ = 100, const double& K_ = 100, const double& v_ = 0.3, const double& r_ = 0.02, const double& d_ = 0, const double& T_ = 1, const string type_ = "C") :S{ S_ }, K{ K_ }, v{ v_ }, r{ r_ }, d{ d_ }, T{ T_ }, type{ type_ } {}
	void set_r(const double& r_) { this->r = r_; }
	//Normal cdf
	double N(const double& x) { return 0.5*(1 + erf(x / sqrt(2))); }
	//to calculate d_1 and d_2
	double d_j(const int& j) { return (log(S / K) + (r + (pow(-1, j - 1))*0.5*v*v)*T) / (v*(pow(T, 0.5))); }
	double price() {
		if (type == "C" || type == "c") {
			return S*exp(-d*T)*N(d_j(1)) - K*exp(-r*T)*N(d_j(2));
		}
		else if (type == "P" || type == "p") {
			return K*exp(-r*T)*N(-d_j(2)) - S*exp(-d*T)*N(-d_j(1));
		}
		else {
			cerr << "not a valid option type" << endl;
			return 1;
		}
	}

	double Delta(const char type) {
		if (type == 'C') return exp(-d*T)*N(d_j(1));
		else return exp(-d*T)*(N(d_j(1)) - 1);
	}

	double Theta(const char type) {
		double days = 365 * T;
		double theta;
		if (type == 'C') {

			theta = (-((S*v*exp(-d*T)) / (2 * sqrt(T))*exp(-d_j(1)*d_j(1) / 2) / (sqrt(2 * M_PI))) - r*K*exp(-r*T)*N(d_j(2))
				+ d*S*exp(-d*T)*N(d_j(1)));
			return 1 / days*theta;
		}
		else {

			theta = (-((S*v*exp(-d*T)) / (2 * sqrt(T))*exp(-d_j(1)*d_j(1) / 2) / (sqrt(2 * M_PI))) + r*K*exp(-r*T)*N(-d_j(2))
				- d*S*exp(-d*T)*N(-d_j(1)));
			return  1 / days*theta;
		}
	}

	double Gamma() { return exp(-d*T) / (S*v*sqrt(T)) * 1 / (sqrt(2 * M_PI))*exp(-d_j(1)*d_j(1) / 2); }
	double Vega() { return S*exp(-d*T)*sqrt(T) * 1 / (sqrt(2 * M_PI))*exp(-d_j(1)*d_j(1) / 2); }
	double impvol(double v0, double op) {
		double keepvol = v;
		for (int i = 0; i<100000; ++i) {
			v = v0;
			if (fabs(op - price()) <= 0.0001) {
				v = keepvol;
				return v0;
			}
			v0 = v0 + (op - price()) / Vega();
		}
		return -1;
	}


};

#endif