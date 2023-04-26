#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "log.h"

static FILE *fp_log;
static char current_time[25];
static char current_date[20];
static char tempfile[270];
static FILE* fout = NULL;
static char buffr[20];

static int get_time(char *string)
{
    struct timeval curTime;
    if (string == NULL)
        return -1;
    gettimeofday(&curTime, NULL);
    int milli = curTime.tv_usec / 1000;
    strftime(buffr, 20, "%Y-%m-%d %H:%M:%S", localtime(&curTime.tv_sec));
    snprintf(string, 25, "%s.%03d ", buffr, milli);
    return 0;
}

static int get_date(char *string)
{
    struct timeval curTime;
    if (string == NULL)
        return -1;
    gettimeofday(&curTime, NULL);
    strftime(string, 20, "%a.%Y%m%d.%H%M%S", localtime(&curTime.tv_sec));
    return 0;
}

int log_open_files(char* folder)
{
    if (fp_log != NULL)
        return -1;

    if( get_date(current_date) <0) {
        printf("Couldn't get date");
        return -1;
    }
    struct stat st;
    if (stat(folder, &st) != 0)
    {
        mkdir(folder, 0777);
    }
    snprintf(tempfile,270,"%s/shoutr.%s.log",folder,current_date);
    fp_log = fopen(tempfile,"a");
    if (fp_log == NULL) {
        printf("Couldn't open shoutr.log file\n");
        return -1;
    }

    return 0;
}

int log_close_files(void)
{
    if (fclose(fp_log) == EOF)
        return EOF;

    return 0;
}

static int log_append(FILE *fp, char *line)
{
    if (fp == NULL || line == NULL)
        return -1;

    if (get_time(current_time) < 0)
        return -1;

    if (fputs(current_time, fp) == EOF)
        return EOF;

    if (fputs(line, fp) == EOF)
        return EOF;

    if (fputc('\n', fp) == EOF)
        return EOF;

    if (fflush(fp) == EOF)
        return EOF;
    else
        return 0;
}

void slog(char *line)
{
    if (log_append(fp_log, line) < 0)
        printf("Coudln't write shoutr log\n");
    if (!fout)
        fout = stdout;
    log_append(fout, line);
}

void printCurrentTime() {
    if (get_time(current_time) < 0)
        return;
    printf("%s", current_time);
}

void plog(char *fmt, ...) {
    printCurrentTime();
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}
