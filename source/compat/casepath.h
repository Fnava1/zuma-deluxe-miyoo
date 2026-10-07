#ifndef __CASEPATH_H__
#define __CASEPATH_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

int casepath(char const *path, char *r);
FILE *fcaseopen(char const *path, char const *mode);

#ifdef __cplusplus
}
#endif

#endif // __CASEPATH_H__
