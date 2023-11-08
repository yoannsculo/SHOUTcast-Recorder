#ifndef __CURL_H_
#define __CURL_H_

void SwapOfs(void *p);
int read_stream(Stream *stream);

#if LIBCURL_VERSION_NUM >= 0x072000
int xferinfo(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
#else
int older_progress(void *p, double dltotal, double dlnow, double ultotal, double ulnow);
#endif


#endif // __CURL_H_
