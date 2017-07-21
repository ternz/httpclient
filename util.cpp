#include <string>
#include "util.h"
#include "request.h"
#include "response.h"

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

bool IsSpace(char c) {
	if(c == ' ' || c == '\t' || c == '\v' /*|| c == '\r' || c == '\n'*/)
		return true;
	return false;
}

const char* MethodStr(Method m) {
	switch(m) {
	case GET:
		return M_GET;
		break;
	case HEAD:
		return M_HEAD;
		break;
	case POST:
		return M_POST;
		break;
	case PUT:
		return M_PUT;
		break;
	case DELETE:
		return M_DELETE;
		break;
	default:
		return "UNKNOWN";
	}
}

Context::~Context() {
	if(request_ != NULL && crc_ != NULL) {
		(*crc_)(&request_);
	}
	freeResponse();
	if(parg_ != NULL && cac_ != NULL) {
		(*cac_)(&parg_);
	}
}

Response* Context::allocResponse(bool write_body) {
	if(response_ == NULL)
		response_ = new Response(write_body);
	return response_;
}
void Context::freeResponse() {
	if(response_ != NULL) 
		delete response_;
}

}