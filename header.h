#ifndef __HEADER_H__
#define __HEADER_H__

#include <string>
#include <map>
using std::string;

namespace http {

#define CRLF "\r\n"
#define CRLF_LEN 2

class Header {
public:
	typedef std::map<string, string>::const_iterator Iterator;

	Header(){}
//	~Header() {}
	Header(const Header& header);
	Header& operator=(const Header& header);

	void Set(const string& key, const string& value);

	void Update(Header& h);

	bool Key(const string& key);

	string Get(const string& key);
	void Get(const string&key, string* oval);

	bool Delete(const string& key);

	Header& operator>>(const string& data);

	size_t ItemSize() {return hmap_.size();}

	Iterator Begin() {return hmap_.begin();}
	Iterator End() {return hmap_.end();}

	void ToString(string* ostr);
	string ToString();

	void Reset();

private:
	void parse(const string& data);

private:

	static const int DEFAULT_TMP_BUFFER_SIZE = 1024;

	std::map<string, string> hmap_;
	string tmp_buffer_;
	//bool discard_;
};

}

#endif