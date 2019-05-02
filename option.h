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

namespace bd = boost::gregorian;
namespace pt = boost::posix_time;

using namespace std;

//used to house all the data for certain expirations
struct OptionData{
	vector <string> contract_name;
	vector <string> last_trade_date;
	vector <string> change;
	vector <string> percent_change;
	vector <float>  implied_vol;
	vector <float>  strike;
	vector <float>  last_price;
	vector <float>  bid;
	vector <float>  ask;
	vector <int>    volume;
	vector <int>    open_interest;
	vector <char>   call_or_put;
	int             time_to_maturity;
	float           dividend_yield;
};
//S,K,v,r,d,T Need price from norman and rate can be ignored
struct MostLiquid{
	float S;
	float K;
	int v;
	float d;
	float T;
	float Mid;
	char type;
	double ivol;
	float Last;
};


#ifndef OPTION_H
#define OPTION_H




class Option{

	private:
		
		map <string, OptionData> options;
		vector <string>          expirations;
		vector <int>             expiration_dates;
		vector <string>          urls;
		vector <int>	         time_stamps;
		vector <int>             number_contracts;
		string                   ticker;
		
	public:
	
		Option(string _ticker);
		~Option();
		
		void get_urls();
		void get_urls(int year, int month, int day);
		//void get_urls(int year, int month);
		void get_urls(int year[], int month[], int day[], int num);
		void init_current();
		
		//void print_urls();
		vector <string> get_expirations();
		void print(vector <string> expiration);
		void set_expiration_dates();
		//int expiration_in_range(int year, int month);
		//time_t checktime(int year, int month);

		//void print_time_stamps();
		void div_yield();
		void set_type();
		void set_ttm();
		MostLiquid get_most_liquid(char type, string expiration);
		//void write_to_csv();



};






#endif
