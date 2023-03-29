#ifndef __PLS_H_
#define __PLS_H_

#define MAX_LINE_LENGTH 500

typedef struct {
	char file[MAX_LINE_LENGTH];
	char title[MAX_LINE_LENGTH];
	int length;
} PlsEntry;

typedef struct {
	unsigned int number_entries;
	PlsEntry *entries;
	int version;
} PlsFile;

int init_pls_struct(PlsFile *pls, unsigned int number_entries);
int is_pls_file(FILE *fp);
void print_pls(PlsFile *pls);

int pls_load_file(char *filename, PlsFile *pls);

#endif // __PLS_H_
