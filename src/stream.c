#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "stream.h"
#include "files.h"
#include "pls.h"
#include "curl.h"

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

int load_stream_from_playlist(char *filename)
{
    Stream stream;
    PlsFile pls;
    int res = 0;

    if (filename == NULL) {
        return -1;
    }

    if (is_pls_extension(filename) == FALSE) {
        return -1;
    }

    res = pls_load_file(filename, &pls);
    if (res < 0) {
        printf("Error : Couldn't load pls file\n");
        return -1;
    }

    res = load_stream(&stream, pls.entries->file);
    if (res < 0) {
        printf("Error : Couldn't load Shoutcast stream\n");
        return -1;
    }
   
    res = read_stream(&stream);
    if (res) {
        printf("Error : Couldn't read Shoutcast stream\n");
        return -1;
	}

    free(pls.entries); 

    return 0; // TODO : define better result
}

