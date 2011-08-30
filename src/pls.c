#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "pls.h"

static unsigned int pls_get_number_entries(FILE *fp);
static int pls_get_field(char *buffer, char *value);
static int pls_get_entries(FILE *fp, PlsFile *pls);

void print_pls(PlsFile *pls) {
    unsigned int i;
    PlsEntry *entry = pls->entries;
    for (i=0;i<pls->number_entries;i++) {
        printf("[%2d] %s - %s\n", i, entry->file, entry->title);
        entry++;
    }
}

int pls_load_file(char *filename, PlsFile *pls) {
    unsigned int number_entries = 0; 
    //int res = 0;
    FILE *fp;
    
    fp = fopen(filename, "r");
    
    if (fp == NULL) {
        return -1;
            }

    if (!is_pls_file(fp)) {
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

int init_pls_struct(PlsFile *pls, unsigned int number_entries) {
    pls->number_entries = number_entries; 
    pls->entries = malloc(number_entries*sizeof(PlsEntry));
    pls->version = 0;
    if (pls->entries == NULL) {
        return -1;
    } else {
        unsigned int i;
        PlsEntry *entry = pls->entries;
        for (i=0;i<pls->number_entries;i++) {
            strcpy(entry->file, "\0");
            strcpy(entry->title, "\0");
            entry++;
        }
        return 0;
    }
}

static int pls_get_field(char *buffer, char *value) {
    char *ptr_begin;
    char *ptr_end;
    
    ptr_begin = strstr(buffer, "=")+1;
    ptr_end = strstr(buffer, "\n"); 
    strncpy(value, ptr_begin, (int)(ptr_end - ptr_begin));
    
    return 0;
}

static int pls_get_entries(FILE *fp, PlsFile *pls) {
    unsigned int i;
    char buffer[MAX_LINE_LENGTH];
    PlsEntry *entry = pls->entries;

    fseek(fp, 0 ,SEEK_SET);
    fgets(buffer, MAX_LINE_LENGTH, fp);
    fgets(buffer, MAX_LINE_LENGTH, fp);

    for (i=0;i<pls->number_entries;i++) {
        fgets(buffer, MAX_LINE_LENGTH, fp);
        // TODO : gérer le NULL
        
        pls_get_field(buffer, entry->file);
        
        fgets(buffer, MAX_LINE_LENGTH, fp);
        pls_get_field(buffer, entry->title);
        
        fgets(buffer, MAX_LINE_LENGTH, fp);
        // strcpy(entry->length, buffer);
        entry++;
    }
    return 0; // TODO : use a better return value 
}

int is_pls_file(FILE *fp) {
    char buffer[MAX_LINE_LENGTH];
    
    fgets(buffer, MAX_LINE_LENGTH, fp);
    if (strcmp(buffer, "[playlist]\n") != 0) {
        fseek(fp, 0 ,SEEK_SET);
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

    return FALSE;
}

static unsigned int pls_get_number_entries(FILE *fp) {
    char buffer[MAX_LINE_LENGTH];
    char number_entries_str[3];
    
    fseek(fp, 0, SEEK_SET);
    fgets(buffer, MAX_LINE_LENGTH, fp);
    fgets(buffer, MAX_LINE_LENGTH, fp);
    
    if (strstr(buffer, "NumberOfEntries=") != NULL) {
        sprintf(number_entries_str, "%c%c", buffer[16], buffer[17]);
        return (unsigned int)atoi(number_entries_str);
    } else {
        return 0;
    }
}
