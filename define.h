#ifndef __DEFINE_H__
#define __DEFINE_H__
#include <cstdio>
namespace http {

#define DEBUG
#ifdef DEBUG
#define httpclient_debug(s, args...) printf(s, ##args)
#else
#define httpclient_debug(s, args...)
#endif

#define EASY_C  0x01
#define MUTIL_C 0x02
#define SHARE_C 0x03
#define CODE_SHIFT 24
#define is_code(code, code_flag) (((code)>>CODE_SHIFT & (code_flag)) == ((code)>>CODE_SHIFT))
#define is_easy_code(code)  is_code(code, EASY_C)
#define is_mutil_code(code) is_code(code, MUTIL_C)
#define is_share_code(code) is_code(code, SHARE_C)
#define make_code(code, code_flag) ((code_flag)<<CODE_SHIFT | (int)(code))
#define easy_code(code)  make_code(code, EASY_C)
#define mutil_code(code) make_code(code, MUTIL_C)
#define share_code(code) make_code(code, SHARE_C)
#define resolv_code(code, code_flag) ((code_flag)<<CODE_SHIFT ^ (int)(code))
#define resolv_easy_code(code)  resolv_code(code, EASY_C)
#define resolv_mutil_code(code) resolv_code(code, MUTIL_C)
#define resolv_share_code(code) resolv_code(code, SHARE_C)

#define CLIENT_OK 0

#define easy_error_code(code) ((code)==CURLE_OK ? CLIENT_OK : easy_code(code))
#define mutil_error_code(code) ((code)==CURLM_OK ? CLIENT_OK : mutil_code(code))
#define share_error_code(code) ((code)==CURLSHE_OK ? CLIENT_OK : share_code(code))

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