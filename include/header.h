#ifndef __HEADER_H_
#define __HEADER_H_

#include "types.h"

int print_header(ICYHeader *header);
int header_listener(Stream *stream, char *buffer);
int is_header(Stream *stream);

#endif // __HEADER_H_
