#include "header.h"
#include "util.h"

namespace http {

Header::Header(const Header& header)
	:/*discard_(false),*/hmap_(header.hmap_) {

}

Header& Header::operator=(const Header& header) {
	hmap_ = header.hmap_;
	tmp_buffer_.resize(0);
	//discard_ = false;
	return *this;
}

void Header::Set(const string& key, const string& value) {
	if(key.empty()) 
		return;
	hmap_[KeyFormat(key)] = value;
}

void Header::Update(Header& h) {
	Iterator it = h.Begin();
	for(; it != h.End(); ++it) {
		hmap_[it->first] = it->second;
	}
}

bool Header::Key(const string& key) {
	std::map<string, string>::iterator it = hmap_.find(KeyFormat(key));
	if(it == hmap_.end()) 
		return false;
	return true;
}

string Header::Get(const string& key) {
	return hmap_[KeyFormat(key)];
}

void Header::Get(const string&key, string* oval) {
	*oval = hmap_[KeyFormat(key)];
}

bool Header::Delete(const string& key) {
	std::map<string, string>::iterator it = hmap_.find(KeyFormat(key));
	if(it == hmap_.end()) 
		return false;
	hmap_.erase(it);
	return true;
}

Header& Header::operator>>(const string& data) {
	size_t bpos=0, epos;
	while(true) {
		epos = data.find(CRLF, bpos);
		if(epos == string::npos) {
			if(bpos < data.size()) {
				tmp_buffer_.append(data.substr(bpos));
			}
			break;
		} else if(!tmp_buffer_.empty()) {
			tmp_buffer_.append(data.substr(bpos, epos));
			parse(tmp_buffer_);
			tmp_buffer_.clear();
			if(tmp_buffer_.size() > DEFAULT_TMP_BUFFER_SIZE)
				tmp_buffer_.resize(DEFAULT_TMP_BUFFER_SIZE);
		} else {
			parse(data.substr(bpos, epos));
		}
		bpos = epos+CRLF_LEN;
	}
	return *this;
}

void Header::parse(const string& data) {
	size_t pos = data.find_first_of(':');
	if(pos == string::npos)
		return;
	hmap_[KeyFormat(data.substr(0, pos))] = data.substr(pos+1);
	//TODO:去掉前后空格
}

void Header::ToString(string* ostr, const string& spliter) {
	ostr->clear();
	std::map<string, string>::iterator it;
	for(it = hmap_.begin(); it != hmap_.end(); ++it) {
		ostr->append(it->first);
		ostr->append(":");
		ostr->append(it->second);
		ostr->append(spliter);
	}
}

string Header::ToString(const string& spliter) {
	string ostr;
	ToString(&ostr, spliter);
	return ostr;
}

void Header::Reset() {
	hmap_.clear();
	tmp_buffer_.resize(0);
}

} //end of namespace