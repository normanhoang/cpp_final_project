
#include <iostream>
#include <vector>
#include <string>
#include <curl/curl.h>
#include <sstream>
#include <numeric>
#include <math.h>
#include "option.h"
#include <map>
#include <cmath>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>

namespace bd = boost::gregorian;
namespace pt = boost::posix_time;


using namespace std;





static int WriteCallback(void *contents, int size, int nmemb, void *userp)
{
	((string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

//initializer
Option::Option(string _ticker){
	ticker          = _ticker;
	//expiration_dates = get_expiration_dates(ticker);
}

Option::~Option() {}

boost::gregorian::date to_bdate(time_t time) {
	return boost::posix_time::from_time_t(time).date();
}

const std::locale fmt(std::locale::classic(),
                      new boost::gregorian::date_facet("%m/%d/%Y"));

//convert date to string
std::string to_str( const boost::gregorian::date& date )
{
    std::ostringstream os;
    os.imbue(fmt);
    os << date;
    return os.str();
}

//convert date to time stamp

time_t timestamp(const boost::gregorian::date& date ){
	using namespace boost::posix_time;
	static ptime epoch(boost::gregorian::date(1970, 1, 1));
	time_duration::sec_type secs = (ptime(date,seconds(0)) - epoch).total_seconds();
	return time_t(secs);
}


void Option::get_urls(){
	CURL *curl;          // 
 	CURLcode res;       //Define these in function
 	string readBuffer; //Body 
 	string readline;  //Individual lines to parse through
 	string url ="https://finance.yahoo.com/quote/"+ticker+"/options?p="+ticker+"&.tsrc=fin-srch";
 	//cout << url << endl;
 	curl = curl_easy_init();
 	string inpStr = "not timestamp";
	vector<int> info; //vector to store all the info from the web

	
	if (curl) {
  		curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); //setting preferred url transfer
  		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); //function to take care of the data
  		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
  		res = curl_easy_perform(curl); //transfer connects to site
  
  
  		//vector<int> info; //vector to store all the info from the web
  		size_t pos  = 0; //used in finding delimiter in readline
  		size_t end  = 0; //used to reduce total textfile size from original download
  		size_t end2 = 0; //used to reduce total textfile size form original download


  		string delim="<div class=\"Fl(start) Pend(18px)";
 
  		end = readBuffer.find(delim);
  		readBuffer.erase(0, end + delim.length());
 
  		end  = readBuffer.find("<div class=\"Fl(start) Mend(18px)");
  		end2 = readBuffer.find_last_of(";");
  		readBuffer.erase(end, end2);
	
  

  		stringstream ss(readBuffer);
  
  		//parsing the data 
  		while(getline(ss, readline, '<')) {
   			delim = "value=\"";
   			pos = readline.find(delim);
   			readline.erase(0,pos+delim.length());
   
   			try{
  				info.push_back(stoi(readline.substr(0,10)));
  				if(!stoi(readline.substr(0,10))){
					throw inpStr;
				}
			}
			catch(invalid_argument g){
			}
  		}
  		curl_easy_cleanup(curl);   
  		                  
 	}
 	curl_global_cleanup();
	
	
	for(int i=0; i<int(info.size()); ++i){
		urls.push_back("https://finance.yahoo.com/quote/"+ticker+"/options?p="+ticker+"&.tsrc=fin-srch&date=" +to_string(info[i]));
		time_stamps.push_back(info[i]);
	}
	
	  
}

void Option::get_urls(int year, int month, int day) {

	bd::date d1(year, month, day);
	time_t ts = timestamp(d1);
	urls.push_back("https://finance.yahoo.com/quote/" + ticker + "/options?p=" + ticker + "&.tsrc=fin-srch&date=" + to_string(ts));
	time_stamps.push_back(int(ts));

}

void Option::get_urls(int year[], int month[], int day[], int num){
	
	for(int i=0; i<num; ++i){
		bd::date d1(year[i], month[i], day[i]);
		time_t ts =  timestamp(d1);
		urls.push_back("https://finance.yahoo.com/quote/"+ticker+"/options?p="+ticker+"&.tsrc=fin-srch" + to_string(ts));
		time_stamps.push_back(int(ts));
	
	}

}






//web scraper takes all relevant data of web page and stores it in class
void Option::init_current(){
for(int k=0; k<int(urls.size()); ++k){
	//initialize expiration date
	bd::date expiration(to_bdate(time_stamps[k]));
	string expiration_date = to_str(expiration);
	expirations.push_back(expiration_date);
	OptionData* temp = new OptionData;
	
	CURL *curl;          // 
 	CURLcode res;       //Define these in function
 	string readBuffer; //Body 
 	string readline;  //Individual lines to parse through
	//string url = "https://finance.yahoo.com/quote/"+ticker+"/options?p="+ticker+"&.tsrc=fin-srch&date="+time_stamp;
	
	curl = curl_easy_init(); //curl handle
	
	if (curl) {
		
  		curl_easy_setopt(curl, CURLOPT_URL, urls[k].c_str()); //setting preferred url transfer
  		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); //function to take care of the data
  		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
  		res = curl_easy_perform(curl); //transfer connects to site
  
  
  		vector<string> info; //vector to store all the info from the web
  		size_t pos  = 0; //used in finding delimiter in readline
  		size_t end  = 0; //used to reduce total textfile size from original download
  		size_t end2 = 0; //used to reduce total textfile size form original download


  		string delim;   //finding certain points in readBuffer string
  
  		//next six lines get rid of irrelevant web data to make parsing more streamlined
  		delim = "<td class=\"data-col0 Ta(start) Pstart(10px)\"";
  		end = readBuffer.find(delim);
  		readBuffer.erase(0, end + delim.length());
  
  		end  = readBuffer.find("window.performance");
  		end2 = readBuffer.find_last_of(";");
  		readBuffer.erase(end, end2);

  

  		stringstream ss(readBuffer);
  
  		bool relData = true; //used to control what data is stored in vectors 
		



		while(getline(ss, readline, '<')) {
   			//get location right before data 
   			delim = "data-reactid=";
   			pos   = readline.find(delim);
   			readline.erase(0, pos + delim.length());
   			delim = ">";
   			pos  = readline.find(delim);
   			readline.erase(0, pos + delim.length());
   			//dont want any of the data between these two entries (Puts, Implied Volatility)
   			if (readline=="Puts"){
   				relData = false;
   				
   			}
   			if (readline=="Implied Volatility"){
   				relData = true;
   				
   				continue;
   			}
   			//only add relevant data skips spaces that dont hold data 
   			if (readline.length()>0){
   				if(relData == true){
   					info.push_back(readline);
   					
   				}
   			}
   	
  
  
  		}
  		curl_easy_cleanup(curl);
		string inpStr       = "No conversion of volume to int for: "; // if the volume entry is 0
  		number_contracts.push_back(info.size()/11);                         // finding the amount of options we scraped
  		
 
  		while(info.size()>0){
  		
  			//erasing commas from number strings
  			boost::erase_all(info[2], ","); 
  			boost::erase_all(info[8], ",");
  			boost::erase_all(info[9], ","); 
  			boost::erase_all(info[3], ","); 
  			boost::erase_all(info[4], ","); 
  			boost::erase_all(info[5], ","); 
  			boost::erase_all(info[9], ","); 
  			//assigning values
  			temp->contract_name.push_back(info[0]);
  			temp->last_trade_date.push_back(info[1]);
  			
  			temp->strike.push_back(stof(info[2]));
  			temp->last_price.push_back(stof(info[3]));
  			temp->bid.push_back(stof(info[4]));
  			temp->ask.push_back(stof(info[5]));
  			temp->change.push_back(info[6]);
  			temp->percent_change.push_back(info[7]);
  			//some inputs on yahoo finance for volume are null or non convertible "-" 
  			//so we convert these values to 0
  			 
  			try{
  				temp->volume.push_back(stoi(info[8]));
  				if(!stoi(info[8])){
					throw inpStr;
				}
			}
			//catch(invalid_argument g){
				//temp->volume.push_back(0);  
			//}
			//catch all other errors
			catch (...){
				while (temp->volume.size() < temp->last_price.size())
					temp->volume.push_back(0);
	
			}
			try{	
				temp->open_interest.push_back(stoi(info[9]));
				if(!stoi(info[9])){
					throw inpStr;
				}
				
			}
			catch(invalid_argument g){
				temp->open_interest.push_back(0);
			}
			catch(...){
				temp->open_interest.push_back(0);
			}
  			try{
  				temp->implied_vol.push_back(stof(info[10].substr(0, info[10].length()-1))/100);
  				if(!stof(info[10])){
					throw inpStr;
				}
				
  			}
  			catch(...){
  				temp->implied_vol.push_back(0);
  			
  			}
  			
  			info.erase(info.begin(), info.begin()+11);
 
  		
  		}
  		
  		options.insert(pair<string, OptionData>(expiration_date, *temp));
  		delete temp;
 	}
 	
 	curl_global_cleanup();


}


}




void Option::set_expiration_dates() {
	CURL *curl;          // 
	CURLcode res;       //Define these in function
	string readBuffer; //Body 
	string readline;  //Individual lines to parse through
	string url = "https://finance.yahoo.com/quote/" + ticker + "/options?p=" + ticker + "&.tsrc=fin-srch";
	//cout << url << endl;
	curl = curl_easy_init();
	string inpStr = "not timestamp";
	vector<int> info; //vector to store all the info from the web

	try {
		if (curl) {
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); //setting preferred url transfer
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); //function to take care of the data
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
			res = curl_easy_perform(curl); //transfer connects to site


										   //vector<int> info; //vector to store all the info from the web
			size_t pos = 0; //used in finding delimiter in readline
			size_t end = 0; //used to reduce total textfile size from original download
			size_t end2 = 0; //used to reduce total textfile size form original download


			string delim = "<div class=\"Fl(start) Pend(18px)";

			end = readBuffer.find(delim);
			readBuffer.erase(0, end + delim.length());

			end = readBuffer.find("<div class=\"Fl(start) Mend(18px)");
			end2 = readBuffer.find_last_of(";");
			readBuffer.erase(end, end2);



			stringstream ss(readBuffer);

			//parsing the data 
			while (getline(ss, readline, '<')) {
				delim = "value=\"";
				pos = readline.find(delim);
				readline.erase(0, pos + delim.length());

				try {
					info.push_back(stoi(readline.substr(0, 10)));
					if (!stoi(readline.substr(0, 10))) {
						throw inpStr;
					}
				}
				catch (invalid_argument g) {
				}
			}
			curl_easy_cleanup(curl);

		}
		curl_global_cleanup();
		expiration_dates = info;
	}
	catch (...) {
		throw "not a valid ticker";
	}
}


void Option::div_yield(){
	string url = "https://finance.yahoo.com/quote/"+ticker+"?p="+ticker+"&.tsrc=fin-srch";
	float div_yield1;
 	CURL *curl;          // 
 	CURLcode res;       //Define these in function
 	string readBuffer; //Body 
 	string readline;  // 
 	string stock;

	
 
 	//cout << url << endl;
 	curl = curl_easy_init(); //curl handle
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); //setting preferred url transfer
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); //function to take care of the data
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl); //transfer connects to site


		vector<string> info; //vector to store all the info from the web
		size_t pos = 0; //used in finding delimiter in readline
		size_t end = 0; //used to reduce total textfile size from original download
		size_t end2 = 0; //used to reduce total textfile size form original download

		string inpStr = "invalid not a float";
		string delim = "Forward Dividend";
		end = readBuffer.find(delim);
		readBuffer.erase(0, end + delim.length());

		end = readBuffer.find("Ex-Dividend Date");
		end2 = readBuffer.find_last_of(";");
		readBuffer.erase(end, end2);



		stringstream ss(readBuffer);


		//parsing the data 
		while (getline(ss, readline, '<')) {
			delim = ">";
			pos = readline.find(delim);
			readline.erase(0, pos + delim.length());


			if (readline.length() > 0) {
				info.push_back(readline);

			}

		}
		curl_easy_cleanup(curl);

		if (info.size() <= 1) {
			throw  "invalid div";
		}
  	
  	
   		try{
  			delim = "(";
  			pos = info[1].find(delim);
  			info[1].erase(0,pos+delim.length());
  	
  			div_yield1 =stof(info[1].substr(0,info[1].length()-2))/100;
  			
  			if(!stof(info[1].substr(0,info[1].length()-2))){
				throw inpStr;
			}
		}
		catch(invalid_argument g){
			div_yield1 = 0;
		}
  	
  
  
	}
 	curl_global_cleanup();
 	
 	for(int i=0; i<int(expirations.size()); ++i){
 		options[expirations[i]].dividend_yield = div_yield1;
 	}
 	

 


}



vector <string> Option::get_expirations(){
	
	return expirations;


}


void Option::print(vector <string> expiration){
	OptionData* temp = new OptionData;//options[expiration];
	for(int j=0; j<int(expiration.size()); ++j){
		
	
		temp = &options[expiration[j]];
	
		for(int i=0; i<number_contracts[j]; ++i){
		
		
			cout << temp->contract_name[i] << ' ' << temp->last_trade_date[i] << ' ' << temp->strike[i]
		    	 << ' ' << temp->last_price[i] << ' ' << temp->bid[i] << ' ' << temp->ask[i] << ' '
		     	<< temp->change[i] << ' ' << temp->percent_change[i] << ' ' << temp->volume[i]
		     	<< ' ' << temp->open_interest[i] << ' ' << temp->implied_vol[i] << ' ' 
		     	<< temp->call_or_put[i] << ' ' << temp ->time_to_maturity << ' ' << temp->dividend_yield << endl;
		}
		
		cout << "\n\n";
	}
	
}





void Option::set_type(){
	
	
	int pos = ticker.length() + 6;
	for(int j=0; j<int(expirations.size()); ++j){
		
		for(int i=0; i<number_contracts[j]; ++i){
			
			options[expirations[j]].call_or_put.push_back(options[expirations[j]].contract_name[i].substr(pos)[0]);
			
		}
	}
}

void Option::set_ttm(){
	time_t nowtime;
	time(&nowtime);
	int td;
	for(int i=0; i<int(time_stamps.size()); ++i){
		td = time_stamps[i] - int(nowtime);
		td = td/60/60/24;
		options[expirations[i]].time_to_maturity = td;
	
	}

}


MostLiquid Option::get_most_liquid(char type, string expiration){
	int index = 0;
	int size = min(options[expiration].volume.size(), options[expiration].call_or_put.size());
	//cout << options[expiration].volume.size() << " " << options[expiration].call_or_put.size() << endl;
	//cout << options[expiration].call_or_put.size() << endl;
	int volume1 = 0;
	
	MostLiquid a;
	
	if (size > 0) {

		if (type == 'P') {
			for (int i = 0; i<size; ++i) {
				if (options[expiration].call_or_put[i] == 'P') {
					if (options[expiration].volume[i] > volume1) {
						volume1 = options[expiration].volume[i];
						index = i;

					}
				}
			}
		}
		if (type == 'C') {
			for (int i = 0; i<size; ++i) {
				if (options[expiration].call_or_put[i] == 'C') {
					if (options[expiration].volume[i] > volume1) {
						volume1 = options[expiration].volume[i];
						index = i;

					}
				}
			}
		}


		a.K = options[expiration].strike[index];
		a.v = options[expiration].volume[index];
		a.d = options[expiration].dividend_yield;
		a.T = float(options[expiration].time_to_maturity);
		a.Mid = (options[expiration].bid[index] + options[expiration].ask[index]) / 2;
		a.Last = options[expiration].last_price[index];
		a.type = type;

		return a;

	}
	else
		throw "no entries";

}
