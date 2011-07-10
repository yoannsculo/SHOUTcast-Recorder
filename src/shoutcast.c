#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shoutcast.h"
#include "types.h"
#include "header.h"
#include "metadata.h"
#include "mp3data.h"
#include "icy-string.h"

int load_stream(Stream *stream, const char *url) {
    ICYHeader *header = &stream->header;
    MetaData *metadata = &stream->metadata;
    Mp3Data *mp3data = &stream->mp3data;
    
    // Setting header
    header->icy_name[0]  = '\0';
    header->icy_notice1[0]  = '\0';
    header->icy_notice2[0]  = '\0';
    header->icy_genre[0] = '\0';
    header->icy_pub[0]   = '\0';
    header->icy_br[0]    = '\0';
    header->is_set       = 0;
    header->buffer       = malloc(4000*sizeof(char));
    header->ptr          = header->buffer;
    header->metaint      = 0;
    
    metadata->ptr = NULL;
    metadata->size = 0;

    mp3data->ptr = NULL;

    // Setting Stream information
    stream->bytes_count         = 0;
    stream->bytes_count_total   = 0;
    stream->blocks_count        = 0;
    stream->stream_title[0]     = '\0';
   
    stream->status = E_STATUS_HEADER;

    stream->output_stream = fopen("toto.mp3","w");
    if (strcpy(stream->url, url) != NULL)
        return 0;
    else
        return 1;
}

void global_listener(Stream *stream, char *buffer) {
    // http_code_listener(stream, buffer);
    
    if (is_header(stream)) {
        header_listener(stream, buffer);
    }
    else if (is_metadata(stream)) {
        metadata_listener(stream, buffer);
    }
    else if(is_mp3data(stream)) {
        mp3data_listener(stream, buffer);
    }
}

int write_data(Stream *stream, size_t *size) {
    int written = fwrite(stream->mp3data.buffer, *size,
                         stream->mp3data.size,
                         (FILE *)stream->output_stream);
    return written;
}
