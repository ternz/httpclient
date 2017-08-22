#ifndef __CLIENT_H__
#define __CLIENT_H__
#include <pthread.h>
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
	enum WaitFor {WFNone, WFAvailable, WFDone, WFTimeout};

	Client(unsigned short workers=0);
	~Client();
	int Init();
	void BindWorkersData(void* data[]); //指针数据，长度必须跟设置的worker数相等
	int Sync(Request* req, Response* rsp);
	int Sync(Request* req, ResponseStreamHandler* handler, bool cleanUpHandler=true);
	int Async(Request* req, ResponseHandler* handler, bool cleanUpHandler=true);  //不能在未确定request完成前再次使用同样的request参数调用Async
	int Async(Request* req, ResponseStreamHandler* handler, bool cleanUpHandler=true);
	int Wait(int timeout_ms);
	int WaitAndStop(); 

	void SetMaxConcurrence(unsigned int val);
	bool IsBusy() {return is_busy_;}

	static const char* ErrStr(int code);
	const char* ErrStr();
	int ErrCode();

public:
	friend class ResponseHandler;
	friend class ResponseStreamHandler;
	class AsyncWorker;

private:
	Client(Client& c); //disable copy
	Client& operator=(const Client& c);
	static size_t streamWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata);
	int notifyRequest(PipeData* pd);
	int workerIdx();

	void increaseConcurrence_nts();  //not thread-safe
	void decreaseConcurrence_nts();

	void increaseConcurrence();
	void decreaseConcurrence();
	void decreaseAndNotify();
	void decreaseAndNotifyAvailable();
	void decreaseAndNotifyDone();
	void busyWait();
	int busyWaitTimeout(int timeout_ms);
	void waitForDone();

private:
	bool is_init_;
	bool is_stop_;
	int errcode_;
	unsigned short workers_;
	CURLM* curl_multi_;
	int* pipefds_;
	AsyncWorker** sync_workers_;
	int worker_idx_;
	std::map<CURL*, PipeData*> handle_map_;

	static const unsigned int DEFAULT_MAX_CONCURRENCE = 100;
	static const double CONCURRENCE_LOWER_LEVEL_PERCENT = 0.8;
	bool is_busy_;
	unsigned int max_concurrence_;
	unsigned int lower_level_;
	unsigned int concurrence_;
	//pthread_mutex_t* concurrence_mutex_;
	pthread_cond_t concurrence_cond_;
	pthread_mutex_t concurrence_cond_mutex_;
	
	WaitFor wf_what_;

#define pipe_rfd(i) pipefds_[(i)*2]
#define pipe_wfd(i) pipefds_[(i)*2+1]
	
};

class Client::AsyncWorker {
public:
	AsyncWorker(Client* client, int rfd);
	~AsyncWorker();
	//int DoRequest(PipeData* pd);
	
	int Start();
	int Join();
	void Stop();

	int Follow(Request* req, ResponseHandler* handler, bool cleanUpHandler=true);  
	int Follow(Request* req, ResponseStreamHandler* handler, bool cleanUpHandler=true);

	void SetWaitMs(int time_ms) {wait_ms_ = time_ms;}

	int GetRunningNum() {
		return running_num_;  //加锁没有意义，不需要加锁
	}

	void SetBindData(void* data) {bind_data_ = data;}

private:
	static void* threadFunc(void* arg);
	//void addHandle(CURL* e, PipeData* pd);
	//void removeHandle(CURL* e);

private:
	static const int DEFAULT_WAIT_MS = 50;

	Client* client_;
	int pipe_rfd_;
	bool run_;
	pthread_t thread_id_;
	CURLM* curl_multi_;
	int wait_ms_;
	int running_num_;
	bool joined_;

	std::map<CURL*, PipeData*> handle_map_;

	void* bind_data_; //TODO:确定在哪释放
};

}

#endif