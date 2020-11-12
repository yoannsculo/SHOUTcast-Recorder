#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <sys/param.h>

#include "types.h"
#include "metadata.h"
#include "icy-string.h"
#include <taglib/tag_c.h>

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

void removechar( char str[], unsigned int i )
{
    for (unsigned int j=i; j<strlen(str)-2; j++)
         str[j]=str[j+1];
    str[strlen(str)-1]='\0';
}

int metadata_body_handler(Stream *stream, char *buffer)
{
	MetaData *metadata = &stream->metadata;
	*metadata->ptr = *buffer;
	if ((unsigned)(metadata->ptr - metadata->buffer) == (metadata->size-1)) {
		char metadata_content[500]="";
		char stream_title[500]="";
		strncpy(metadata_content, metadata->buffer, MIN(metadata->size,500));
		if(0==get_metadata_field(metadata_content, "StreamTitle", stream_title))
		{
			stream_title[499]='\0';
			metadata_content[499]='\0';
			// filter problematic characters from StreamTitle
	                for (unsigned int i =0; i < MIN(metadata->size,496); i++) {
				if (stream_title[i]=='\0') { break;} //done
				if (stream_title[i]=='*')  { stream_title[i]='_'; continue; }
				if (stream_title[i]=='?')  { stream_title[i]='_'; continue; }
				if (stream_title[i]=='>')  { stream_title[i]='_'; continue; }
				if (stream_title[i]=='<')  { stream_title[i]='_'; continue; }
				if (stream_title[i]=='\\') { stream_title[i]='_'; continue; }
				if (stream_title[i]=='|')  { stream_title[i]='_'; continue; }
				if (stream_title[i]=='\"') { stream_title[i]='_'; continue; }
				if (stream_title[i]==':')  { stream_title[i]='_'; continue; }
				if (stream_title[i]=='/')  { stream_title[i]='_'; continue; }
				if (stream_title[i]==0xC0)  { stream_title[i]='A'; continue; }
				if (stream_title[i]==0xC1)  { stream_title[i]='A'; continue; }
				if (stream_title[i]==0xC2)  { stream_title[i]='A'; continue; }
				if (stream_title[i]==0xC3)  { stream_title[i]='A'; continue; }
				if (stream_title[i]==0xC4)  { stream_title[i]='A'; continue; }
				if (stream_title[i]==0xC5)  { stream_title[i]='A'; continue; }
				if (stream_title[i]==0xC6)  { stream_title[i]='E'; continue; }
				if (stream_title[i]==0xC7)  { stream_title[i]='C'; continue; }
				if (stream_title[i]==0xC8)  { stream_title[i]='E'; continue; }
				if (stream_title[i]==0xC9)  { stream_title[i]='E'; continue; }
				if (stream_title[i]==0xCA)  { stream_title[i]='E'; continue; }
				if (stream_title[i]==0xCB)  { stream_title[i]='E'; continue; }
				if (stream_title[i]==0xCC)  { stream_title[i]='I'; continue; }
				if (stream_title[i]==0xCD)  { stream_title[i]='I'; continue; }
				if (stream_title[i]==0xCE)  { stream_title[i]='I'; continue; }
				if (stream_title[i]==0xCF)  { stream_title[i]='I'; continue; }
				if (stream_title[i]==0xD0)  { stream_title[i]='D'; continue; }
				if (stream_title[i]==0xD1)  { stream_title[i]='N'; continue; }
				if (stream_title[i]==0xD2)  { stream_title[i]='O'; continue; }
				if (stream_title[i]==0xD3)  { stream_title[i]='O'; continue; }
				if (stream_title[i]==0xD4)  { stream_title[i]='O'; continue; }
				if (stream_title[i]==0xD5)  { stream_title[i]='O'; continue; }
				if (stream_title[i]==0xD6)  { stream_title[i]='O'; continue; }
				if (stream_title[i]==0xD7)  { stream_title[i]='x'; continue; }
				if (stream_title[i]==0xD8)  { stream_title[i]='O'; continue; }
				if (stream_title[i]==0xD9)  { stream_title[i]='U'; continue; }
				if (stream_title[i]==0xDA)  { stream_title[i]='U'; continue; }
				if (stream_title[i]==0xDB)  { stream_title[i]='U'; continue; }
				if (stream_title[i]==0xDC)  { stream_title[i]='U'; continue; }
				if (stream_title[i]==0xDD)  { stream_title[i]='Y'; continue; }
				if (stream_title[i]==0xDE)  { stream_title[i]='p'; continue; }
				if (stream_title[i]==0xDF)  { stream_title[i]='B'; continue; }
				if (stream_title[i]==0xE0)  { stream_title[i]='a'; continue; }
				if (stream_title[i]==0xE1)  { stream_title[i]='a'; continue; }
				if (stream_title[i]==0xE2)  { stream_title[i]='a'; continue; }
				if (stream_title[i]==0xE3)  { stream_title[i]='a'; continue; }
				if (stream_title[i]==0xE4)  { stream_title[i]='a'; continue; }
				if (stream_title[i]==0xE5)  { stream_title[i]='a'; continue; }
				if (stream_title[i]==0xE6)  { stream_title[i]='a'; continue; }
				if (stream_title[i]==0xE7)  { stream_title[i]='c'; continue; }
				if (stream_title[i]==0xE8)  { stream_title[i]='e'; continue; }
				if (stream_title[i]==0xE9)  { stream_title[i]='e'; continue; }
				if (stream_title[i]==0xEA)  { stream_title[i]='e'; continue; }
				if (stream_title[i]==0xEB)  { stream_title[i]='e'; continue; }
				if (stream_title[i]==0xEC)  { stream_title[i]='i'; continue; }
				if (stream_title[i]==0xED)  { stream_title[i]='i'; continue; }
				if (stream_title[i]==0xEE)  { stream_title[i]='i'; continue; }
				if (stream_title[i]==0xEF)  { stream_title[i]='i'; continue; }
				if (stream_title[i]==0xF0)  { stream_title[i]='o'; continue; }
				if (stream_title[i]==0xF1)  { stream_title[i]='o'; continue; }
				if (stream_title[i]==0xF2)  { stream_title[i]='o'; continue; }
				if (stream_title[i]==0xF3)  { stream_title[i]='o'; continue; }
				if (stream_title[i]==0xF4)  { stream_title[i]='o'; continue; }
				if (stream_title[i]==0xF5)  { stream_title[i]='o'; continue; }
				if (stream_title[i]==0xF6)  { stream_title[i]='o'; continue; }
				if (stream_title[i]==0xF7)  { stream_title[i]='+'; continue; }
				if (stream_title[i]==0xF8)  { stream_title[i]='o'; continue; }
				if (stream_title[i]==0xF9)  { stream_title[i]='u'; continue; }
				if (stream_title[i]==0xFA)  { stream_title[i]='u'; continue; }
				if (stream_title[i]==0xFB)  { stream_title[i]='u'; continue; }
				if (stream_title[i]==0xFC)  { stream_title[i]='u'; continue; }
				if (stream_title[i]==0xFD)  { stream_title[i]='y'; continue; }
				if (stream_title[i]==0xFE)  { stream_title[i]='p'; continue; }
				if (stream_title[i]==0xFF)  { stream_title[i]='y'; continue; }
			}

			struct timeval curTime;
			gettimeofday(&curTime, NULL);
			int milli = curTime.tv_usec / 1000;

			char buffr [80];
			strftime(buffr, 80, "%Y-%m-%d %H:%M:%S", localtime(&curTime.tv_sec));

			char currentTime[84] = "";
			sprintf(currentTime, "%s.%03d", buffr, milli);

			printf("%s stream_title: %s\n", currentTime, stream_title);
			if (0 != strncmp(stream->stream_title, stream_title, 500))
			{
				char ext[3];
				strncpy(ext, stream->ext, 3);
				char oldfilename[255];
				char oldtitle[500];
				strncpy(oldfilename,stream->filename, 255);
				strncpy(oldtitle,stream->stream_title, 500);

				newfilename(stream, stream_title);

				taglib_set_strings_unicode(FALSE);
				TagLib_File *media_file;

				if (strncmp(ext,"aac",3) == 0)
				{
					media_file = taglib_file_new_type(oldfilename, TagLib_File_MP4);
				} else {
					media_file = taglib_file_new(oldfilename);
				}
				if (media_file != NULL)
				{
					TagLib_Tag *tag = taglib_file_tag(media_file);
					if (tag != NULL)
					{
						taglib_tag_set_comment(tag, oldtitle);
						char* token=strtok(oldtitle,"-");
						if (stream->TA == 0) {
							if (token) {
								taglib_tag_set_title(tag,token);
								token=strtok(NULL,"-");
							}
							if (token) {
								taglib_tag_set_artist(tag,token);
							}
						} else {
							if (token) {
								taglib_tag_set_artist(tag,token);
								token=strtok(NULL,"-");
							}
							if (token) {
								taglib_tag_set_title(tag,token);
							}
						}
						taglib_file_save(media_file);
					}
					taglib_tag_free_strings();
					taglib_file_free(media_file);
				}
			}
		}
		// slog metadata_content
		slog(metadata_content);

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
	if (stream->status == E_STATUS_METADATA_BODY){
		return TRUE;
	} else {
		return FALSE;
	}
}

int is_metadata_header(Stream *stream)
{
	if (stream->status == E_STATUS_METADATA_HEADER){
		return TRUE;
	} else {
		return FALSE;
	}
}
