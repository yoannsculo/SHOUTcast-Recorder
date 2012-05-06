#ifndef __LOG_H_
#define __LOG_H_

int log_open_files(void);
int log_close_files(void);
void slog(char *line);
void slog_prog(char *line);

#endif
