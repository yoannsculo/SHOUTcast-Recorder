#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <unistd.h> // linux

#include <regex.h>

#include "types.h"
#include "parsing.h"
#include "shoutcast.h"
#include "curl.h"
#include "log.h"

int load_stream_from_playlist(char *filename);

void usage(void)
{
	printf("Usage: shoutr [OPTIONS]\n");
	printf("       -p\t: playlist file\n");
	printf("       -u\t: stream url\n");
}

int main(int argc, char *argv[])
{
	int ret = -1;
	int pflag = 0;
	int uflag = 0;
	int c;
	char *cvalue = NULL;

	while ((c = getopt(argc, argv, "p:u:h")) != -1) {
		switch(c) {
			// playlist
			case 'p':
				pflag = 1;
				cvalue = optarg;
				break;
			// stream url
			case 'u':
				uflag = 1;
				cvalue = optarg;
				break;
			case 'h':
			default:
				usage();
				goto err_early;
				break;
		}
	}


	if (pflag && uflag) {
		usage();
		goto err_early;
	}

	if ((ret = log_open_files()) < 0) {
		printf("Couldn't open log files.\n");
		goto err_early;
	}

	if (pflag) {
		if ((ret = load_stream_from_playlist(cvalue)) < 0) {
			printf("Couldn't load stream from playlist\n");
			goto err;
		}
	}

	if (uflag) {
		Stream stream;
		load_stream(&stream, cvalue);

		if ((ret = read_stream(&stream)) < 0) {
			printf("Error : Couldn't read Shoutcast stream\n");
			goto err;
		}
	}

	// res = load_stream(&stream, "http://88.190.24.47:80");
err:
	log_close_files();
err_early:
	return ret;
}

size_t parse_data(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	unsigned int i;
	char buffer;
	Stream *stream = (Stream *)userdata;

	stream->mp3data.buffer = (char*) malloc(nmemb);
	stream->mp3data.ptr = stream->mp3data.buffer; 

	for (i=0;i<nmemb;i++) {
		buffer = ((char*)ptr)[i];
		global_listener(stream, &buffer);
		stream->bytes_count_total++;
	}

	write_data(stream, &size); 
	free(stream->mp3data.buffer);
	stream->mp3data.size = 0;

	stream->blocks_count++;

	return nmemb;
}
