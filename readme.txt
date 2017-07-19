基于libcurl，开发一个类似golang net/http包的c++ http客户端库

名称空间:http

Header类
Header(std::map<string, string>& vals);

void Set(const string& key, const string& value);
设置一组键值，若设置重复的key，value将被覆盖

void Delete(const string& key, const string& value);
删除一组键值，若key不存在，则什么都不做

string Get(const string& key);
获取一组键值，若key不存在，返回空字符串。

//string& operator[](const string& key);

//Header& operator>>(const char* data);
Header& operator>>(const string& data);
以流的方式读取data解析header。可以连续输入多个header，必须以CRLF结束一个header，即使只有一个header也应以CRLF结尾以结束输入。若输入不符合header格式，则忽略输入。eg. header>>"key1:value1\r\nkey2:value2\r\n";
可以不用一次输入完整的数据，数据可分开多次输入。eg. header>>"key1:";
header>>"value1\r\nkey2:value2\r\n";

void ToString(string* ostr);
string ToString();
返回以字符串表示的header，以\r\n分隔多个header。eg. "key1:value1\r\nkey2:value2\r\n"

std::map<string, string> hmap_;
存储header

char* tmp_buffer_;
解析header的buffer。初始化为NULL，使用到时才分配内存，对象销毁时释放内存。

static const int TMP_BUFFER_SIZE = 1024;

enum Method {
	GET,
	HEAD,
	POST,
	PUT,
//	PATCH,
	DELETE,
//	CONNECT,
//	OPTIONS,
//	TRACE
}

Request类
//所有设置先暂存，到正在发起请求时才设置到curl
Request(Method m, string& url);
url中可含query string

void SetMethod(Method m);
//Method GetMethod();

void SetUrl(string& url);
//string GetUrl();

void SetFormParameter(const string& key, cosnt string& value);

void SetHeader(Header* h);
void SetHeaderValue(const string& key, const string& value);
//void GetHeaderValue(const string& key);

void RequestHeader();
在响应中包含头，默认不包含。若使用HEAD方法，自动包含。

//typedef ReadFunc void (*)(char*, size_t, void*)
//void SetReadFunction(ReadFunc func);

void SetRequestReader(RequestReader* writer, bool chunked=true, sizt_t size=0);
//注意，DataReader指针会在数据读完以后自动delete，在调用该函数后不应保留该指针并使用。推荐用法 SetRequestReader(new SomeRequestReader(...));
默认使用chunk传输，也就是默认设置Transfer-Encoding: chunked头，若不使用chunked，则必须指点传输数据量的大小size.当读完所有数据后，返回0表示数据已读取完毕。

void SetData(string& data, bool chunked=false);
void SetData(const char* data, szie_t size, bool chunked=false);

void Reset();

int Prepare();

const char* ErrStr();

//string url_;
//Method method_;
Header* header_;
string form_;
string data_;
RequestReader* writer_;
CURL* curl_handle_;
CURLCode code_;
//Response *response_;
初始化为NULL，设置时才分配内存


Response类

Header GetHeader();
string GetHeaderValue(const string& key);
void GetHeaderValue(const string& key, string* oval);

sizt_t ContentLength();
string ContentType();
void ContentType(string* oval);

void Data(string* data);
char* Data();
size_t DataSize();


Header* header_;
int status_code_;



Client类
Client(unsigned short workers=0);
workers为0时不使用多线程，大于0表示使用线程的数量

int Sync(Request* req, Response* rsp);
int Sync(Request* req, ResponseStreamHandler* handler);
int Async(Request* req, ResponseHandler* handler);
int Async(Request* req, ResponseStreamHandler* handler);

int Wait(int time_ms);
等待请求处理完成，time_ms大于0时，表示最大等待时长；等于0时，立刻返回；小于0，表示永久等待直到所有请求完成。
返回值为0表示所有请求已处理完成；大于0表示还有请求正在处理；小于0表示出错。


RequestReader类
该类为抽象类，用户必须实现自己的类。适合在传输数据量过大时配合Reauest类使用。
virtual size_t Read(char* buffer, size_t size) = 0;
把数据写到外部提供的buffer，size为buffer能一次写入的最大值。返回实际写出的数据量。当写完所有数据后，在该函数再次被调用时，返回0表示数据已写出完毕。
virtual ~RequestReader(){} 
必须有，保证继承类的正确释放

ResponseHandler类
ResponseHandler(Context* cxt=NULL);
可以
int Handle(Context* cxt) = 0;
virtual ~ResponseHandler(){}
Context* cxt_;

ResponseStreamHandler类
ResponseStreamHandler(Context* cxt=NULL);
int Handle(Context* cxt, char* buffer, size_t size) = 0;
virtual ~ResponseStreamHandler(){}
Context* cxt_;

typedef void (*ContextRequestCleaner)(Request**)
typedef void (*ContextArgCleaner)(void**)
注意把arg转换为对应的类型
Context类
Context(Request* req=NULL, void* parg=NULL, ContextRequestCleaner *crc=NULL, ContextArgCleaner *cac=NULL);
void SetRequest(Request* req, ContextRequestCleaner *crc=NULL);
void SetArg(void* parg, ContextArgCleaner *cac=NULL);
Request* Request();
Response* Response();
template<typename T> T* Arg();
Request* request_;
Response* response_;
void* parg_;



DataReader类
该类为抽象类，用户必须实现自己的类。适合在传输数据量过大时配合Response类使用。
virtual size_t Read(char* buffer, size_t size) = 0;
从buffer读取数据，size为buffer现有的数据量。返回实际读取了的数据量。
virtual ~DataReader(){}
必须有，保证继承类的正确释放

GlodbalObject类
自动调用curl_global_init(CURL_GLOBAL_ALL)和curl_global_cleanup()
CURL_GLOBAL_SSL