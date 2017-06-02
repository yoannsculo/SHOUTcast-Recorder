#ifndef __SHOUTCAST_H_
#define __SHOUTCAST_H_

#include "types.h"

int load_stream(Stream *stream, const char *url, const char *proxy, const char *basefilename, const char* duration);
void global_listener(Stream *stream, char *buffer);
int write_data(Stream *stream);

#endif // __SHOUTCAST_H_
