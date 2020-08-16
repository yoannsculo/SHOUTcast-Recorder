#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "metadata.h"
#include "icy-string.h"

#include "log.h"

int metadata_listener(Stream *stream, char *buffer)
{
	if (!is_metadata(stream))
		return -1;

	if (is_metadata_header(stream))
		return metadata_header_handler(stream, buffer);
	else if (is_metadata_body(stream))
		return metadata_body_handler(stream, buffer);

	return 0;
}

int metadata_header_handler(Stream *stream, char *buffer)
{
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


int metadata_body_handler(Stream *stream, char *buffer)
{
	MetaData *metadata = &stream->metadata;
	*metadata->ptr = *buffer;
	if ((unsigned)(metadata->ptr - metadata->buffer) == (metadata->size-1)) {
		char metadata_content[500];
		char stream_title[500];
		strncpy(metadata_content, metadata->buffer, metadata->size);
		get_metadata_field(metadata_content, "StreamTitle", stream_title);
		stream_title[499]='\0';
		// filter problematic characters from StreamTitle
                for (int i =0; i < 499; i++) {
			if (stream_title[i]=='*')  { stream_title[i]='_'; continue; } 
			if (stream_title[i]=='?')  { stream_title[i]='_'; continue; } 
			if (stream_title[i]=='>')  { stream_title[i]='_'; continue; } 
			if (stream_title[i]=='<')  { stream_title[i]='_'; continue; } 
			if (stream_title[i]=='\\') { stream_title[i]='_'; continue; }
			if (stream_title[i]=='|')  { stream_title[i]='_'; continue; } 
			if (stream_title[i]=='\"') { stream_title[i]='_'; continue; } 
			if (stream_title[i]==':')  { stream_title[i]='_'; continue; } 
			if (stream_title[i]=='/')  { stream_title[i]='_'; continue; } 
			if (stream_title[i]=='\0') { break;} //done
		}
		printf("%s\n", stream_title);
		if (0 != strncmp(stream->stream_title, stream_title, 500))
		{
			strncpy(stream->stream_title,stream_title, 500);
			stream->metadata_count++;
			char new_filename[255] = "";
			snprintf(new_filename,255,"%s%03d-%s.mp3", stream->basefilename, stream->metadata_count, stream->stream_title);
                        new_filename[254]='\0';
			fclose(stream->output_stream);
			stream->output_stream = fopen(new_filename, "wb");
		}
                snprintf(stream_title,500, "%s\n", stream->stream_title);
		slog_prog(stream_title);

		stream->bytes_count = 0;
		stream->status = E_STATUS_MP3DATA;
	} else {
		metadata->ptr++;
	}
	return 0;
}

int is_metadata(Stream *stream)
{
	if (is_metadata_body(stream) || is_metadata_header(stream))
		return TRUE;
	else
		return FALSE;
}

int is_metadata_body(Stream *stream)
{
	if (stream->status == E_STATUS_METADATA_BODY)
		return TRUE;
	else
		return FALSE;
}

int is_metadata_header(Stream *stream)
{
	if (stream->status == E_STATUS_METADATA_HEADER)
		return TRUE;
	else
		return FALSE;
}
