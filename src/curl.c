#include <curl/curl.h>
#include <curl/easy.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "parsing.h"

#if LIBCURL_VERSION_NUM >= 0x073d00
#define TIME_IN_US 1
#define TIMETYPE curl_off_t
#define TIMEOPT CURLINFO_TOTAL_TIME_T
#define MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL     1000000
#else
#define TIMETYPE double
#define TIMEOPT CURLINFO_TOTAL_TIME
#define MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL     1
#endif

struct myprogress {
  TIMETYPE lastruntime; /* type depends on version, see above */
  CURL *curl;
  TIMETYPE duration;
  Stream *stream;
  time_t last;
  void (*thread) (void *);
};

void SwapOfs(void *p) {
  struct myprogress *myp = (struct myprogress *)p;
  CURL *curl = myp->curl;
  Stream *stream = myp->stream;
  TIMETYPE duration = stream->duration;
  duration*=MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL;
  time_t now;
  time(&now);
  TIMETYPE curtime = 0;
  curl_easy_getinfo(curl, TIMEOPT, &curtime);
  /* under certain circumstances it may be desirable for certain functionality
     to only run every N seconds, in order to do this the transaction time can
     be used */
  if( (curtime - myp->lastruntime) >= duration || difftime(now,myp->last)>= stream->duration) {
    newfilename(stream, stream->stream_title);
    printf("SwapOfs %lld %lld-%08lld=%lld %ld-%ld=%ld %s\n", duration, curtime, myp->lastruntime,curtime-myp->lastruntime, now, myp->last, now-myp->last,stream->filename);
    myp->lastruntime = curtime;
    myp->last=now;
  }
}

#if LIBCURL_VERSION_NUM >= 0x072000
int xferinfo(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
  struct myprogress *myp = (struct myprogress *)p;
  myp->thread(p);
  return 0;
}
#else
/* for libcurl older than 7.32.0 (CURLOPT_PROGRESSFUNCTION) */
int older_progress(void *p, double dltotal, double dlnow, double ultotal, double ulnow)
{
  struct myprogress *myp = (struct myprogress *)p;
  myp->thread(p);
  return 0;
}
#endif

int read_stream(Stream *stream)
{
  struct myprogress prog;
  time_t now;
  struct tm *info;
  char buffer[80];
  CURL *curl;
  CURLcode curl_res;
  struct curl_slist *headers = NULL;
  int ret = 0;
  if (stream->url == NULL) {
    time(&now);
    info = localtime( &now );
    strftime(buffer,80,"%T Error : stream->url null\n", info);
    printf(buffer);
    ret = -1;
    goto early_err;
  }
  time(&now);
  info = localtime( &now );
  strftime(buffer,80, "%T", info);
  printf("%s stream->url [%s]\n", buffer, stream->url);
  printf("%s stream->proxy [%s]\n", buffer, stream->proxy);
  printf(buffer);
  if ((curl = curl_easy_init()) == NULL) {
    time(&now);
    info = localtime( &now );
    strftime(buffer,80,"%T Error : curl_easy_init\n", info);
    printf(buffer);
    ret = -1;
    goto early_err;
  }
  printf("libcurlversion %s\n", LIBCURL_VERSION);
  printf("curl_off_t_fmt %s\n", CURL_FORMAT_CURL_OFF_T);
  printf("sizeof(curl_typeof_curl_off_t) %d\n", sizeof(CURL_TYPEOF_CURL_OFF_T));
  prog.curl = curl;
  prog.stream = stream;
  prog.thread = &SwapOfs;
  prog.lastruntime = 0;
  time(&prog.last);
  headers = curl_slist_append(headers, "Icy-MetaData:1"); // On force la récupération des metadata
  curl_easy_setopt(curl, CURLOPT_URL, stream->url);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_PROXY, stream->proxy);
  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, parse_header);
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, stream);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, parse_data);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, stream);
  curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
  curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 60L);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

  curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 102400L);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 30000L);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.64.0");
  curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
  curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
  curl_easy_setopt(curl, CURLOPT_HTTP09_ALLOWED, 1L);

#if LIBCURL_VERSION_NUM >= 0x072000
  curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, xferinfo);
  /* pass the struct pointer into the xferinfo function, note that this is an alias to CURLOPT_PROGRESSDATA */
  curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &prog);
#else
  curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, older_progress);
  /* pass the struct pointer into the progress function */
  curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &prog);
#endif
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
  curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
  time_t start_t, end_t;
  time(&start_t);
  uint seconds_elapsed = 0;
  uint duration=stream->duration*(1+stream->repeat);
  do {
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, duration-seconds_elapsed);
    curl_res = curl_easy_perform(curl);
    if (curl_res != CURLE_OK && curl_res != CURLE_OPERATION_TIMEDOUT && curl_res != CURLE_RECV_ERROR) {
      time(&now);
      info = localtime( &now );
      strftime(buffer,80,"%T", info);
      printf("%s ERROR %2d: %s\n", buffer, curl_res, curl_easy_strerror(curl_res));
      ret = -1;
      goto err;
    }
    time(&end_t);
    seconds_elapsed=(uint)difftime(end_t, start_t);
  } while(seconds_elapsed < duration);
err:
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
early_err:
  return ret;
}
