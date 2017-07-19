#ifndef __DEFINE_H__
#define __DEFINE_H__

namespace http {

enum Method {
	GET,
	HEAD,
	POST,
	PUT,
//	PATCH,
	DELETE
//	CONNECT,
//	OPTIONS,
//	TRACE
};

#define M_GET "GET"
#define M_HEAD "HEAD"
#define M_POST "POST"
#define M_PUT "PUT"
#define M_DELETE "DELETE"

}

#endif