#include <curl/curl.h>
#include "define.h"
#include "request.h"
#include "response.h"
#include "util.h"
#include "client.h"

namespace http {

Client::Client(unsigned short workers):workers_(workers) {

}

int Client::Sync(Request* req, Response* rsp) {
	int res;
	res = req->Prepare();
	if(res != CURLE_OK) 
		return easy_code(res);
	curl_easy_setopt(req->curl_handle_, CURLOPT_HEADERFUNCTION, rsp->headerCallback);
	curl_easy_setopt(req->curl_handle_, CURLOPT_HEADERDATA, rsp);
	curl_easy_setopt(req->curl_handle_, CURLOPT_WRITEFUNCTION, rsp->writeCallback);
	curl_easy_setopt(req->curl_handle_, CURLOPT_WRITEDATA, rsp);
	res = curl_easy_perform(req->curl_handle_);
	return easy_error_code(res);
}
	
int Client::Sync(Request* req, ResponseStreamHandler* handler, bool cleanUpHandler) {
	int res;
	res = req->Prepare();
	if(res != CURLE_OK) 
		return easy_code(res);
	
	Context* cxt = handler->getContext();
	cxt->allocResponse(false);
	curl_easy_setopt(req->curl_handle_, CURLOPT_HEADERFUNCTION, cxt->GetResponse()->headerCallback);
	curl_easy_setopt(req->curl_handle_, CURLOPT_HEADERDATA, cxt->GetResponse());
	curl_easy_setopt(req->curl_handle_, CURLOPT_WRITEFUNCTION, streamWriteCallback);
	curl_easy_setopt(req->curl_handle_, CURLOPT_WRITEDATA, handler);
	res = curl_easy_perform(req->curl_handle_);

	if(cleanUpHandler)
		delete handler;
	return easy_error_code(res);
}
	
int Client::Async(Request* req, ResponseHandler* handler, bool cleanUpHandler) {
	return 0;
}
	
int Client::Async(Request* req, ResponseStreamHandler* handler, bool cleanUpHandler) {
	return 0;
}

size_t Client::streamWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
	ResponseStreamHandler* handler = reinterpret_cast<ResponseStreamHandler*>(userdata);
	return handler->Handle(handler->getContext(), ptr, size*nmemb);
}

const char* Client::ErrStr(int code) {
	if(code == CLIENT_OK) {
		return "OK";
	} else if(is_easy_code(code)) {
		return curl_easy_strerror((CURLcode)resolv_easy_code(code));
	} else if(is_mutil_code(code)) {
		return curl_multi_strerror((CURLMcode)resolv_mutil_code(code));
	} else if(is_share_code(code)) {
		return curl_share_strerror((CURLSHcode)resolv_share_code(code));
	} else {
		return "unknown error";
	}
}

}