#include <string>
#include <string.h>
#include <errno.h>
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

const char* ClientErrStr(int code) {
	switch(code) {
	case CLIENT_OK:
		return "OK";
	case CLIENT_ERR_RESOURCE_INIT:
		return "resource init failed";
	case CLIENT_ERR_OUT_OF_MEMORY:
		return "out of memory";
	case CLIENT_ERR_AGAIN:
		return "client resource temporarily unavaibale";
	case CLIENT_ERR_REQUEST_EXIST:
		return "request already exist";
	case CLIENT_ERR_INVALID_PARAMTER:
		return "invalid paramter";
	case CLIENT_ERR_STOP:
		return "client has stop";
	default:
		return "unknown error";
	}
}

const char* ErrStr(int code) {
	if(is_easy_code(code)) {
		return curl_easy_strerror((CURLcode)resolv_easy_code(code));
	} else if(is_multi_code(code)) {
		return curl_multi_strerror((CURLMcode)resolv_multi_code(code));
	} else if(is_share_code(code)) {
		return curl_share_strerror((CURLSHcode)resolv_share_code(code));
	} else if(is_errno_code(code)) {
		return strerror(resolv_errno_code(code));
	} else {
		return ClientErrStr(code);
	}
}

}