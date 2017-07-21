#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>
#include "response.h"
#include "util.h"

using namespace std;

namespace http {

Response::Response(bool write_body)
	:status_code_(0), write_body_(write_body) {

}

size_t Response::ContentLength() {
	string clstr = header_.Get("Content-Length");
	return strtoul(clstr.c_str(), NULL, 0);
}
	
string Response::ContentType() {
	return header_.Get("Content-Type");
}

void Response::GetBody(string* body) {
	body->assign(body_);
}

const char* Response::GetBody() {
	return body_.data();
}
	
size_t Response::BodySize() {
	return body_.size();
}

size_t Response::parseFirstLine(char* buffer, size_t size) {
	char* cr = buffer;
	bool flag = false;
	while((cr-buffer) <= (size-CRLF_LEN)) {
		if(strncmp(cr, CRLF, CRLF_LEN) == 0) {
			flag = true;
			break;
		}
		++cr;
	}
	if(!flag) return 0;
	int n = cr - buffer + CRLF_LEN;
	string code_str;
	//state machine:
	//0: http version
	//1: remove space
	//2: status code
	//3: remove space
	//4: reason phrase
	int state = 0;
	while(buffer < cr) {
		// if(IsSpace(*buffer)) {
		// 	switch(state) {
		// 	case 0:
		// 	case 2:
		// 		++state;
		// 		break;
		// 	}
		// } else {
		// 	switch(state) {
		// 	case 0:
		// 		http_version_.append(1, *buffer);
		// 		break;
		// 	case 1:
		// 		code_str.append(1, *buffer);
		// 		++state;
		// 		break;
		// 	case 2:
		// 		code_str.append(1, *buffer);
		// 		break;
		// 	case 3:
		// 		reason_phrase_.append(1, *buffer);
		// 		++state;
		// 		break;
		// 	case 4:
		// 		reason_phrase_.append(1, *buffer);
		// 		break;
		// 	}
		// }
		switch(state) {
		case 0:
			if(!IsSpace(*buffer)) {
				http_version_.append(1, *buffer);
			} else {
				++state;
			}
			break;
		case 1:
			if(!IsSpace(*buffer)) {
				code_str.append(1, *buffer);
		 		++state;
			}
			break;
		case 2:
			if(!IsSpace(*buffer)) {
				code_str.append(1, *buffer);
			} else {
				++state;
			}
			break;
		case 3:
			if(!IsSpace(*buffer)) {
				reason_phrase_.append(1, *buffer);
		 		++state;
			}
			break;
		case 4:
			reason_phrase_.append(1, *buffer);
			break;
		}
		++buffer;
	}
	status_code_ = atoi(code_str.c_str());
	return n;
}

size_t Response::headerCallback(char *buffer, size_t size, size_t nitems, void *userdata) {
	size_t datasize = size*nitems;
	Response* me = reinterpret_cast<Response*>(userdata);
	if(me->status_code_ == 0) {
		return me->parseFirstLine(buffer, datasize);
	}
	me->header_>>string(buffer, datasize);
	return datasize;
}

size_t Response::writeCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
	Response* me = reinterpret_cast<Response*>(userdata);
	size_t datasize = size*nmemb;
	me->body_.append(ptr, datasize);
	return datasize;
}

string Response::ToString() {
	ostringstream oss;
	oss<<"{Version:"<<http_version_<<" StatusCode:"<<status_code_
		<<" ReasonPhrase:"<<reason_phrase_<<" Header:{"<<header_.ToString(", ")
		<<"} Body:"<<body_<<"}";
	return oss.str();
}

}