#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

#include "icy-string.h"

int extract_header_fields(ICYHeader *header)
{
	char metaint[20];
	get_http_header_field(header->buffer, "icy-name", header->icy_name);
	get_http_header_field(header->buffer, "icy-notice1", header->icy_notice1);
	get_http_header_field(header->buffer, "icy-notice2", header->icy_notice2);
	get_http_header_field(header->buffer, "icy-genre", header->icy_genre);
	get_http_header_field(header->buffer, "icy-pub", header->icy_pub);
	get_http_header_field(header->buffer, "icy-br", header->icy_br);
	if(0==get_http_header_field(header->buffer, "icy-metaint", metaint)){
		header->metaint = atoi(metaint);
	} else {
		header->metaint = 0;
	}
	return 0;
}

int get_http_header_field(char *header, const char* field, char* value)
{
	int i;
	char *occurrence = NULL; 
	int content_pos = 0;

	occurrence  = strstr(header, field);
	content_pos = strlen(field)+1;
	if (occurrence != NULL)	{
		for (i=content_pos; occurrence[i] != '\0';i++) {
			if (is_cr_present(occurrence, i)) {
				// "<field>:" is deleted
				strncpy(value, occurrence+content_pos, i-content_pos);
				value[i-content_pos-1] = '\0';
				return 0;
			}
		}
	}
	// Value hasn't been found
	value[0] = '\0';
	return 1;
}


int get_metadata_field(char *metadata, const char* field, char* value)
{
	char *split;
	char *occurrence = NULL;
	split = strtok (metadata,";");
	while (split != NULL) {
		occurrence  = strstr(split, field);
		if (occurrence != NULL) {
			unsigned int content_pos = strlen(field)+2;
			unsigned int content_size = strlen(split)-content_pos-1; 
			strncpy(value, occurrence+content_pos, content_size);
			value[content_size] = '\0';
			return 0;
		} 
		split = strtok (NULL,";");
	}
	// Value hasn't been found
	value[0]='\0';
	return 1;
}

int is_cr_present(char *str, int pos)
{
	if (str[pos-1] == '\r' && str[pos] == '\n')
		return TRUE;
	else
		return FALSE;
}

int is_end_of_http_header(ICYHeader *header)
{
	unsigned int buffer_size = 0;
	buffer_size = header->ptr - header->buffer + 1;

	if (buffer_size < 4)
		return FALSE;

	if (*(header->ptr-3) == '\r' &&
	    *(header->ptr-2) == '\n' &&
	    *(header->ptr-1) == '\r' &&
	    *(header->ptr)   == '\n')
		return TRUE;
	else
		return FALSE;
}
