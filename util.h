#ifndef __UTIL_H__
#define __UTIL_H__
#include <string>
#include "define.h"
#include "client.h"
namespace http {

inline void UpperChar(char& c);
inline void LowerChar(char& c);
std::string KeyFormat(std::string key);
bool IsSpace(char c);
const char* MethodStr(Method m);
const char* ClientErrStr(int code);
const char* ErrStr(int code);

class Request;
class Response;
class Client;

typedef void (*ContextRequestCleaner)(Request**);
typedef void (*ContextArgCleaner)(void**);

class Context {
public:
	friend class Client;
	Context(Request* req=NULL, void* parg=NULL, ContextRequestCleaner crc=NULL, ContextArgCleaner cac=NULL)
		:request_(req), parg_(parg), crc_(crc), cac_(cac), response_(NULL) {}

	~Context();

	void SetRequest(Request* req, ContextRequestCleaner crc=NULL) {
		request_ = req;
		crc_ = crc;
	}
	void SetArg(void* parg, ContextArgCleaner cac=NULL) {
		parg_ = parg;
		cac_ = cac;
	}
	Request* GetRequest() {return request_;}
	Response* GetResponse() {return response_;}
	template<typename T> T* GetArg() {return reinterpret_cast<T*>(parg_);}
private:
	Response* allocResponse(bool write_body=true);
	void freeResponse();
private:
	Request* request_;
	Response* response_;
	void* parg_;

	ContextRequestCleaner crc_;
	ContextArgCleaner cac_;
};

class RequestReader {
public:
	virtual size_t Read(char* buffer, size_t size) = 0;
	virtual RequestReader* Copy() {return NULL;}
	virtual ~RequestReader(){}
};

class ResponseHandler {
public:
	friend class Client;
	//friend class Client::AsyncWorker;
	ResponseHandler(Context* cxt=NULL):cxt_(cxt),bind_data_(NULL),worker_(NULL){}
	virtual void Handle(Context* cxt) = 0;
	virtual void HandleTimeout(Context* cxt){}
	virtual ~ResponseHandler(){
		if(cxt_!=NULL) delete cxt_;
	}
protected:
	Context* getContext(){
		if(cxt_ == NULL)
			cxt_ = new Context();
		return cxt_;
	}
	void setBindData(void* data) {bind_data_ = data;}
	void* getBindData() {return bind_data_;} 
	void setWorker(Client::AsyncWorker* worker) {worker_ = worker;}
	Client::AsyncWorker* getWorker() {return worker_;}
	//TODO:如果在单线程异步模式下在handler内发起另一个request, 添加set get client
protected:
	Context* cxt_; //注意清理问题
	void* bind_data_;
	Client::AsyncWorker* worker_;
};

class ResponseStreamHandler {
public:
	friend class Client;
	//friend class Client::AsyncWorker;
	ResponseStreamHandler(Context* cxt=NULL):cxt_(cxt),is_first_exec_(true),bind_data_(NULL),worker_(NULL){}
	virtual void HandleStart(Context* cxt){}
	virtual size_t HandleStream(Context* cxt, char* buffer, size_t size) = 0;
	virtual void HandleEnd(Context* cxt){}
	virtual void HandleTimeout(Context* cxt){}

	virtual ~ResponseStreamHandler(){
		//httpclient_debug("ResponseStreamHandler clean context\n");
		if(cxt_!=NULL) delete cxt_;
	}
	
protected:
	Context* getContext(){
		if(cxt_ == NULL)
			cxt_ = new Context();
		return cxt_;
	}
	void setBindData(void* data) {bind_data_ = data;}
	void* getBindData() {return bind_data_;} 
	void setWorker(Client::AsyncWorker* worker) {worker_ = worker;}
	Client::AsyncWorker* getWorker() {return worker_;}

protected:
	Context* cxt_; //注意清理问题
	void* bind_data_;
	Client::AsyncWorker* worker_;

private:
	bool is_first_exec_;
};

void common_request_deleter(Request** req);

}
#endif