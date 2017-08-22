#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <curl/curl.h>
#include "define.h"

namespace http {

#ifndef CURL_GLOBAL
#define CURL_GLOBAL CURL_GLOBAL_ALL
#endif

#define STR1(R) #R
#define STR2(R) STR1(R)

class GlobalObject {
public:
	GlobalObject() {
		//httpclient_debug("curl global init:%d\n", CURL_GLOBAL);
		curl_global_init(CURL_GLOBAL);
	}
	~GlobalObject() {
		//httpclient_debug("curl global cleanup\n");
		curl_global_cleanup();
	}
	void Init() {
		//void function
	}
};

extern GlobalObject object;

}

#endif
