#ifndef __TYPES_H_
#define __TYPES_H_

#define TRUE  1
#define FALSE 0
#define TITLE_SIZE 160 

typedef enum {
	E_STATUS_NONE,
	E_STATUS_HTTP_CODE,
	E_STATUS_HEADER,
	E_STATUS_METADATA_HEADER,
	E_STATUS_METADATA_BODY,
	E_STATUS_MP3DATA
} parsing_status;

typedef struct
{
	char icy_name[500];
	char icy_notice1[500];
	char icy_notice2[500];
	char icy_genre[TITLE_SIZE];
	char icy_pub[10];
	char icy_br[10]; // bitrate

	char *ptr;            // Pointer used to parse header buffer
	char *buffer;         // Dynamic buffer with the whole http header.
        // Freed after being use to store information
	int  is_set;          // 0: header is not set
        // 1: header is set
	unsigned int metaint; // MP3 data bytes between metadata blocks
}ICYHeader;

typedef struct
{
	char *ptr;              // Current metadata byte pointer
	char buffer[2000];      // Metadata buffer. Can't exceed "size" bytes
	unsigned int size;      // Size of metadata block
	unsigned int multiple;  // Boolean, if Metadata is cut into pieces
}MetaData;

typedef struct
{
	char *ptr;          // Current MP3 data byte pointer
	char *buffer;       // MP3 data buffer. Can't exceed "nmemb" bytes
	unsigned int size;  // MP3 data buffer size.
}Mp3Data;

typedef struct
{
	char url[TITLE_SIZE];          // Stream url
	char proxy[TITLE_SIZE];	// Network proxy
	char basefilename[TITLE_SIZE]; // basefilename of output file
	char filename[TITLE_SIZE];     // current filename of output file
	char ext[TITLE_SIZE];           // current extension of output file
	char onlytitle[TITLE_SIZE];	// record only if title contains onlytitle
	char to_ignore[TITLE_SIZE];	// ignore stream_title changes when including this string
	unsigned int duration;  // max recording duration
	unsigned int repeat;    // max recording repeats
	FILE *output_stream;    // Output MP3 file
	char stream_title[TITLE_SIZE]; // Current title
	char station[TITLE_SIZE];      // station
	int TA; // title = <Title> - <Artist> else <Artist> - <Title>
	parsing_status status; 

	ICYHeader header;        // Stream header (won't change after being set)
	MetaData metadata;       // Stream metadata (will change during session)
	Mp3Data mp3data;         // MP3 raw data

	unsigned int bytes_count; // Number of bytes received since last metadata block
	unsigned int bytes_count_total; // Number of bytes received since beginning
	unsigned int blocks_count; // Number of HTTP blocks received
	unsigned int metadata_count; // Number of metadata blocks received since beginning
} Stream;

void newfilename(Stream* stream,const char* title);

#endif // __TYPES_H_
