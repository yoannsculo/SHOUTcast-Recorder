#ifndef __METADATA_H_
#define __METADATA_H_

#include "types.h"

int is_metadata(Stream *stream);
int is_metadata_body(Stream *stream);
int is_metadata_header(Stream *stream);

int metadata_listener(Stream *stream, char *buffer);
int metadata_header_handler(Stream *stream, char *buffer);
int metadata_body_handler(Stream *stream, char *buffer);

#endif // __METADATA_H_
