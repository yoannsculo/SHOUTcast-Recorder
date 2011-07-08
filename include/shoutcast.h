#ifndef __SHOUTCAST_H_
#define __SHOUTCAST_H_

#include "types.h"

int load_stream(Stream *stream, const char *url);
int extract_header_fields(ICYHeader *header);
int get_http_header_field(char *header, const char* field, char* value);

int write_data(Stream *stream, size_t *size);

int print_header(ICYHeader *header);

int is_header(Stream *stream);
int is_metadata(Stream *stream);
int is_metadata_body(Stream *stream);
int is_metadata_header(Stream *stream);
int is_mp3data(Stream *stream);

int is_cr_present(char *str, int pos);

void global_listener(Stream *stream, char *buffer);
int header_listener(Stream *stream, char *buffer);
int metadata_listener(Stream *stream, char *buffer);
int mp3data_listener(Stream *stream, char *buffer);

#endif // __SHOUTCAST_H_
