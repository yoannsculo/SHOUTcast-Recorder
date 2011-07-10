#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "metadata.h"
#include "icy-string.h"

int metadata_listener(Stream *stream, char *buffer) {
    if (!is_metadata(stream)) {
        return 1;
    }

    if (is_metadata_header(stream)) {
        return metadata_header_handler(stream, buffer);
    } else if (is_metadata_body(stream)) {
        return metadata_body_handler(stream, buffer);
    }
    return 0;
}


int metadata_header_handler(Stream *stream, char *buffer) {
    MetaData *metadata = &stream->metadata;

    metadata->ptr = metadata->buffer; // Rewind
    metadata->size = abs((int)*buffer) * 16;

    if (metadata->size == 0) {
        stream->bytes_count = 0;
        stream->status = E_STATUS_MP3DATA;
    }
    else {
        stream->status = E_STATUS_METADATA_BODY;
    }

    return 0;
}


int metadata_body_handler(Stream *stream, char *buffer) {
    MetaData *metadata = &stream->metadata;
    *metadata->ptr = *buffer;
    if ((unsigned)(metadata->ptr - metadata->buffer) == (metadata->size-1)) {
        char metadata_content[500];
        strncpy(metadata_content, metadata->buffer, metadata->size);
        get_metadata_field(metadata_content, "StreamTitle", stream->stream_title);
        printf("%s\n", stream->stream_title);
        stream->bytes_count = 0;
        stream->status = E_STATUS_MP3DATA;
    } else {
        metadata->ptr++;
    }
    return 0;
}



int is_metadata(Stream *stream) {
    if (is_metadata_body(stream) || is_metadata_header(stream)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int is_metadata_body(Stream *stream) {
    if (stream->status == E_STATUS_METADATA_BODY) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int is_metadata_header(Stream *stream) {
    if (stream->status == E_STATUS_METADATA_HEADER) {
        return TRUE;
    } else {
        return FALSE;
    }
}
