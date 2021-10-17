#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "types.h"
#include "stream.h"
#include "files.h"
#include "pls.h"
#include "curl.h"
#include "log.h"

void free_stream(Stream *stream)
{
	plog("free_stream\n");
	ICYHeader *header = &stream->header;
	free(header->buffer);
	header->ptr= NULL;
}
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

int exists(const char *fname)
{
    FILE *file;
    if ((file = fopen(fname, "r")))
    {
        fclose(file);
        return 1;
    }
    return 0;
}

void newfilename(Stream* stream, const char* title)
{
 const int size=255+1+3+1+500+1+255;
 char filename[size];
 time_t rawtime;
 struct tm * timeinfo;
 time (&rawtime);
 timeinfo = localtime(&rawtime);
 char basefilename[255];
 strftime(basefilename,254,stream->basefilename,timeinfo);
 if (title==NULL||strlen(title)==0) {
  snprintf(filename,size,"%s.%03d.%s", basefilename, stream->metadata_count, stream->ext);
  while (1 == exists(filename)) {
   stream->metadata_count++;
   snprintf(filename,size,"%s.%03d.%s", basefilename, stream->metadata_count, stream->ext);
  }
 } else {
  snprintf(filename,size,"%s.%03d.%s.%s", basefilename, stream->metadata_count, title, stream->ext);
  while (1 == exists(filename)) {
   stream->metadata_count++;
   snprintf(filename,size,"%s.%03d.%s.%s", basefilename, stream->metadata_count, title, stream->ext);
  }
 }
 if (title==NULL||strlen(title)==0) {
   // don't search for title match if no title to match 
 } else {
  if (stream->onlytitle!=NULL&&strlen(stream->onlytitle)!=0) {
   char str[255];
   strncpy(str, stream->onlytitle, 255);
   int title_found = 0;
   char* token=strtok(str,",");
   if (token) {
    while (token) {
     if(stristr(title, token)!=NULL) {
      title_found = 1;
     }
     token=strtok(NULL,",");
    }
   } else {
     if(stristr(title, stream->onlytitle)!=NULL) {
      title_found = 1;
     }
   }
   if (title_found == 0) {
    snprintf(filename,size,"%s","/dev/null");
    } else {
    stream->metadata_count++;
   }
  } else {
    stream->metadata_count++;
  }
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
 plog("newfilename: %s\n", filename);
}

