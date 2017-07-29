#ifndef __UTIL_H__
#define __UTIL_H__
#include <string>
#include "define.h"
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
	ResponseHandler(Context* cxt=NULL):cxt_(cxt){}
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
protected:
	Context* cxt_; //注意清理问题
};

class ResponseStreamHandler {
public:
	friend class Client;
	ResponseStreamHandler(Context* cxt=NULL):cxt_(cxt),is_first_exec_(true){}
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
protected:
	Context* cxt_; //注意清理问题
private:
	bool is_first_exec_;
};

}
#endif