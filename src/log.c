#include <stdio.h>
#include <time.h>

#include "log.h"

static FILE *fp_log;
static char current_time[20];
static char current_date[20];
static char tempfile[270];

static int get_time(char *string)
{
	time_t rawtime;
  	struct tm *timeinfo;
	
	if (string == NULL)
		return -1;

    	time (&rawtime);
      	timeinfo = localtime(&rawtime);

	sprintf(string, "%d:%02d:%02d-%02d:%02d:%02d ", 1900+timeinfo->tm_year,
							timeinfo->tm_mon+1,
						    	timeinfo->tm_mday,
						    	timeinfo->tm_hour,
						    	timeinfo->tm_min,
						    	timeinfo->tm_sec);

	return 0;
}

static int get_date(char *string)
{
	time_t rawtime;
  	struct tm *timeinfo;
	
	if (string == NULL)
		return -1;

    	time (&rawtime);
      	timeinfo = localtime(&rawtime);

	sprintf(string, "%d%02d%02d", 1900+timeinfo->tm_year, timeinfo->tm_mon+1, timeinfo->tm_mday);

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
}

