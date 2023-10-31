#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <ctype.h>

#include "types.h"
#include "metadata.h"
#include "icy-string.h"

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

void trim(char *str)
{
    const char *seps = "\t\n\v\f\r ";
    int i = 0;
    while (strchr(seps, str[i]) != NULL) {
        str++;
    }
    rtrim(str);
}

int metadata_body_handler(Stream *stream, char *buffer)
{
    MetaData *metadata = &stream->metadata;
    *metadata->ptr = *buffer;
    if ((unsigned)(metadata->ptr - metadata->buffer) == (metadata->size-1)) {
        char metadata_content[TITLE_SIZE];
        memset(metadata_content, 0, TITLE_SIZE);
        char stream_title[TITLE_SIZE];
        memset(stream_title, 0, TITLE_SIZE);
        strncpy(metadata_content, metadata->buffer, MIN(metadata->size,TITLE_SIZE));
        if(0==get_metadata_field(metadata_content, "StreamTitle", stream_title))
        {
            stream_title[TITLE_SIZE-1]='\0';
            metadata_content[TITLE_SIZE-1]='\0';
            // filter problematic characters from StreamTitle
            // and make PascalCase
            int toUpperCase = 1; //first character toUpperCase
            for (unsigned int i=0; i < MIN(metadata->size,TITLE_SIZE); i++) {
                if (stream_title[i]=='\0') { break;} //done
                char c=ascii[(int)stream_title[i]];
                if (toUpperCase==1) {
                     stream_title[i]=toupper(c);
                     toUpperCase=0;
                } else {
                     stream_title[i]=tolower(c);
                }
                if ((' '==stream_title[i])||('.'==stream_title[i])||('('==stream_title[i])){
                     toUpperCase=1; //next character toUpperCase
                }
            }
            trim(stream_title);
            if (stream_title==NULL||strlen(stream_title)==0) {
                strncpy(stream_title, stream->station, TITLE_SIZE);
            }
            if (0 != strncmp(stream->stream_title, stream_title, TITLE_SIZE)
                && NULL == strstr(stream_title, stream->to_ignore))
            {
                plog("stream_title: [%s] [%s]\n", stream_title, stream->stream_title);
                newfilename(stream, stream_title);
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
