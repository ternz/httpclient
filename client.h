#ifndef __CLIENT_H__
#define __CLIENT_H__
#include <curl/curl.h>

namespace http {

class Request;
class Response;
class ResponseHandler;
class ResponseStreamHandler;

// struct StreamWriteCallbackArg {
// 	Client* client;
// 	ResponseStreamHandler* handler;
// 	Context* context;
// };

class Client {
public:
	Client(unsigned short workers=0);
	int Sync(Request* req, Response* rsp);
	int Sync(Request* req, ResponseStreamHandler* handler, bool cleanUpHandler=true);
	int Async(Request* req, ResponseHandler* handler, bool cleanUpHandler=true);
	int Async(Request* req, ResponseStreamHandler* handler, bool cleanUpHandler=true);

	const char* ErrStr(int code);

private:
	static size_t streamWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata);

private:
	unsigned short workers_;
};

}

#endif