#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
#include <io.h>
#include "../shoutcastRecorder/shoutcastRecorder/getopt.h"
#else
#include <unistd.h> // linux

#include <regex.h>
#endif
#include "types.h"
#include "parsing.h"
#include "shoutcast.h"
#include "curl.h"
#include "log.h"

int load_stream_from_playlist(Stream *stream, char *filename);

void usage(void)
{
    printf("Usage: shoutr [-p <playlist>|-u <stream_url>] [OPTIONS]\n");
    printf("options:\n");
    printf("\t-d\t: recording duration (in seconds)\n");
    printf("\t-e\t: fileextension (default mp3)\n");
    printf("\t-f\t: basefilename (default radio)\n");
    printf("\t-l\t: logfolder (default current folder)\n");
    printf("\t-n\t: name of station (default radio)\n");
    printf("\t-r\t: recording repeats (default 0 = none)\n");
    printf("\t-s\t: split title - artist (0) or artist - title (1, default)\n");
    printf("\t-t\t: title to record (default any)\n");
    printf("\t-x\t: proxy (default no proxy)\n");
}

int main(int argc, char *argv[])
{
    if (argc == 1) {
        usage();
        return -1;
    }
    int ret = -1;
    int pflag = 0;
    int uflag = 0;
    int c;
    char *cvalue = NULL;
    char *log = malloc(TITLE_SIZE*sizeof(char));
    memset(log, 0, TITLE_SIZE);
    strncpy(log, ".", TITLE_SIZE-1);
    Stream stream;
    memset(stream.to_ignore, 0, TITLE_SIZE);
    memset(stream.ext, 0, TITLE_SIZE);
    strncpy(stream.ext, "mp3", TITLE_SIZE-1);
    memset(stream.proxy, 0, TITLE_SIZE);
    stream.duration=0;
    stream.repeat=0;
    stream.TA = 1;
    memset(stream.basefilename, 0, TITLE_SIZE);
    strncpy(stream.basefilename,"radio",TITLE_SIZE-1);
    memset(stream.stream_title, 0, TITLE_SIZE);
    memset(stream.station, 0, TITLE_SIZE);
    memset(stream.onlytitle, 0, TITLE_SIZE);

    while ((c = getopt(argc, argv, "p:u:h:x:f:e:d:r:i:l:n:t:s:")) != -1) {
        switch(c) {
            // playlist
            case 'p':
                pflag = 1;
                cvalue = optarg;
                printf("playlist: %s\n", optarg);
                break;
            // stream url
            case 'u':
                uflag = 1;
                cvalue = optarg;
                printf("url: %s\n", optarg);
                break;
            // proxy
            case 'x':
                strncpy(stream.proxy, optarg, TITLE_SIZE-1);
                printf("proxy: %s\n", optarg);
                break;
            // fileextension
            case 'e':
                strncpy(stream.ext, optarg, TITLE_SIZE-1);
                printf("ext: %s\n", optarg);
                break;
            // basefilename
            case 'f':
                strncpy(stream.basefilename, optarg, TITLE_SIZE-1);
                printf("basefilename: %s\n", optarg);
                break;
            // duration
            case 'd':
                stream.duration = atoi(optarg);
                printf("duration: %s\n", optarg);
                break;
            case 'r':
                stream.repeat=atoi(optarg);
                printf("repeat: %s\n", optarg);
                break;
            case 's':
                stream.TA = atoi(optarg);
                printf("TA: %si\n", optarg);
                break;
            case 'l':
                strncpy(log, optarg, TITLE_SIZE-1);
                printf("log: %s\n", optarg);
                break;
            case 'n':
                strncpy(stream.stream_title, optarg, TITLE_SIZE-1);
                strncpy(stream.station, optarg, TITLE_SIZE-1);
                printf("station: %s\n", optarg);
                break;
            case 't':
                strncpy(stream.onlytitle, optarg, TITLE_SIZE-1);
                printf("onlytitle: %s\n", optarg);
                break;
            case 'i':
                strncpy(stream.to_ignore, optarg, TITLE_SIZE-1);
                printf("to_ignore: %s\n", optarg);
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
    if ((ret = log_open_files(log)) < 0) {
        printf("Couldn't open log files.\n");
        goto err_early;
    }
    if (pflag) {
        if ((ret = load_stream_from_playlist(&stream, cvalue)) < 0) {
            printf("Couldn't load stream from playlist\n");
            goto err;
        }
    }

    if (uflag) {
        load_stream(&stream, cvalue);
    }

    if ((ret = read_stream(&stream)) < 0) {
        printf("Error : Couldn't read Shoutcast stream\n");
        goto err;
    }
err:
    log_close_files();
err_early:
    free(log);
    return ret;
}

size_t parse_header(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    unsigned int i;
    char buffer;
    Stream *stream = (Stream *)userdata;
    size_t numbytes = size * nmemb;

    //stream->header.buffer;
    //copy *ptr to header.buffer starting at header.ptr position
    void* dest = stream->header.ptr;
    memcpy(dest, ptr, numbytes);

    for (i=0;i<numbytes;i++) {
        buffer = ((char*)ptr)[i];
        global_listener(stream, &buffer);
        stream->bytes_count_total++;
    }

    printf("%.*s", (int) numbytes, (char*)ptr);
    return numbytes;
}

size_t parse_data(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    unsigned int i;
    char buffer;
    Stream *stream = (Stream *)userdata;
    size_t numbytes = size * nmemb;

    stream->mp3data.buffer = (char*) malloc(numbytes);
    stream->mp3data.ptr = stream->mp3data.buffer;

    for (i=0;i<numbytes;i++) {
        buffer = ((char*)ptr)[i];
        global_listener(stream, &buffer);
        stream->bytes_count_total++;
    }

    write_data(stream);
    free(stream->mp3data.buffer);
    stream->mp3data.size = 0;

    stream->blocks_count++;

    return numbytes;
}
