#include <stdio.h>
#include <string.h>

#include "types.h"
#include "files.h"

int get_extension(char *extension, char *string)
{
    char *pstr = NULL;

    if (string == NULL)
        return -1;

    pstr = strrchr(string, '.');
    if (pstr == NULL) {
        strcpy(extension, "");
        return -1;
    }

    strcpy(extension, pstr+1);

    return 0;
}

int is_pls_extension(char *string)
{
    char extension[10];

    if (string == NULL)
        return FALSE;

    get_extension(extension, string);

    if (!strcmp(extension, "pls"))
        return TRUE;
    else
        return FALSE;
}

