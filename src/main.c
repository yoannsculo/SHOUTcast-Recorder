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

// int main(int argc, char** argv)
int main()
{
	int ret = 0;

	// TODO : handler parameters
	// TODO : add option --radio_list=<file>
	// TODO : add option --quiet
	// TODO : add option --ncurses

	// TODO : add a big shoucast radio list with lots of radios 

	// TODO : add function shoutr_start(Stream *stream)
	// TODO : add function shoutr_stop(Stream *stream)

	if ((ret = log_open_files()) < 0)
		goto err_early;

	if ((ret = load_stream_from_playlist("frequence3.pls")) < 0) {
		printf("Couldn't load stream from playlist\n");
		goto err;
	}

	// load_stream_from_playlist("frequence3.pls");

	// res = load_stream(&stream, "http://stream-hautdebit.frequence3.net:8000");
	// res = load_stream(&stream, "http://80.237.152.83:8000");
	// res = load_stream(&stream, "http://88.190.24.47:80");
	// res = load_stream(&stream, "http://88.191.122.117:5000");
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
