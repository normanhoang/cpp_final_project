#define GARCH_ITERATIONS 200
#define MC_ITERATIONS 1500
#define RISK_FREE_RATE 0.02375
#define _USE_MATH_DEFINES

#include <iostream>
#include <string>
#include <curl/curl.h>
#include <vector>
#include <fstream>
#include <iterator> 
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <numeric>
#include <tuple>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <map>

#include "stock.h"
#include "garch.h"
#include "option.h"
#include "BS.h"

using namespace std;
namespace bd = boost::gregorian;
namespace pt = boost::posix_time;

struct save_data {
	string ticker;
	char call_or_put;
	string exp_date;
	double option_price;
	double strike;
	double forecasted_vol;
	double implied_vol;
	double vol_diff;
	float days_until_exp;
	int volume;
	double ci_high;
	double ci_low;
	double delta;
};

bool compareStruct(save_data sd1, save_data sd2) {
	return (fabs(sd1.vol_diff) > fabs(sd2.vol_diff));
}

int main() {

	ifstream in_file("SP100.csv");
	string readline;
	int count = 0;

	if (!in_file.is_open()) {
		cerr << "Failed to open csv file.\n";
		return -1;
	}

	
	while (getline(in_file, readline))
		++count;

	//we do not need to include the header for dynamic array
	--count;

	in_file.clear();
	in_file.seekg(0);



	vector<Stock> stock_list;
	vector<Option> option_list;
	//vector<string> ticker_list{ "PEP", "LOW", "MRK" };
	
	// uncomment for full download
	vector<string> ticker_list;

	for (int i = 0; i < count; ++i) {
		getline(in_file, readline);
		//cout << readline << endl;
		ticker_list.push_back(readline);
	}

	
	
	for (auto it = ticker_list.begin(); it != ticker_list.end(); ++it) {
		stock_list.push_back(Stock(*it));
		option_list.push_back(Option(*it));
	}



	int N = 3;						// Number of GARCH(1,1) parameters
	double NumIters = 1;			// First Iteration
	double Tolerance = 1e-15;		// Tolerance on best and worst function values

	vector<save_data> data_list;


	int year = 2019;
	int month = 7;
	int day = 19;

	cout << "enter expiration date" << endl;
	cout << "year: ";
	cin >> year;
	cout << "month: ";
	cin >> month;
	cout << "day: ";
	cin >> day;

	auto it2 = option_list.begin();
	for (auto it = stock_list.begin(); it != stock_list.end();) {

		try {
			cout << (*it).get_ticker() << endl;

			(*it).init_hist();
			cout << "current price: " << (*it).get_last_price() << endl;
			
			vector<double> NM = NelderMead(LogLikelihood, N, NumIters, GARCH_ITERATIONS, Tolerance, it->get_hist_prices());
			cout << "GARCH(1,1) Parameters" << endl;
			cout << "--------------------------------" << endl;
			cout << "Omega       = " << NM[0] << endl;
			cout << "Alpha       = " << NM[1] << endl;
			cout << "Beta        = " << NM[2] << endl;
			//cout << "Persistence = " << NM[1] + NM[2] << endl;
			cout << "--------------------------------" << endl;
			cout << "Log Likelihood value = " << NM[3] << endl;
			cout << "Number of iterations = " << NM[4] << endl;


			MostLiquid ml;
			

			(*it2).set_expiration_dates();

			// set for July 2019
			(*it2).get_urls(year, month, day);
			(*it2).init_current();
			vector <string> exp = (*it2).get_expirations();
			(*it2).set_type();
			(*it2).set_ttm();
			(*it2).div_yield();

			cout << "type, K, vol, d, ttm, mid" << endl;

			// grabs call or put with the most volume
			MostLiquid a = (*it2).get_most_liquid('C', exp[0]);
			cout << "C " << a.K << " " << a.v << " " << a.d << " " << a.T << " " << a.Mid << endl;
			BS bs_a((*it).get_last_price(), a.K, .3, RISK_FREE_RATE, a.d, a.T / 365, "C");
			a.ivol = bs_a.impvol(.5, a.Mid);

			MostLiquid b = (*it2).get_most_liquid('P', exp[0]);
			cout << "P " << b.K << " " << b.v << " " << b.d << " " << b.T << " " << b.Mid << endl;
			BS bs_b((*it).get_last_price(), b.K, .3, RISK_FREE_RATE, b.d, b.T / 365, "P");
			b.ivol = bs_b.impvol(.5, b.Mid);



			// set expiration days here
			(*it).set_days_until_exp(int(a.T));

			vector<double> garch_results;
			for (int i = 0; i < MC_ITERATIONS; ++i) {
				garch_results.push_back((*it).vol_forecast((*it).get_days_until_exp(), NM, i));
			}

			double average_vol = accumulate(garch_results.begin(), garch_results.end(), 0.0) / garch_results.size();
			cout << "Forecasted vol in " << (*it).get_days_until_exp() << " days: " << average_vol << endl;
			tuple<double, double> ci = (*it).conf_int(garch_results, average_vol);
			cout << "95% confidence interval of " << get<0>(ci) << " " << get<1>(ci) << endl;

			(*it).set_forecasted_vol(average_vol);
			(*it).set_ci_low(get<0>(ci));
			(*it).set_ci_high(get<1>(ci));
				
	
			// saves most liquid option, either call or put
			if (a.ivol == -1 && b.ivol == -1) 
				throw "cannot calculate implied vol";
			else if (a.ivol == -1 && b.Mid > 0.115)
				ml = b;
			else if (b.ivol == -1 && a.ivol != -1)
				ml = a;
			else if (fabs(a.ivol - average_vol) < fabs(b.ivol - average_vol) && b.Mid > 0.115) 
				// checks difference from forecasted vol, and checks that put price is greater than .115
				ml = b;
			else if (fabs(a.ivol - average_vol) > fabs(b.ivol - average_vol) && a.ivol != -1)
				ml = a;
			else
				throw "no valid options";

			BS bs_ml((*it).get_last_price(), ml.K, (*it).get_vol(), RISK_FREE_RATE, ml.d, ml.T / 365, string(1, ml.type));
			cout << ml.type << " delta: " << bs_ml.Delta(ml.type) << endl;

			// saves all relevant data to a struct
			save_data sd;
			sd.call_or_put = ml.type;
			sd.ci_high = (*it).get_ci_high();
			sd.ci_low = (*it).get_ci_low();
			sd.days_until_exp = ml.T;
			sd.delta = bs_ml.Delta(ml.type);
			sd.exp_date = to_string(month) + "/" + to_string(day) + "/" + to_string(year);
			sd.forecasted_vol = (*it).get_forecasted_vol();
			sd.implied_vol = ml.ivol;
			// after hours, yahoo finance shows 0 for the bid and ask
			// if this happens, we use last price
			if (ml.Mid != 0)
				sd.option_price = ml.Mid;
			else
				sd.option_price = ml.Last;
			sd.strike = ml.K;
			sd.ticker = (*it).get_ticker();
			sd.volume = ml.v;
			sd.vol_diff = (*it).get_forecasted_vol() - ml.ivol;

			data_list.push_back(sd);

			cout << endl;


			++it;
			++it2;
		}
		catch (const std::exception& ex)
		{
			// specific handling for all exceptions extending std::exception, except
			// std::runtime_error which is handled explicitly
			std::cerr << "Error occurred: " << ex.what() << std::endl;
			cout << endl;
			it = stock_list.erase(it);
			it2 = option_list.erase(it2);
		}
		catch (const char* msg) {
			// should be catching 'not enough data' from 90 day average
			cerr << (*it).get_ticker() << ": " << msg << endl;
			cout << endl;
			it = stock_list.erase(it);
			it2 = option_list.erase(it2);
		}
		catch (...) {
			cout << "generic error with " << (*it).get_ticker() << endl;
			cout << endl;
			it = stock_list.erase(it);
			it2 = option_list.erase(it2);
		}
		
	}


	sort(data_list.begin(), data_list.end(), compareStruct);

	ofstream out_file("volatility.csv");
	if (!out_file.is_open()) {
		cerr << "failed to open volatility.csv\n";
		return -1;
	}

	out_file << "ticker,date,call_or_put,strike,option_price_mid,forecasted_vol,ci_high,ci_low,implied_vol,vol_diff,delta,days_until_exp,volume" << endl;

	// saves all of data to a csv file
	for (auto it = data_list.begin(); it != data_list.end(); ++it) {
		out_file << (*it).ticker << "," << (*it).exp_date << "," << (*it).call_or_put << "," << (*it).strike <<
			"," << (*it).option_price << "," << (*it).forecasted_vol << "," <<
			(*it).ci_high << "," << (*it).ci_low << "," << (*it).implied_vol <<
			"," << (*it).vol_diff << "," << (*it).delta << "," << (*it).days_until_exp << "," << (*it).volume << endl;


		// prints top 10 volatility spreads to screen with delta neutral strategy
		if (it < data_list.begin() + 10) {
			cout << (*it).ticker << endl;
			if ((*it).vol_diff > 0 && (*it).call_or_put == 'C') {
				cout << "Buy n of the option " << (*it).ticker << " C " << (*it).exp_date << " strike= " << (*it).strike <<
					" and short " << fabs((*it).delta) * 100 << " *n shares of the underlying." << endl;
			}
			else if ((*it).vol_diff < 0 && (*it).call_or_put == 'C') {
				cout << "Short n of the option " << (*it).ticker << " C " << (*it).exp_date << " strike= " << (*it).strike <<
					" and buy " << fabs((*it).delta) * 100 << " *n shares of the underlying." << endl;
			}
			else if ((*it).vol_diff > 0 && (*it).call_or_put == 'P') {
				cout << "Buy n of the option " << (*it).ticker << " P " << (*it).exp_date << " strike= " << (*it).strike <<
					" and buy " << fabs((*it).delta) * 100 << " *n shares of the underlying." << endl;
			}
			else if ((*it).vol_diff < 0 && (*it).call_or_put == 'P') {
				cout << "Short n of the option " << (*it).ticker << " P " << (*it).exp_date << " strike= " << (*it).strike <<
					" and Short " << fabs((*it).delta) * 100 << " *n shares of the underlying." << endl;
			}
		}
	}
	

	in_file.close();
	out_file.close();

	return 0;
}