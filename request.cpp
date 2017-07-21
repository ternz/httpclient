#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sstream>
#include <curl/curl.h>
#include "define.h"
#include "request.h"
#include "util.h"

using namespace std;

namespace http {

Request::Request(Method m, const string& url)
	:method_(m), code_(CURLE_OK), reader_(NULL), read_pos_(0), chunked_(false), read_size_(0) {
	curl_handle_ = curl_easy_init();
	SetUrl(url);
}

Request::~Request() {
	if(reader_ != NULL) {
		delete reader_;
	}
	if(curl_handle_ != NULL) {
		curl_easy_cleanup(curl_handle_);
	}
}

void Request::SetUrl(const std::string& url) {
	size_t pos = url.find_first_of('?');
	if(pos != string::npos) {
		url_ = url.substr(0, pos);
		form_ = url.substr(pos+1);
	} else {
		url_ = url;
		form_.clear();
	}
}

void Request::SetFormParameter(const std::string& key, const std::string& value) {
	if(!form_.empty()) {
		form_.append("&");
	}
	form_.append(key+"="+value);
}

void Request::Reset() {
	method_ = GET;
	url_.clear();
	header_.Reset();
	form_.clear();
	data_.clear();
	read_pos_ = 0;
	read_size_=0;
	chunked_=false;
	if(reader_ != NULL) {
		delete reader_;
		reader_ = NULL;
	}
	curl_easy_reset(curl_handle_);
	CURLcode code_ = CURLE_OK;
}

bool Request::isReadData() {
	if(data_.empty() && reader_ == NULL)
		return false;
	return true;
}

void Request::setReadData() {
	if(!form_.empty()) {
		form_.assign(curl_easy_escape(curl_handle_, form_.c_str(), form_.size()));
		curl_easy_setopt(curl_handle_, CURLOPT_POSTFIELDS, form_.c_str());
		curl_easy_setopt(curl_handle_, CURLOPT_POSTFIELDSIZE, form_.size());
	}
	if(isReadData()) {
		curl_easy_setopt(curl_handle_, CURLOPT_READFUNCTION, readCallback);
		curl_easy_setopt(curl_handle_, CURLOPT_READDATA, this);
	}
	if(chunked_) {
		header_.Set("Transfer-Coding", "chunked");
	}
}

size_t Request::readCallback(char *buffer, size_t size, size_t nmemb, void *userp) {
	Request* me = reinterpret_cast<Request*>(userp);
	size_t n = size*nmemb;
	if(!me->data_.empty()) {
		if(me->read_pos_ >= me->data_.size()) {
			me->read_pos_ = 0;
			return 0;
		}
		size_t remain = me->data_.size() - me->read_pos_;
		size_t readsize = remain > n ? n : remain;
		memcpy(buffer, me->data_.data(), readsize);
		me->read_pos_ += readsize;
		return readsize;
	}
	if(me->reader_ != NULL) {
		return me->reader_->Read(buffer, n);
	}
	fprintf(stderr, "In request object %d, no data to read in readCallback\n", me);
	return 0;
}

int Request::Prepare() {
	if(curl_handle_ == NULL) {
		curl_handle_ = curl_easy_init();
		if(curl_handle_ == NULL) {
			code_ = CURLE_FAILED_INIT;
			return code_;
		}
	}
	curl_easy_setopt(curl_handle_, CURLOPT_URL, url_.c_str());
	switch(method_) {
	case HEAD:
		curl_easy_setopt(curl_handle_, CURLOPT_CUSTOMREQUEST, M_HEAD);
		break;
	case POST:
		curl_easy_setopt(curl_handle_, CURLOPT_POST, 1L);
		setReadData();
		break;
	case PUT:
		curl_easy_setopt(curl_handle_, CURLOPT_PUT, 1L);
		setReadData();
		break;
	case DELETE:
		curl_easy_setopt(curl_handle_, CURLOPT_CUSTOMREQUEST, M_DELETE);
		break;
	//default:
		//curl_easy_setopt(curl_handle_, CURLOPT_HTTPGET, 1L);
	}
	//set headers
	if(header_.ItemSize() > 0) {
		struct curl_slist *list = NULL;
		string item;
		for(Header::Iterator it = header_.Begin(); it != header_.End(); ++it) {
			item.assign(it->first+":"+it->second);
			list = curl_slist_append(list, item.c_str());
		}
		curl_easy_setopt(curl_handle_, CURLOPT_HTTPHEADER, list);
		curl_slist_free_all(list);
	}

	code_ = CURLE_OK;
	return code_;
}

string Request::ToString() {
	ostringstream oss;
	oss<<"{Method:"<<MethodStr(method_)<<" Url:"<<url_<<" Form:"<<form_
		<<" Header:{"<<header_.ToString(", ")<<"} Body:"<<data_<<" }";
	return oss.str();
}

}