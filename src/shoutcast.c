#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shoutcast.h"
#include "types.h"
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



int header_listener(Stream *stream, char *buffer) {
    if (!is_header(stream)) {
        return 1;
    }
    
    ICYHeader *header = &stream->header;
    *header->ptr = *buffer;
    
    if (is_end_of_http_header(header)) {
        extract_header_fields(header); 
        free(header->buffer);
        
        if (header->metaint == 0) {
            printf("Error : Couldn't find metaint information\n");
            exit(-1);
        }

        print_header(header);
        
        // TODO init_for_mp3data(stream) ? 
        stream->bytes_count  = 0;
        stream->bytes_count_total = 0; // TODO : Commenter
        stream->mp3data.size = 0;
        stream->status = E_STATUS_MP3DATA;
    }
    else {
        header->ptr++;
    }
    return 0;
}

int is_header(Stream *stream) {
    if (stream->status == E_STATUS_HEADER) {
        return TRUE;
    } else {
        return FALSE;
    }
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

int is_mp3data(Stream *stream) {
    if (stream->status == E_STATUS_MP3DATA) {
        return TRUE;
    } else {
        return FALSE;
    }
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

int mp3data_listener(Stream *stream, char *buffer) {
    if (!is_mp3data(stream)) {
        return 1;
    }
    
    Mp3Data *mp3data = &stream->mp3data;

    *mp3data->ptr = *buffer;
    mp3data->size++; 
    mp3data->ptr++;
    stream->bytes_count++;
    
    if (stream->bytes_count == stream->header.metaint) {
        stream->status = E_STATUS_METADATA_HEADER;
    }

    return 0;
}

int write_data(Stream *stream, size_t *size) {
    int written = fwrite(stream->mp3data.buffer, *size,
                         stream->mp3data.size,
                         (FILE *)stream->output_stream);
    return written;
}

int print_header(ICYHeader *header) {
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
