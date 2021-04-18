#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "types.h"
#include "stream.h"
#include "files.h"
#include "pls.h"
#include "curl.h"

int load_stream(Stream *stream, const char *url)
{
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
	stream->output_stream       = NULL;
        stream->filename[0]         = '\0';

	stream->status = E_STATUS_HEADER;

	strncpy(stream->url, url, 255);
	newfilename(stream, stream->stream_title);
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

static char tolower[256]={
'_','_','_','_','_','_','_','_','_','_','_','_','_','_','_','_',
'_','_','_','_','_','_','_','_','_','_','_','_','_','_','_','_',
' ','!','_','#','$','%','&','\'','(',')','_','+',',','-','.','_',
'0','1','2','3','4','5','6','7','8','9','_',';','_','=','_','_',
'@','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
'p','q','r','s','t','u','v','w','x','y','z','[','_',']','^',' ',
'`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
'p','q','r','s','t','u','v','w','x','y','z','{','_','}','~',' ',
'_','_','_','_','_','_','_','_','_','_','_','_','_','_','_','_',
'_','_','_','_','_','_','_','_','_','_','_','_','_','_','_','_',
'_','_','_','_','_','_','_','_','_','_','_','_','_','_','_','_',
'_','_','_','_','_','_','_','_','_','_','_','_','_','_','_','_',
'a','a','a','a','a','a','a','c','e','e','e','e','i','i','i','i',
'o','n','o','o','o','o','o','x','o','u','u','u','u','y','p','b',
'a','a','a','a','a','a','a','c','e','e','e','e','i','i','i','i',
'o','o','o','o','o','o','o','+','o','u','u','u','u','y','p','y'};

char* stristr(const char* haystack, const char* needle) {
  do {
    const char* h = haystack;
    const char* n = needle;
    while (tolower[(int)(*h)] == tolower[(int)(*n)] && *n) {
      h++;
      n++;
    }
    if (*n == 0) {
      return (char *) haystack;
    }
  } while (*haystack++);
  return 0;
}


void newfilename(Stream* stream, const char* title)
{
 const int size=255+1+3+1+500+1+255;
 char filename[size];
 if (title==NULL||strlen(title)==0) {
  snprintf(filename,size,"%s.%03d.%s", stream->basefilename, stream->metadata_count, stream->ext);
 } else {
  snprintf(filename,size,"%s.%03d.%s.%s", stream->basefilename, stream->metadata_count, title, stream->ext);
 }
 if (stream->onlytitle!=NULL&&strlen(stream->onlytitle)!=0&&stristr(title, stream->onlytitle)==NULL) {
    snprintf(filename,size,"%s","/dev/null");
 } else {
   stream->metadata_count++;
 }
 filename[254]='\0';
 if (stream->output_stream != NULL) fclose(stream->output_stream);
 stream->output_stream = fopen(filename, "wb");
 strncpy(stream->filename, filename, 255);
 if (title==NULL||strlen(title)==0) {
   stream->stream_title[0]='\0';
 } else {
   strncpy(stream->stream_title, title, 500);
 }
 struct timeval curTime;
 gettimeofday(&curTime, NULL);
 int milli = curTime.tv_usec / 1000;
 char buffr[20];
 strftime(buffr, 20, "%Y-%m-%d %H:%M:%S", localtime(&curTime.tv_sec));
 char currentTime[30] = "";
 sprintf(currentTime, "%s.%03d", buffr, milli);
 printf("%s newfilename: %s\n", currentTime, filename);
}

