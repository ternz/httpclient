#ifndef __RESPONSE_H__
#define __RESPONSE_H__

#include <string>
#incluce "header.h"

namespace http {

class Response {
public:
	Response();

	//Header* GetHeader();
	std::string GetHeaderValue(const std::string& key);
	bool HasHeader(const std::string& key);

	sizt_t ContentLength();
	std::string ContentType();

	void GetData(string* data);
	cosnt char* GetData();
	size_t DataSize();

private:
	Header header_;
	std::string http_version_;
	int status_code_;
	std::string reason_phrase_;
	
};

}

#endif