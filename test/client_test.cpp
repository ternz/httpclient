#include <iostream>
#include <string>
#include "httpclient.h"

using namespace std;
using namespace http;

void TestClient(string url) {
	Request request(GET, url);
	Client client;
	Response response;
	int res = client.Sync(&request, &response);
	cout<<"res: "<<client.ErrStr(res)<<endl;
	cout<<"request: "<<request.ToString()<<endl;
	cout<<"response: "<<response.ToString()<<endl;
}

class RSHtest: public ResponseStreamHandler {
public:
	RSHtest(Context* cxt=NULL):ResponseStreamHandler(cxt) {}
	void HandleStart(Context* cxt) {
		cout<<"response header:\n"<<cxt->GetResponse()->GetHeader().ToString()<<endl;
		cout<<"response body:"<<endl;
	}
	size_t HandleStream(Context* cxt, char* buffer, size_t size) {
		cout<<string(buffer, size);
		return size;
	}
	void HandleEnd(Context* cxt) {
		cout<<"request end"<<endl;
	}
};

void TestClientStream(string url) {
	Request request(GET, url);
	Client client(1);
	cout<<"request: "<<request.ToString()<<endl;
	int res = client.Async(&request, new RSHtest());
	client.Wait(-1);
	cout<<"res: "<<client.ErrStr(res)<<endl;
}

int main(int argc, char* argv[]) {
	//httpclient_debug("enter main\n");
	//TestClient(argv[1]);
	TestClientStream(argv[1]);
	return 0;
}