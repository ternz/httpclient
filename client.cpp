#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <curl/curl.h>
#include <errno.h>
#include <string.h>
#include <map>
#include <sys/time.h>
#include "define.h"
#include "request.h"
#include "response.h"
#include "util.h"
#include "client.h"

using namespace std;

namespace http {

#define __INIT__ do{int res = Init();if(res != CLIENT_OK) return res;}while(0)



PipeData* newPipeData(Request* req, PipeData::HandlerType type, void* handler, bool cleanUpHandler) {
	//return new PipeData{req, type, handler, cleanUpHandler};
	PipeData* pd = new PipeData();
	pd->request = req;
	pd->handlerType = type;
	pd->handler.stream = (ResponseStreamHandler*)handler;
	pd->cleanUpHandler = cleanUpHandler;
	return pd;
}

void freePipeData(PipeData* pd) {
	delete pd;
}

void freePipeDataAndHandler(PipeData* pd) {
	if(pd->cleanUpHandler) {
		switch(pd->handlerType) {
		case PipeData::nonstream:
			delete pd->handler.nonstream;
			break;
		case PipeData::stream:
			delete pd->handler.stream;
			break;
		}
	}
	delete pd;
}

class Client::SyncWorker {
public:
	SyncWorker(Client* client, int rfd);
	~SyncWorker();
	//int DoRequest(PipeData* pd);
	
	int Start();
	int Join();
	void Stop();

	void SetWaitMs(int time_ms) {wait_ms_ = time_ms;}

	int GetRunningNum() {
		return running_num_;  //加锁没有意义，不需要加锁
	}

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

	map<CURL*, PipeData*> handle_map_;
};

Client::SyncWorker::SyncWorker(Client* client, int rfd)
	:client_(client), pipe_rfd_(rfd), run_(false), thread_id_(-1), curl_multi_(NULL)
	,wait_ms_(DEFAULT_WAIT_MS), running_num_(0), joined_(false) {

}

Client::SyncWorker::~SyncWorker() {
	//TODO:join or stop?
	if(curl_multi_ != NULL) {
		map<CURL*, PipeData*>::iterator it = handle_map_.begin();
		for(; it != handle_map_.end(); ++it) {
			freePipeDataAndHandler(it->second);
			curl_multi_remove_handle(curl_multi_, it->first);
		}
		curl_multi_cleanup(curl_multi_);
	}
	//close(pipe_rfd_);
}

int Client::SyncWorker::Start() {
	if(run_) return CLIENT_OK;

	if(curl_multi_ == NULL) {
		curl_multi_ = curl_multi_init();
		if(curl_multi_ == NULL) 
			return CLIENT_ERR_RESOURCE_INIT;
	}
	
	run_ = true;
	joined_ = false;
	int res;
	res = pthread_create(&thread_id_, NULL, threadFunc, this);
	if(res != 0) {
		run_ = false;
		return errno_code(errno);
	}
	
	return CLIENT_OK;
}

int Client::SyncWorker::Join() {
	joined_ = true;
	if(thread_id_ != -1) {
		int res = pthread_join(thread_id_, NULL);
		if(res != 0) {
			return errno_code(errno);
		}
	}
	return CLIENT_OK;
}

void Client::SyncWorker::Stop() {
	//TODO:join?
	run_ = false;
}

void* Client::SyncWorker::threadFunc(void* arg) {
	SyncWorker* me = reinterpret_cast<SyncWorker*>(arg);
	struct curl_waitfd pipe_fd;
	pipe_fd.fd = me->pipe_rfd_;
	pipe_fd.events = CURL_WAIT_POLLIN;

	int res;
	while(me->run_) {
		int numfds=0;
		res = curl_multi_wait(me->curl_multi_, &pipe_fd, 1, me->wait_ms_, &numfds);
		if(res != CURLM_OK) {
			httpclient_error("curl_multi_wait failed: %s\n", curl_multi_strerror((CURLMcode)res));
			//TODO:exit?
		}
		if(numfds == 0) { //timeout
			if(me->joined_ && me->running_num_ == 0)
				break;
			//else {
				//curl_multi_perform(me->curl_multi_, &(me->running_num_));
				//continue;   
			//}
		} else if(pipe_fd.revents & CURL_WAIT_POLLIN) {
			//httpclient_debug("read pipe:%d\n", pipe_fd.fd);
			while(true) {
				PipeData* data;
				res = read(pipe_fd.fd, &data, PTR_SIZE);
				if(res < 0) {
					if(errno == EAGAIN || errno == EWOULDBLOCK) break;
					else {
						//TODO: exit
						httpclient_error("read pipe failed: %s\n", strerror(errno));
					}
				} else if(res == 0) {
					//TODO:close pipe
					httpclient_debug("pipe %d close\n", pipe_fd.fd);
				} else {
					//TODO:res != PTR_SIZE?
					//me->addHandle(e, data);
					res = addHandler(me->curl_multi_, me->handle_map_, data->request->curl_handle_, data);
					if(res != CLIENT_OK) {
						//TODO:handler error
						httpclient_error("add handle failed: %s\n", ErrStr(res));
					}
				}
			}
		}

		res = curl_multi_perform(me->curl_multi_, &(me->running_num_));
		if(res != CURLM_OK) {
			httpclient_error("curl_multi_perform failed: %s\n", curl_multi_strerror((CURLMcode)res));
			//TODO:error handle
		}

		int msgs_left=0;
		CURLMsg *msg=NULL;
		while(msg=curl_multi_info_read(me->curl_multi_, &msgs_left)) {
			if(msg->msg == CURLMSG_DONE) {
				PipeData* pd = me->handle_map_[msg->easy_handle];
				if(msg->data.result == CURLE_OK) {
					if(pd->handlerType == PipeData::nonstream) {
						pd->handler.nonstream->Handle(pd->handler.nonstream->getContext());
					} else {
						pd->handler.stream->HandleEnd(pd->handler.stream->getContext());
					}
				} else if(msg->data.result == CURLE_OPERATION_TIMEDOUT) {
					//TODO:timeout
					if(pd->handlerType == PipeData::nonstream) {
						pd->handler.nonstream->HandleTimeout(pd->handler.nonstream->getContext());
					} else {
						pd->handler.stream->HandleTimeout(pd->handler.stream->getContext());
					}
				} else {
					//handle error
					httpclient_error("request:%s failed: %s\n", 
						me->handle_map_[msg->easy_handle]->request->ToString().c_str(), 
						curl_easy_strerror(msg->data.result));
					
				}
				//me->removeHandle(msg->easy_handle);
				res = removeHandler(me->curl_multi_, me->handle_map_, msg->easy_handle);
				if(res != CLIENT_OK) {
					httpclient_error("remove handle error: %s\n", ErrStr(res));
				}
			}
		}
	}
}

/*void Client::SyncWorker::addHandle(CURL* e, PipeData* pd) {
	pair<map<CURL*, PipeData*>::iterator, bool> ret;
	ret = handle_map_.insert(pair<CURL*, PipeData*>(e, pd));
	if(ret.second) {
		int res = curl_multi_add_handle(curl_multi_, e);
		if(res != CURLM_OK) {
			httpclient_error("curl_multi_add_handle failed: %s\n", curl_multi_strerror(res));
		    //TODO:error handle, free memory, inform request failed, callback?
			freePipeData(pd); 
		}
	} else {
		//add handle failed
		//freePipeDataAndHandler(pd);
		freePipeData(pd);  //内存泄露总比崩溃好
	}
}

void Client::SyncWorker::removeHandle(CURL* e) {
	int res = curl_multi_remove_handle(curl_multi_, e);
	if(res != CURLM_OK) {
		httpclient_error("curl_multi_remove_handle error: %s\n", curl_multi_strerror(res));
	}
	PipeData* data = handle_map_[e];
	if(data != NULL) {
		freePipeDataAndHandler(data);
	}
	handle_map_.erase(e);
}*/

int addHandler(CURLM* cm, std::map<CURL*, PipeData*>& m, CURL* e, PipeData* pd) {
	pair<map<CURL*, PipeData*>::iterator, bool> ret;
	ret = m.insert(pair<CURL*, PipeData*>(e, pd));
	if(ret.second) {
		int res = curl_multi_add_handle(cm, e);
		if(res != CURLM_OK) {
			//httpclient_error("curl_multi_add_handle failed: %s\n", curl_multi_strerror(res));
		    //TODO:error handle, free memory, inform request failed, callback?
			freePipeData(pd); 
			return multi_code(res);
		}
	} else {
		//add handle failed
		//freePipeDataAndHandler(pd);
		freePipeData(pd);  //内存泄露总比崩溃好
	}
	return CLIENT_OK;
}

int removeHandler(CURLM* cm, std::map<CURL*, PipeData*>& m, CURL* e) {
	int res = curl_multi_remove_handle(cm, e);
	if(res != CURLM_OK) {
		//httpclient_error("curl_multi_remove_handle error: %s\n", curl_multi_strerror(res));
		return multi_code(res);
	}
	PipeData* data = m[e];
	if(data != NULL) {
		freePipeDataAndHandler(data);
	}
	m.erase(e);
	return CLIENT_OK;
}



Client::Client(unsigned short workers)
	:workers_(workers), errcode_(CLIENT_OK), pipefds_(NULL)
	,sync_workers_(NULL), worker_idx_(0), is_init_(false) {
	if(workers_ == 0) {
		curl_multi_ = curl_multi_init();
	} else {
		curl_multi_ = NULL;
	}
}

Client::~Client() {
	if(curl_multi_ != NULL) {
		curl_multi_cleanup(curl_multi_);
	} 
	if(pipefds_ != NULL) {
		for(int i=0; i<2*workers_; ++i) {
			if(pipefds_[i] != -1)
				close(pipefds_[i]);
		}
		delete[] pipefds_;
	}
	if(sync_workers_ != NULL) {
		for(int i=0; i<workers_; ++i) {
			if(sync_workers_ != NULL)
				delete sync_workers_[i];
		}
		delete[] sync_workers_;
	}
}

int Client::Init() {
	if(is_init_) return CLIENT_OK;

	if(workers_ == 0) {
		if(curl_multi_ == NULL) {
			curl_multi_ = curl_multi_init();
			if(curl_multi_ == NULL) 
				return CLIENT_ERR_RESOURCE_INIT;
		}
	} else {
		pipefds_ = new int[2*workers_];
		if(pipefds_ == NULL) 
			return CLIENT_ERR_OUT_OF_MEMORY;
		for(int i=0; i<2*workers_; ++i) {
			pipefds_[i] = -1;
		}

		sync_workers_ = new SyncWorker*[workers_];
		if(sync_workers_ == NULL)
			return CLIENT_ERR_OUT_OF_MEMORY;
		for(int i=0; i<workers_; ++i) {
			sync_workers_[i] = NULL;
		}

		for(int i=0; i<workers_; ++i) {
			if(pipe2(&pipe_rfd(i), O_NONBLOCK) == -1) 
				return errno_error_code(errno); 
		}

		for(int i=0; i<workers_; ++i) {
			sync_workers_[i] = new SyncWorker(this, pipe_rfd(i));
			if(sync_workers_[i] == NULL) 
				return CLIENT_ERR_OUT_OF_MEMORY;
			int res = sync_workers_[i]->Start();
			if(res != CLIENT_OK)
				return res;
		}
	}

	is_init_ = true;
	return CLIENT_OK;
}

int Client::Sync(Request* req, Response* rsp) {
	__INIT__;
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
	__INIT__;

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
	if(res == CURLE_OK)
		handler->HandleEnd(handler->getContext());

	if(cleanUpHandler)
		delete handler;
	return easy_error_code(res);
}
	
int Client::Async(Request* req, ResponseHandler* handler, bool cleanUpHandler) {
	__INIT__;

	int res;
	res = req->Prepare();
	if(res != CURLE_OK) 
		return easy_code(res);

	Context* cxt = handler->getContext();
	Response* rsp = cxt->allocResponse(true);
	curl_easy_setopt(req->curl_handle_, CURLOPT_HEADERFUNCTION, rsp->headerCallback);
	curl_easy_setopt(req->curl_handle_, CURLOPT_HEADERDATA, rsp);
	curl_easy_setopt(req->curl_handle_, CURLOPT_WRITEFUNCTION, rsp->writeCallback);
	curl_easy_setopt(req->curl_handle_, CURLOPT_WRITEDATA, rsp);

	PipeData* pd = newPipeData(req, PipeData::nonstream, handler, cleanUpHandler);

	if(curl_multi_ != NULL) {
		//res = curl_multi_add_handle(curl_multi_, req->curl_handle_);
		//return mutil_error_code(res);
		return addHandler(curl_multi_, handle_map_, req->curl_handle_, pd);
	}
	
	res = notifyRequest(pd);
	if(res != CLIENT_OK) {
		curl_easy_reset(req->curl_handle_);
	}
	return res;
}
	
int Client::Async(Request* req, ResponseStreamHandler* handler, bool cleanUpHandler) {
	__INIT__;

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

	PipeData* pd = newPipeData(req, PipeData::stream, handler, cleanUpHandler);

	if(curl_multi_ != NULL) {
		//res = curl_multi_add_handle(curl_multi_, req->curl_handle_);
		return addHandler(curl_multi_, handle_map_, req->curl_handle_, pd);
		//return mutil_error_code(res);
	}

	res = notifyRequest(pd);
	if(res != CLIENT_OK) {
		curl_easy_reset(req->curl_handle_);
	}
	return res;
}

int Client::Wait(int timeout_ms) {
	int res;
	errcode_ = CLIENT_OK;
	if(workers_ == 0) {
		long time_left = timeout_ms;
		if(timeout_ms == -1)
			time_left = MAX_INT_VAL;
		struct timeval tv_start, tv_end;
		//long time_diff;
		int still_running;
		do {
			gettimeofday(&tv_start, NULL);
			res = curl_multi_perform(curl_multi_, &still_running);
			if(res != CURLM_OK) {
				errcode_ = multi_code(res);
				break;
			}
			if(still_running == 0) {
				break;
			}
			res = curl_multi_wait(curl_multi_, NULL, 0, (int)time_left, NULL);
			if(res != CURLM_OK) {
				errcode_ = multi_code(res);
				break;
				//TODO: 一致性
			}
			gettimeofday(&tv_end, NULL);
			time_left -= (tv_end.tv_sec - tv_start.tv_sec)*1000 + (tv_end.tv_usec - tv_start.tv_usec)/1000;
		}while(time_left > 0);

		int msgs_left=0;
		CURLMsg *msg=NULL;
		while(msg = curl_multi_info_read(curl_multi_, &msgs_left)) {
			if(msg->msg == CURLMSG_DONE) {
				PipeData* pd = handle_map_[msg->easy_handle];
				if(msg->data.result == CURLE_OK) {
					if(pd->handlerType == PipeData::nonstream) {
						pd->handler.nonstream->Handle(pd->handler.nonstream->getContext());
					} else {
						pd->handler.stream->HandleEnd(pd->handler.stream->getContext());
					}
				} else if(msg->data.result == CURLE_OPERATION_TIMEDOUT) {
					//timeout
					if(pd->handlerType == PipeData::nonstream) {
						pd->handler.nonstream->HandleTimeout(pd->handler.nonstream->getContext());
					} else {
						pd->handler.stream->HandleTimeout(pd->handler.stream->getContext());
					}
				} else {
					//handle error
					httpclient_error("request:%s failed: %s\n", 
						handle_map_[msg->easy_handle]->request->ToString().c_str(), 
						curl_easy_strerror(msg->data.result));
					
				}
				//me->removeHandle(msg->easy_handle);
				res = removeHandler(curl_multi_, handle_map_, msg->easy_handle);
				if(res != CLIENT_OK) {
					httpclient_error("remove handle error: %s\n", ErrStr(res));
					errcode_ = res;
				}
			}
		}
		if(errcode_ != CLIENT_OK)
			return -1;
		else
			return still_running;
	} else {
		if(timeout_ms == -1) {
			for(int i=0; i<workers_; ++i) {
				sync_workers_[i]->Join();
				//TODO:what if reuse a client?
			}
		} else if(timeout_ms == 0) {
			//count running request
		} else {
			//just sleep? TODO:how about it finnishs in timeout_ms?
			usleep(timeout_ms*1000);
		}
		int num=0;
		for(int i=0; i<workers_; ++i) {
			num += sync_workers_[i]->GetRunningNum();
		}
		return num;
	}
}

int Client::workerIdx() {
	int tmp = worker_idx_;
	worker_idx_ = ++worker_idx_ % workers_;
	return tmp;
}

int Client::notifyRequest(PipeData* pd) {
	int idx = workerIdx();
	int mark = idx;
	int res;
	do {
		res = write(pipe_wfd(idx), &pd, PTR_SIZE);
		if(res == -1) {
			if(errno == EAGAIN) {
				idx = workerIdx();
			} else {
				return errno_code(errno);
			}
		} else {
			return CLIENT_OK;
		}
	} while(idx != mark);
	return CLIENT_ERR_AGAIN;
}

size_t Client::streamWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
	ResponseStreamHandler* handler = reinterpret_cast<ResponseStreamHandler*>(userdata);
	if(handler->is_first_exec_) {
		handler->HandleStart(handler->getContext());
		handler->is_first_exec_ = false;
	}
	return handler->HandleStream(handler->getContext(), ptr, size*nmemb);
}

const char* Client::ErrStr(int code) {
	if(is_easy_code(code)) {
		return curl_easy_strerror((CURLcode)resolv_easy_code(code));
	} else if(is_multi_code(code)) {
		return curl_multi_strerror((CURLMcode)resolv_multi_code(code));
	} else if(is_share_code(code)) {
		return curl_share_strerror((CURLSHcode)resolv_share_code(code));
	} else if(is_errno_code(code)) {
		return strerror(resolv_errno_code(code));
	} else {
		return ClientErrStr(code);
	}
}

}