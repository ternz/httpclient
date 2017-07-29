#include <iostream>
#include <string>
#include <sys/time.h>
#include <cstdio>
#include <unistd.h>
#include "httpclient.h"

using namespace std;
using namespace http;

static const char *urls[] = {
  "http://www.microsoft.com",
  "http://www.baidu.com",
  "http://www.qq.com/",
  "http://www.sina.com.cn/"
};
#define CNT 4

#define TEPEAT_TIME 20

static Request requests[CNT];

class RSHtest: public ResponseStreamHandler {
public:
	RSHtest(Context* cxt=NULL):ResponseStreamHandler(cxt) {}
	void HandleStart(Context* cxt) {
		//cout<<"start request: "<<cxt->GetRequest()->GetUrl()<<endl;
	}
	size_t HandleStream(Context* cxt, char* buffer, size_t size) {
		//cout<<string(buffer, size);
		return size;
	}
	void HandleEnd(Context* cxt) {
		//cout<<"request "<<cxt->GetRequest()->GetUrl()<<" end "<<cxt->GetResponse()->GetStatusCode()<<endl;
	}
};

void delete_request(Request** req) {
	//printf("delete request: %x\n", *req);
	delete *req;
}


void TestSync(Client* client) {
	struct timeval tv_start, tv_end;
	gettimeofday(&tv_start, NULL);
	for(int j=0; j<TEPEAT_TIME; ++j) {
		for(int i=0; i<CNT; ++i) {
			client->Sync(&requests[i], new RSHtest(new Context(&requests[i])));
		}
	}
	gettimeofday(&tv_end, NULL);
	cout<<"Sync all request finished in "
		<<(tv_end.tv_sec - tv_start.tv_sec) + (tv_end.tv_usec - tv_start.tv_usec)/1000000.0
		<<"s"<<endl;
}

void TestAsync(Client* client) {
	struct timeval tv_start, tv_end;
	int res;
	gettimeofday(&tv_start, NULL);
	for(int j=0; j<TEPEAT_TIME; ++j) {
		for(int i=0; i<CNT; ++i) {
			Request* req = new Request(requests[i]);
			res = client->Async(req, 
					new RSHtest(new Context(req, NULL, delete_request, NULL)));
			//	client->Wait(0);
			if(res != CLIENT_OK) {
				cout<<"client Async error:"<<ErrStr(res)<<endl;
			}
		}
		//usleep(50000);
	}
	client->Wait(-1);
	gettimeofday(&tv_end, NULL);
	cout<<"Async all request finished in "
		<<(tv_end.tv_sec - tv_start.tv_sec) + (tv_end.tv_usec - tv_start.tv_usec)/1000000.0
		<<"s"<<endl;
}

int main(int argc, char* argv[]) {
	Client client(1);
	for(int i=0; i<CNT; ++i) {
		requests[i].SetUrl(urls[i]);
	}
	TestSync(&client);
	TestAsync(&client);
	return 0;
}