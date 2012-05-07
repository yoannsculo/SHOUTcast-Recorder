#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "mp3data.h"

// TODO : add id3tag_file(FILE *fp, char *title);

int mp3data_listener(Stream *stream, char *buffer)
{
	if (!is_mp3data(stream))
		return 1;

	Mp3Data *mp3data = &stream->mp3data;

	*mp3data->ptr = *buffer;
	mp3data->size++; 
	mp3data->ptr++;
	stream->bytes_count++;

	if (stream->bytes_count == stream->header.metaint)
		stream->status = E_STATUS_METADATA_HEADER;

	return 0;
}

int is_mp3data(Stream *stream)
{
	if (stream->status == E_STATUS_MP3DATA)
		return TRUE;
	else
		return FALSE;
}
