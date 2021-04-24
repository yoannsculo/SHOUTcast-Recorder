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

static char ascii[256]={'_','_','_','_','_','_','_','_','_','_','_','_','_','_','_','_',
'_','_','_','_','_','_','_','_','_','_','_','_','_','_','_','_',
' ','!','_','#','$','%','&','\'','(',')','_','+',',','-','.','_',
'0','1','2','3','4','5','6','7','8','9','_',';','_','=','_','_',
'@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
'P','Q','R','S','T','U','V','W','X','Y','Z','[','_',']','^','_',
'`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
'p','q','r','s','t','u','v','w','x','y','z','{','_','}','~',' ',
'_','_','_','_','_','_','_','_','_','_','_','_','_','_','_','_',
'_','_','_','_','_','_','_','_','_','_','_','_','_','_','_','_',
'_','_','_','_','_','_','_','_','_','_','_','_','_','_','_','_',
'_','_','_','_','_','_','_','_','_','_','_','_','_','_','_','_',
'A','A','A','A','A','A','E','C','E','E','E','E','I','I','I','I',
'D','N','O','O','O','O','O','x','O','U','U','U','U','Y','p','B',
'a','a','a','a','a','a','a','c','e','e','e','e','i','i','i','i',
'o','o','o','o','o','o','o','+','o','u','u','u','u','y','p','y'};


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

void rtrim(char *str)
{
    const char *seps = "\t\n\v\f\r ";
    int i = strlen(str) - 1;
    while (i >= 0 && strchr(seps, str[i]) != NULL) {
        str[i] = '\0';
        i--;
    }
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
	                for (unsigned int i =0; i < MIN(metadata->size,500); i++) {
				if (stream_title[i]=='\0') { break;} //done
				stream_title[i]=ascii[(int)stream_title[i]];
			}
                        rtrim(stream_title);
			if (0 != strncmp(stream->stream_title, stream_title, 500))
			{
				struct timeval curTime;
				gettimeofday(&curTime, NULL);
				int milli = curTime.tv_usec / 1000;

				char buffr [80];
				strftime(buffr, 80, "%Y-%m-%d %H:%M:%S", localtime(&curTime.tv_sec));
				char currentTime[84] = "";
				sprintf(currentTime, "%s.%03d", buffr, milli);
				printf("%s stream_title: [%s]\n", currentTime, stream_title);

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
