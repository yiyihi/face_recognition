#ifndef _BASE64_H
#define _BASE64_H
 
#include <stdlib.h>
#include <string.h>
 
char *base64_encode_file(const unsigned char * bindata, char * base64, int binlength);
 
#endif