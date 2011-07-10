#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shoutcast.h"
#include "types.h"

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

int extract_header_fields(ICYHeader *header) {
    char metaint[20];
    get_http_header_field(header->buffer, "icy-name", header->icy_name);
    get_http_header_field(header->buffer, "icy-notice1", header->icy_notice1);
    get_http_header_field(header->buffer, "icy-notice2", header->icy_notice2);
    get_http_header_field(header->buffer, "icy-genre", header->icy_genre);
    get_http_header_field(header->buffer, "icy-pub", header->icy_pub);
    get_http_header_field(header->buffer, "icy-br", header->icy_br);
    get_http_header_field(header->buffer, "icy-metaint", metaint);
    header->metaint = atoi(metaint);
    return 0;
}

int get_http_header_field(char *header, const char* field, char* value) {
    int i;
    char *occurrence = NULL; 
    int content_pos = 0;

    occurrence  = strstr(header, field);
    content_pos = strlen(field)+1;
    // TODO : Test NULL value 
    for (i=content_pos; occurrence[i] != '\0';i++) {
        if (is_cr_present(occurrence, i)) {
            // "<field>:" is deleted
            strncpy(value, occurrence+content_pos, i-content_pos);
            value[i-content_pos-1] = '\0';
            return 0;
        }
    }
    // Value hasn't been found
    value[0] = '\0';
    return 1;
}


int get_metadata_field(char *metadata, const char* field, char* value) {
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

int is_cr_present(char *str, int pos) {
    if (str[pos-1] == '\r' && str[pos] == '\n') {
        return TRUE;
    } else {
        return FALSE;
    }
}

int is_end_of_http_header(ICYHeader *header) {
    unsigned int buffer_size = 0;
    buffer_size = header->ptr - header->buffer + 1;
    
    if (buffer_size < 4) {
        return FALSE;
    }
    
    if (*(header->ptr-3) == '\r' && *(header->ptr-2) == '\n' &&
        *(header->ptr-1) == '\r' && *(header->ptr)   == '\n') {
        return TRUE;
    } else {
        return FALSE;
    }
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
