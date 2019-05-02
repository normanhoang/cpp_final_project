#ifndef STOCK_H
#define STOCK_H

#include <iostream>
#include <tuple>
#include <vector>
#include <string>
#include <curl/curl.h>
#include <sstream>


using namespace std;



class Stock {
private:
	string ticker;
	vector<string> hist_dates;
	vector<double> hist_prices;
	vector<double> hist_returns;
	double vol;
	double forecasted_vol;
	double ci_low;
	double ci_high;
	int days_until_exp;
public:
	Stock(string ticker);
	~Stock();

	void init_hist();
	void calc_returns();
	void calc_vol();
	void calc_3mo_ra();
	void set_days_until_exp(const int& days);
	void set_forecasted_vol(const double& vol);
	void set_ci_low(const double& vol);
	void set_ci_high(const double& vol);

	const string& get_ticker() const;
	const vector<double> get_hist_prices() const;
	const double& get_vol() const;
	const double& get_last_price() const;
	const double& get_last_return() const;
	const int& get_days_until_exp() const;
	const double& get_forecasted_vol() const;
	const double& get_ci_high() const;
	const double& get_ci_low() const;

	const double vol_forecast(const int& days, vector<double> garch, const int& seed) const;
	const tuple<double, double> conf_int(vector<double> garchresults, const double& average);
};



#endif