#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <string>
#include <curl/curl.h>
#include "define.h"
#include "util.h"
#include "header.h"

namespace http {

class Client;

class Request {
public:
	friend class Client;
	Request();
	Request(Method m, const std::string& url);
	Request(const Request& req);
	Request& operator=(const Request& req);
	~Request();

	void SetMethod(Method m) {method_ = m;}
	Method GetMethod() {return method_;}

	void SetUrl(const std::string& url);
	std::string GetUrl() {return url_;}
	std::string GetQuery() {return form_;}

	void SetTimeout(int timeout_s) {timeout_ms_ = timeout_s * 1000;}  //TODO:overflow?
	void SetTimeoutMs(int timeout_ms) {timeout_ms_ = timeout_ms;}

	void SetFormParameter(const std::string& key, const std::string& value);

	void SetHeader(Header& h) {header_ = h;}
	void UpdateHeader(Header& h) {header_.Update(h);}
	void SetHeaderValue(const std::string& key, const std::string& value) {header_.Set(key, value);}
	std::string GetHeaderValue(const string& key) {return header_.Get(key);}
	void ResetHeader() {header_.Reset();}

	void SetRequestReader(RequestReader* reader, bool chunked=true/*, size_t size=0*/) {
		reader_ = reader;
		chunked_ = chunked;
		//read_size_ = size;
	}

	void SetData(const string& data, bool chunked=false) {
		data_ = data; 
		read_pos_ = 0;
		chunked_ = chunked;
	}
	void SetData(const char* data, size_t size, bool chunked=false) {
		data_.assign(data, size);
		read_pos_ = 0;
		chunked_ = chunked;
	}

	int Prepare();

	const char* ErrStr(){
		return curl_easy_strerror(code_);
	}

	void Reset();

	string ToString();

private:
	void setReadData();
	bool isReadData();
	static size_t readCallback(char *buffer, size_t size, size_t nmemb, void *userp);

private:
	Method method_;
	std::string url_;
	Header header_;
	string form_;
	int timeout_ms_;

	string data_;  //data比request reader优先
	size_t read_pos_;
	RequestReader* reader_;
	size_t read_size_;  //no use!
	bool chunked_;

	CURL* curl_handle_;
	CURLcode code_;
};

}

#endif