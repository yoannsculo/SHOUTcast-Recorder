#ifndef __SHOUTCAST_H_
#define __SHOUTCAST_H_

#include "types.h"

int load_stream(Stream *stream, const char *url);
void global_listener(Stream *stream, char *buffer);
int write_data(Stream *stream, size_t *size);

#endif // __SHOUTCAST_H_
