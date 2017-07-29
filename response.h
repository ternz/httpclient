#ifndef __RESPONSE_H__
#define __RESPONSE_H__

#include <string>
#include "header.h"

namespace http {

class Client;

class Response {
public:
	friend class Client;

	explicit Response(bool write_body=true);

	Header GetHeader() {return header_;} //返回header拷贝
	std::string GetHeaderValue(const std::string& key){
		return header_.Get(key);
	}
	bool HasHeader(const std::string& key) {
		return  header_.Key(key);
	}

	std::string GetVersion() {return http_version_;}
	int GetStatusCode() {return status_code_;}
	std::string GetReasonPhrase() {return reason_phrase_;}

	size_t ContentLength();
	std::string ContentType();

	void GetBody(std::string* body);
	const char* GetBody();
	size_t BodySize();

	std::string ToString();

private:
	Header* getHeader() {
		return &header_;
	}
	size_t parseFirstLine(char* buffer, size_t size);
	static size_t headerCallback(char *buffer,   size_t size,   size_t nitems,   void *userdata);
	static size_t writeCallback(char *ptr, size_t size, size_t nmemb, void *userdata);
private:
	Header header_;
	std::string http_version_;
	int status_code_;
	std::string reason_phrase_;
	std::string body_;
	bool write_body_;
};

}

#endif