#include <iostream>
#include <string>
#include "util.h"

using namespace std;
using namespace http;

void TestKeyFormat() {
	string sa[3] = {"content-type", "aUth-x-token", "A123_bcD-1ef-ghi"};
	for(int i=0; i < 3; ++i) {
		cout<<sa[i]<<" -> "<<KeyFormat(sa[i])<<endl;
	}
}

int main(int argc, char* argv[]) {
	TestKeyFormat();
}