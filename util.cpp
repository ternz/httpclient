#include <string>
#include "util.h"

using namespace std;

namespace http {

void UpperChar(char& c) {
	if(c >= 'a' && c <= 'z') 
		c -= 'a' - 'A';
}

void LowerChar(char& c) {
	if(c >= 'A' && c <= 'Z')
		c += 'a' - 'A';
}

std::string KeyFormat(std::string key) {
	if(key.empty()) 
		return key;
	string::iterator it = key.begin();
	UpperChar(*it);
	while(++it != key.end()) {
		if(*(it-1) == '-')
			UpperChar(*it);
		else
			LowerChar(*it);
	}
	return key;
}

}