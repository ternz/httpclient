#ifndef __DEFINE_H__
#define __DEFINE_H__
#include <cstdio>
namespace http {

#define CLIENTDEBUG
#ifdef CLIENTDEBUG
#define httpclient_debug(s, args...) printf(s, ##args)
#define httpclient_error(s, args...) fprintf(stderr, s, ##args)
#else
#define httpclient_debug(s, args...)
#define httpclient_error(s, args...)
#endif

#define EASY_C  0x01
#define MUTIL_C 0x02
#define SHARE_C 0x03
#define ERRNO_C 0x04
#define CODE_SHIFT 24
#define BIT_MASK 0x0F000000
#define is_code(code, code_flag) (((code)&BIT_MASK) == ((code_flag)<<CODE_SHIFT))
#define is_easy_code(code)  is_code(code, EASY_C)
#define is_multi_code(code) is_code(code, MUTIL_C)
#define is_share_code(code) is_code(code, SHARE_C)
#define is_errno_code(code) is_code(code, ERRNO_C)
#define make_code(code, code_flag) ((code_flag)<<CODE_SHIFT | (int)(code))
#define easy_code(code)  make_code(code, EASY_C)
#define multi_code(code) make_code(code, MUTIL_C)
#define share_code(code) make_code(code, SHARE_C)
#define errno_code(code) make_code(code, ERRNO_C)
#define resolv_code(code, code_flag) ((code_flag)<<CODE_SHIFT ^ (int)(code))
#define resolv_easy_code(code)  resolv_code(code, EASY_C)
#define resolv_multi_code(code) resolv_code(code, MUTIL_C)
#define resolv_share_code(code) resolv_code(code, SHARE_C)
#define resolv_errno_code(code) resolv_code(code, ERRNO_C)

#define CLIENT_OK					 0
#define CLIENT_ERR_RESOURCE_INIT	 1
#define CLIENT_ERR_OUT_OF_MEMORY	 2
#define CLIENT_ERR_AGAIN 			 3
#define CLIENT_ERR_REQUEST_EXIST	 4
#define CLIENT_ERR_INVALID_PARAMTER	 5
#define CLIENT_ERR_STOP 			 6
#define CLIENT_OPT_TIMEOUT easy_code(CURLE_OPERATION_TIMEDOUT)

#define CLIENT_WAIT_FOR_AVAILABLE 0
#define CLIENT_WAIT_FOR_DONE -1

#define easy_error_code(code) ((code)==CURLE_OK ? CLIENT_OK : easy_code(code))
#define mutil_error_code(code) ((code)==CURLM_OK ? CLIENT_OK : multi_code(code))
#define share_error_code(code) ((code)==CURLSHE_OK ? CLIENT_OK : share_code(code))
#define errno_error_code(code) ((code)==0 ? CLIENT_OK : errno_code(code))

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

#define PTR_SIZE (sizeof(void*))

#define MAX_INT_VAL 0x7FFFFFFF

}

#endif