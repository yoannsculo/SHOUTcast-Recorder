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
    printf("\t-i\t: title - artist (0) or artist - title (1, default)\n");
    printf("\t-l\t: logfolder (default current folder)\n");
    printf("\t-n\t: name of station (default radio)\n");
    printf("\t-r\t: recording repeats (default 0 = none)\n");
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
    char *proxy = NULL;
    char *duration = "0";
    char *repeat = "0";
    char *ta = "1";

    char* basefilename = (char*) malloc(255*sizeof(char));
    sprintf(basefilename, "radio");

    char* stationname = NULL;

    char* fileext = (char*) malloc(255*sizeof(char));
    sprintf(fileext, "mp3");

    char* log = (char*) malloc(255*sizeof(char));
    sprintf(log, ".");

    char* title = NULL;

    while ((c = getopt(argc, argv, "p:u:h:x:f:e:d:r:i:l:n:t:")) != -1) {
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
            // proxy
            case 'x':
                proxy = optarg;
                break;
            // fileextension
            case 'e':
                fileext = optarg;
                break;
            // basefilename
            case 'f':
                basefilename = optarg;
                break;
            // duration
            case 'd':
                duration = optarg;
                break;
            case 'r':
                repeat = optarg;
                break;
            case 'i':
                ta = optarg;
                break;
            case 'l':
                log = optarg;
                break;
            case 'n':
                stationname = optarg;
                break;
            case 't':
                title = optarg;
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

    Stream stream;
    stream.TA=atoi(ta);

    memset(stream.basefilename, 0, 255);
    strncpy(stream.basefilename,basefilename,254);

    memset(stream.ext, 0, 255);
    strncpy(stream.ext, fileext, 254);

    memset(stream.stream_title, 0, 500);
    memset(stream.station, 0, 255);
    if (stationname != NULL) {
        strncpy(stream.stream_title, stationname, 254);
        strncpy(stream.station, stationname, 254);
    }

    stream.duration=atoi(duration);
    stream.repeat=atoi(repeat);

    memset(stream.proxy, 0, 255);
    if (proxy != NULL) {
        strncpy(stream.proxy, proxy, 254);
    }

    memset(stream.onlytitle, 0, 255);
    if (title != NULL) {
        strncpy(stream.onlytitle, title, 254);
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
