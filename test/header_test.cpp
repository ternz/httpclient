#include <iostream>
#include <string>
#include "header.h"

using namespace std;
using namespace http;

void TestHeader() {
	Header header;
	header.Set("content-type", "application/json");
	cout<<header.Get("conTEnT-tYpe")<<endl;

	header>>"expired:123455\r\ndate:">>"2017-07-12">>"\r\n">>"test:testing\r\n";
	cout<<header.ToString()<<endl;

	cout<<"key date exist? "<<header.Key("date")<<endl;
	cout<<"key content-length exist? "<<header.Key("content-length")<<endl<<endl;

	header.Delete("test");
	cout<<header.ToString()<<endl;

	Header header2(header);
	cout<<"header copy 2:"<<endl<<header2.ToString()<<endl;

	Header header3;
	header3 = header;
	cout<<"header copy 3:"<<endl<<header3.ToString()<<endl;

	cout<<endl<<"header size:"<<header.ItemSize()<<endl;
	/*string key, val;
	for(int i=0; i < header.Size(); ++i) {
		header.GetItem(i, &key, &val);
		cout<<i<<" "<<key<<":"<<val<<endl;
	}*/
	Header::Iterator it = header.Begin();
	for(; it != header.End(); ++it) {
		cout<<it->first<<":"<<it->second<<endl;
	}
}

int main(int argc, char* argv[]) {
	TestHeader();
	return 0;
}