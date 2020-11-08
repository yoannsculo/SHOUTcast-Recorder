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
				if ((stream_title[i]==0xE9)&&(stream_title[i+1]==0x8B)) { //Ë E9 B
					stream_title[i]=0xD3;
					removechar(stream_title,i+1);
					continue;
				} else {
					if ((stream_title[i]==0xE9)&&(stream_title[i+1]==0xA1)) { // á
						stream_title[i]=0xE1;
						removechar(stream_title,i+1);
						continue;
					} else {
						if (stream_title[i]==0xE9) { // é
							stream_title[i]=0xE9;
							continue;
						}
					}
				}
				if ( 	(stream_title[i]==0xC3)&&
					(stream_title[i+1]==0x83)&&
					(stream_title[i+2]==0x83)&&
					(stream_title[i+3]==0xC2))
				{
					stream_title[i]=0xE9; //é C3 83 C2 A9
					removechar(stream_title,i+1);
					removechar(stream_title,i+2);
					removechar(stream_title,i+3);
					continue;
				}
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
