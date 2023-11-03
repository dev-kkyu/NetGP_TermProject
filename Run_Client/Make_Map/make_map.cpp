#include <iostream>
#include <fstream>
#include <string>
#include <random>

using namespace std;

random_device rd;
default_random_engine dre{ rd() };

constexpr int MAX_LENGTH = 100;

int main()
{
	uniform_int_distribution<int> uid{ 0, 2 };

	ofstream out{ "map" + to_string(time(NULL)) + ".txt" };

	for (int i = 0; i < MAX_LENGTH; ++i) {
		for (int j = 0; j < 16; ++j) {
			out << !!uid(dre) << ' ';
		}
		out << endl;
	}

}