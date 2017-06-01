#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "stream.h"
#include "files.h"
#include "pls.h"
#include "curl.h"

int load_stream(Stream *stream, const char *url, const char *proxy, const char *basefilename, const char* duration)
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

	stream->status = E_STATUS_HEADER;

	stream->duration = atoi(duration);

	if ((strcpy(stream->url, url) != NULL) && (strcpy(stream->proxy, proxy) != NULL)&& (strcpy(stream->basefilename, basefilename) != NULL))
	{
		sprintf(filename, "%s%d.mp3", stream->basefilename, stream->metadata_count);
		stream->output_stream = fopen(filename, "wb");
		return 0;
	}
	return -1;
}

int load_stream_from_playlist(char *filename, const char *proxy, const char *basefilename, const char *duration)
{
	Stream stream;
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

	if ((ret = load_stream(&stream, pls.entries->file, proxy, basefilename, duration)) < 0) {
		printf("Error : Couldn't load Shoutcast stream\n");
		goto err;
	}

	if ((ret = read_stream(&stream)) < 0) {
		printf("Error : Couldn't read Shoutcast stream\n");
		goto err;
	}

err:
	free(pls.entries);
early_err:
	return ret;

}
