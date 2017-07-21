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
	RSHtest(Context* cxt=NULL):ResponseStreamHandler(cxt), isPrintHeader(false) {}
	size_t Handle(Context* cxt, char* buffer, size_t size) {
		if(!isPrintHeader) {
			cout<<"response header:\n"<<cxt->GetResponse()->GetHeader().ToString()<<endl;
			cout<<"response body:"<<endl;
			isPrintHeader = true;
		}
		cout<<string(buffer, size);
		return size;
	}
private:
	bool isPrintHeader;
};

void TestClientStream(string url) {
	Request request(GET, url);
	Client client;
	cout<<"request: "<<request.ToString()<<endl;
	//ResponseStreamHandler* handler = new RSHtest();
	int res = client.Sync(&request, new RSHtest());
	cout<<"res: "<<client.ErrStr(res)<<endl;
}

int main(int argc, char* argv[]) {
	//httpclient_debug("enter main\n");
	//TestClient(argv[1]);
	TestClientStream(argv[1]);
	return 0;
}