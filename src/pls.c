#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "pls.h"

static unsigned int pls_get_number_entries(FILE *fp);
static int pls_get_field(char *buffer, char *value);
static int pls_get_entries(FILE *fp, PlsFile *pls);

void print_pls(PlsFile *pls)
{
    unsigned int i;
    PlsEntry *entry = pls->entries;
    for (i=0;i<pls->number_entries;i++) {
        printf("%2d %s\n", i, entry->file);
        entry++;
    }
}

int pls_load_file(char *filename, PlsFile *pls)
{
    unsigned int number_entries = 0;
    //int res = 0;
    FILE *fp;

    fp = fopen(filename, "r");

    if (fp == NULL) {
        printf("fopen(%s) failed\n", filename);
        return -1;
        }

    if (!is_pls_file(fp)) {
        printf("%s is not a pls file\n", filename);
        return -1;
    }

    number_entries = pls_get_number_entries(fp);
    init_pls_struct(pls, number_entries);
    pls_get_entries(fp, pls);

    printf("number_entries = %d\n", number_entries);

    print_pls(pls);
    fclose(fp);

    return 0;
}

int init_pls_struct(PlsFile *pls, unsigned int number_entries)
{
    pls->number_entries = number_entries;
    pls->entries = malloc(number_entries*sizeof(PlsEntry));
    pls->version = 0;
    if (pls->entries == NULL) {
        printf("no entries\n");
        return -1;
    } else {
        unsigned int i;
        PlsEntry *entry = pls->entries;
        for (i=0;i<pls->number_entries;i++) {
            memset(entry->file, 0, sizeof entry->file);
            memset(entry->title, 0, sizeof entry->file);
            entry++;
        }
        return 0;
    }
}

static int pls_get_field(char *buffer, char *value)
{
    char *ptr_begin;
    char *ptr_end;

    ptr_begin = strstr(buffer, "=")+1;
    ptr_end = strstr(buffer, "\n")-1;
    strncpy(value, ptr_begin, (int)(ptr_end - ptr_begin + 1));

    return 0;
}

static int pls_get_entries(FILE *fp, PlsFile *pls)
{
    unsigned int i;
    char buffer[MAX_LINE_LENGTH];
    PlsEntry *entry = pls->entries;

    fseek(fp, 0 ,SEEK_SET);

    i=0;
    while (i<pls->number_entries) {
        fgets(buffer, MAX_LINE_LENGTH, fp);
        if (strstr(buffer, "File") != NULL) {
            pls_get_field(buffer, entry->file);
            entry++;
            i++;
        }
    }
    return 0; // TODO : use a better return value
}

int is_pls_file(FILE *fp)
{
    char buffer[MAX_LINE_LENGTH];
    fseek(fp, 0 ,SEEK_SET);
    fgets(buffer, MAX_LINE_LENGTH, fp);

    if (strncmp(buffer, "[playlist]", 10) != 0) {
        fseek(fp, 0 ,SEEK_SET);
        printf("no [playlist] in %s\n", buffer);
        return FALSE;
    }

    // TODO : Check NumberOfEntries
    // TODO : Check goup of 3 entries

    while (!feof (fp)) {
        fgets(buffer, MAX_LINE_LENGTH, fp);
    }
    if (strstr(buffer, "Version=") != NULL) {
        fseek(fp, 0, SEEK_SET);
        return TRUE;
    }
    printf("no [Version=]\n");
    return FALSE;
}

static unsigned int pls_get_number_entries(FILE *fp)
{
    char buffer[MAX_LINE_LENGTH];
    char number_entries_str[3];

    fseek(fp, 0, SEEK_SET);
    fgets(buffer, MAX_LINE_LENGTH, fp);
    while (!feof (fp)) {
        fgets(buffer, MAX_LINE_LENGTH, fp);
        if (strstr(buffer, "NumberOfEntries=") != NULL) {
            sprintf(number_entries_str, "%c%c", buffer[16], buffer[17]);
            return (unsigned int)atoi(number_entries_str);
        }
    }
    return 0;
}
