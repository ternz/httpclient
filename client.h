#ifndef __CLIENT_H__
#define __CLIENT_H__
#include <curl/curl.h>
#include <map>

namespace http {

class Request;
class Response;
class ResponseHandler;
class ResponseStreamHandler;

struct PipeData {
	enum HandlerType{nonstream, stream};
	Request* request;
	HandlerType handlerType;
	union {
		ResponseHandler* nonstream;   //handlerType:0
		ResponseStreamHandler* stream;  //handlerType:1
	}handler;
	bool cleanUpHandler;
};

static inline PipeData* newPipeData(Request* req, PipeData::HandlerType type, void* handler, bool cleanUpHandler);
static inline void freePipeData(PipeData* pd);
static void freePipeDataAndHandler(PipeData* pd);
int addHandler(CURLM* cm, std::map<CURL*, PipeData*>& m, CURL* e, PipeData* pd);
int removeHandler(CURLM* cm, std::map<CURL*, PipeData*>& m, CURL* e);

class Client {
public:
	Client(unsigned short workers=0);
	~Client();
	int Init();
	int Sync(Request* req, Response* rsp);
	int Sync(Request* req, ResponseStreamHandler* handler, bool cleanUpHandler=true);
	int Async(Request* req, ResponseHandler* handler, bool cleanUpHandler=true);  //不能在未确定request完成前再次使用同样的request参数调用Async
	int Async(Request* req, ResponseStreamHandler* handler, bool cleanUpHandler=true);
	int Wait(int timeout_ms);

	static const char* ErrStr(int code);
	const char* ErrStr();
	int ErrCode();

private:
	class SyncWorker;

private:
	Client(Client& c); //disable copy
	Client& operator=(const Client& c);
	static size_t streamWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata);
	int notifyRequest(PipeData* pd);
	int workerIdx();

private:
	bool is_init_;
	int errcode_;
	unsigned short workers_;
	CURLM* curl_multi_;
	int* pipefds_;
	SyncWorker** sync_workers_;
	int worker_idx_;
	std::map<CURL*, PipeData*> handle_map_;

#define pipe_rfd(i) pipefds_[(i)*2]
#define pipe_wfd(i) pipefds_[(i)*2+1]
	
};

}

#endif