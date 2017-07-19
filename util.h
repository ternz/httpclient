#ifndef __UTIL_H__
#define __UTIL_H__
#include <string>
namespace http {

inline void UpperChar(char& c);
inline void LowerChar(char& c);
std::string KeyFormat(std::string key);

class Request;
class Response;

typedef void (*ContextRequestCleaner)(Request**);
typedef void (*ContextArgCleaner)(void**);

class Context {
public:
	Context(Request* req=NULL, void* parg=NULL, ContextRequestCleaner *crc=NULL, ContextArgCleaner *cac=NULL);
	void SetRequest(Request* req, ContextRequestCleaner *crc=NULL);
	void SetArg(void* parg, ContextArgCleaner *cac=NULL);
	Request* GetRequest();
	Response* GetResponse();
	template<typename T> T* GetArg();
private:
	Request* request_;
	Response* response_;
	void* parg_;

	ContextRequestCleaner* crc_;
	ContextArgCleaner* cac_;
};

class RequestReader {
public:
	virtual size_t Read(char* buffer, size_t size) = 0;
	virtual ~RequestReader(){}
};

class ResponseHandler {
public:
	ResponseHandler(Context* cxt=NULL);
	virtual int Handle(Context* cxt) = 0;
	virtual ~ResponseHandler(){}
protected:
	Context* cxt_;
};

class ResponseStreamHandler {
public:
	ResponseStreamHandler(Context* cxt=NULL);
	virtual int Handle(Context* cxt, char* buffer, size_t size) = 0;
	virtual ~ResponseStreamHandler(){}
protected:
	Context* cxt_;
};

}
#endif