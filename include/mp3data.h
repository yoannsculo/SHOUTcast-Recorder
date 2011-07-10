#ifndef __MP3DATA_H_
#define __MP3DATA_H_

#include "types.h"

int is_mp3data(Stream *stream);
int mp3data_listener(Stream *stream, char *buffer);

#endif // __MP3DATA_H_
