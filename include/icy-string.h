#ifndef __ICY_STRING_H_
#define __ICY_STRING_H_

int extract_header_fields(ICYHeader *header);
int get_http_header_field(char *header, const char* field, char* value);
int get_metadata_field(char *metadata, const char* field, char* value);
int is_cr_present(char *str, int pos);
int is_end_of_http_header(ICYHeader *header);

#endif // __ICY_STRING_H_
