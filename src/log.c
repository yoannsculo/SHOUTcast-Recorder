#include <stdio.h>
#include <time.h>

#include "log.h"

static FILE *fp_log;
static FILE *fp_prog;

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

int log_open_files(void)
{
	fp_log = fopen("shoutr.log","a");
	if (fp_log == NULL) {
		printf("Couldn't open log file\n");
		return -1;
	}

	fp_prog = fopen("prog.log","a");
	if (fp_prog == NULL) {
		printf("Couldn't open log file\n");
		fclose(fp_log);
		return -1;
	}

	return 0;
}

int log_close_files(void)
{
	if (fclose(fp_log) == EOF)
		return EOF;

	if (fclose(fp_prog) == EOF)
		return EOF;

	return 0;
}

static int log_append(FILE *fp, char *line)
{
	char current_time[20];

	if (fp == NULL || line == NULL)
		return -1;

	if (get_time(current_time) < 0)
		return -1;

	if (fputs(current_time, fp) == EOF)
		return EOF;

	if (fputs(line, fp) == EOF)
		return EOF;

	if (fflush(fp) == EOF)
		return EOF;
	else
		return 0;
}

void slog(char *line)
{
	log_append(fp_log, line);
}

void slog_prog(char *line)
{
	if (log_append(fp_prog, line) < 0)
		printf("Coudln't write prog log\n");
}
