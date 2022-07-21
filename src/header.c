#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "types.h"
#include "header.h"
#include "icy-string.h"

#define HEADER_MAX 20000 // Max parsed bytes to get a header

int header_listener(Stream *stream, char *buffer)
{
    ICYHeader *header;
    if (!is_header(stream))
        return 1;

    header = &stream->header;

    if (stream->bytes_count_total >= HEADER_MAX) {
        printf("Error : couldn't retrieve server information.\n");
        exit(-1);
    }

    *header->ptr = *buffer;

    if (is_end_of_http_header(header)) {
        extract_header_fields(header);

        if (header->metaint == 0) {
            printf("Error : Couldn't find metaint information\n");
        } else {
            stream->status = E_STATUS_MP3DATA;
        }
        print_header(header);

        // TODO init_for_mp3data(stream) ?
        stream->bytes_count  = 0;
        stream->bytes_count_total = 0; // TODO : Commenter
        stream->mp3data.size = 0;
    }
    else {
        header->ptr++;
    }
    return 0;
}

int is_header(Stream *stream)
{
    if (stream->status == E_STATUS_HEADER){
        return TRUE;

    } else {
        return FALSE;
    }
}

int print_header(ICYHeader *header)
{
    printf("##################################\n");
    printf("Name\t: %s\n", header->icy_name);
    printf("icy-notice1\t: %s\n", header->icy_notice1);
    printf("icy-notice2\t: %s\n", header->icy_notice2);
    printf("Genre\t: %s\n", header->icy_genre);
    //printf("Public\t: %s\n", (header->icy_pub?"yes":"no"));
    printf("Bitrate : %s kbit/s\n", header->icy_br);
    printf("metaint\t: %d\n", header->metaint);
    printf("##################################\n");
    return 0;
}
