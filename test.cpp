#include <iostream>
#include <string>
#include <curl/curl.h>
#include <sstream>

using namespace std;


static int WriteCallback(void *contents, int size, int nmemb, void *userp)
{
	((string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

int main()
{
	CURL *curl;
	CURLcode res;
	string readBuffer;
	string readline;

	string stock;

	cout << "Please enter a stock ticker: ";
	cin >>  stock;

	string url = "https://api.iextrading.com/1.0/stock/" + stock + "/chart/1y";
	cout << url << endl;
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		


		size_t pos = 0;
		string delim;
		string date, close;
		stringstream ss(readBuffer);
		while(true) {
			getline(ss, readline, '}');
			if (readline == "]")
				break;
			delim = "\"date\":";
			pos = readline.find(delim);
			readline.erase(0, pos + delim.length());
			pos = readline.find(",");
			date = readline.substr(0, pos);
			date = date.substr(1, date.length() - 2);

			delim = "\"close\":";
			pos = readline.find(delim);
			readline.erase(0, pos + delim.length());
			pos = readline.find(",");
			close = readline.substr(0, pos);

			cout << date << " " << close << endl;
		}
		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();
	return 0;
}