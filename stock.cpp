
#include <iostream>
#include <vector>
#include <string>
#include <curl/curl.h>
#include <sstream>
#include <numeric>
#include <math.h>
#include <tuple>

#include "stock.h"
#include "invnorm.h"

using namespace std;

static int WriteCallback(void *contents, int size, int nmemb, void *userp)
{
	// used by libcurl to download web data
	((string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}


Stock::Stock(string _ticker): ticker(_ticker){

}

Stock::~Stock() {

}

void Stock::init_hist() {
	// downloads historical price data
	CURL *curl;
	CURLcode res;
	string readBuffer;
	string readline;

	string url = "https://api.iextrading.com/1.0/stock/" + ticker + "/chart/1y";
	//cout << url << endl;
	curl = curl_easy_init();

	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);



		size_t pos = 0;
		string delim;
		string date, close;
		stringstream ss(readBuffer);
		while (true) {
			getline(ss, readline, '}');
			if (readline == "]")
				break;
			// collecting dates into vector
			delim = "\"date\":";
			pos = readline.find(delim);
			readline.erase(0, pos + delim.length());
			pos = readline.find(",");
			date = readline.substr(0, pos);
			date = date.substr(1, date.length() - 2);

			hist_dates.push_back(date);

			// collecting close prices into vector
			delim = "\"close\":";
			pos = readline.find(delim);
			readline.erase(0, pos + delim.length());
			pos = readline.find(",");
			close = readline.substr(0, pos);

			hist_prices.push_back(stod(close));

		}

	}
	curl_global_cleanup();
	calc_returns();
	calc_vol();
	calc_3mo_ra();


}


void Stock::calc_returns() {

	auto position = hist_prices.begin();
	++position;
	for (auto i = position; i != hist_prices.end(); ++i) {
		hist_returns.push_back(log(*i / *(i - 1)));
	}
}

void Stock::calc_vol() {
	double average = accumulate(hist_returns.begin(), hist_returns.end(), 0.0) / hist_returns.size();
	double total = 0;
	//cout << average << endl;
	for (auto i = hist_returns.begin(); i != hist_returns.end(); ++i) {
		total += (*i - average) * (*i - average);
	}
	vol = sqrt((total / (hist_returns.size() - 1))) * sqrt(252);
	cout << ticker << " volatility: " << vol << endl;
}

void Stock::calc_3mo_ra() {
	double sum = 0;
	int end = hist_prices.size();
	if (end > 90) {
		for (int i = 1; i < 91; ++i) {
			sum += hist_prices[end - i];
		}
		double average = sum / 90;
		//cout << "90 day average: " << average << endl;
		//cout << "last price: " << hist_prices[end - 1] << endl;
	}
	else {
		throw "not enough data";
	}
}

void Stock::set_days_until_exp(const int& days) {
	days_until_exp = days;
}

void Stock::set_forecasted_vol(const double& vol) {
	forecasted_vol = vol;
}

void Stock::set_ci_low(const double& vol) {
	ci_low = vol;
}

void Stock::set_ci_high(const double& vol) {
	ci_high = vol;
}

const double& Stock::get_last_price() const {
	int end = hist_prices.size();
	return hist_prices[end - 1];
}

const double& Stock::get_last_return() const {
	int end = hist_returns.size();
	return hist_returns[end - 1];
}

const string& Stock::get_ticker() const {
	return ticker;
}

const vector<double> Stock::get_hist_prices() const{
	return hist_prices;
}

const double& Stock::get_vol() const {
	return vol;
}

const int& Stock::get_days_until_exp() const {
	return days_until_exp;
}

const double& Stock::get_forecasted_vol() const {
	return forecasted_vol;
}

const double& Stock::get_ci_high() const {
	return ci_high;
}

const double& Stock::get_ci_low() const {
	return ci_low;
}

const double Stock::vol_forecast(const int& days, vector<double> garch, const int& seed) const {
	// uses GARCH(1,1) model to forcast volatility

	vector<double> Y;
	vector<double> variance;
	double tempY;
	double tempvar;

	// initial value for Y2 and var is the average of Y squared in the past
	vector<double> past_Y2;
	for (auto e : hist_returns)
		past_Y2.push_back(e*e);
	
	double average_Y2 = accumulate(past_Y2.begin(), past_Y2.end(), 0.0) / past_Y2.size();

	Y.push_back(sqrt(average_Y2));
	variance.push_back(average_Y2);
	srand(seed);
	//srand(time(NULL));
	double a = (double)rand() / (RAND_MAX);


	// generating normal numbers, but careful not to get exactly 0 or 1
	vector<double> R;
	for (int i = 0; i < days; ++i) {
		R.push_back((double)rand() / (RAND_MAX));
		if (R[i] == 0)
			R[i] = 1.2e-7;
		else if (R[i] == 1) {
			R[i] = 1 - 1.2e-7;
		}
	}

	// inverse normal geneartor
	vector<double> E(invnor(R));



	for (int i = 0; i < days; ++i) {
		tempvar = garch[0] + garch[1] * Y[i] * Y[i] + garch[2]*variance[i];
		tempY = sqrt(tempvar) * E[i];
		
		Y.push_back(tempY);
		variance.push_back(tempvar);
	}
	
	//cout << tempY << endl;
	return sqrt(tempvar) * sqrt(252);
}

const tuple<double, double> Stock::conf_int(vector<double> garchresults, const double& average) {
	double clow, chigh;
	
	double sum = 0;
	for (auto e : garchresults) {
		sum += (e - average) * (e - average);
	}
	double sigma = sqrt(sum / (garchresults.size() - 1));

	clow = average - 1.96 * sigma / sqrt(garchresults.size());
	chigh = average + 1.96 * sigma / sqrt(garchresults.size());

	tuple<double, double> temp(clow, chigh);

	return temp;
}