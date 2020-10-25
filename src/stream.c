#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "stream.h"
#include "files.h"
#include "pls.h"
#include "curl.h"

int load_stream(Stream *stream, const char *url)
{
	char filename[255] = "";

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
	stream->metadata_count      = 0;
	stream->stream_title[0]     = '\0';
	stream->output_stream       = NULL;

	stream->status = E_STATUS_HEADER;

	memset(stream->url, 0, 255);
	strncpy(stream->url, url, 254);
	memset(filename, 0, 255);
	memset(stream->filename, 0, 255);
	
	newfilename(stream, filename, 255, stream->stream_title);
	strncpy(stream->filename, filename, 254);
	stream->output_stream = fopen(filename, "wb");
	return 0;
}

int load_stream_from_playlist(Stream *stream, char *filename)
{
	PlsFile pls;
	int ret = 0;

	if (filename == NULL) {
		ret = -1;
		goto early_err;
	}

	if (!is_pls_extension(filename)) {
		ret = -1;
		goto early_err;
	}

	if ((ret = pls_load_file(filename, &pls)) < 0) {
		printf("Error : Couldn't load pls file\n");
		goto early_err;
	}

	if ((ret = load_stream(stream, pls.entries->file)) < 0) {
		printf("Error : Couldn't load Shoutcast stream\n");
		goto err;
	}

err:
	free(pls.entries);
early_err:
	return ret;

}

void newfilename(const Stream *stream, char* filename, unsigned int size, char* title)
{
 if (strlen(title)==0) {
  snprintf(filename,size,"%s.%03d.%s", stream->basefilename, stream->metadata_count, stream->ext);
 } else {
  snprintf(filename,size,"%s.%03d.%s.%s", stream->basefilename, stream->metadata_count, title, stream->ext);
 }
 filename[size-1]='\0';
}

