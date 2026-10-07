#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <strings.h>

#ifdef __cplusplus
extern "C" {
#endif

int casepath(char const *path, char *r) {
    if (!path || !r) return 0;
    if (access(path, F_OK) == 0) {
        strcpy(r, path);
        return 1;
    }
    size_t len = strlen(path);
    if (len == 0) return 0;

    char current[1024];
    current[0] = '\0';

    char temp[1024];
    strncpy(temp, path, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';

    char *token = strtok(temp, "/\\");
    if (path[0] == '/' || path[0] == '\\') {
        current[0] = '/';
        current[1] = '\0';
    }

    while (token != NULL) {
        DIR *dir = opendir(current[0] ? current : ".");
        if (!dir) {
            return 0;
        }

        struct dirent *entry;
        int found = 0;
        while ((entry = readdir(dir)) != NULL) {
            if (strcasecmp(entry->d_name, token) == 0) {
                if (current[0] && current[strlen(current)-1] != '/') {
                    strcat(current, "/");
                }
                strcat(current, entry->d_name);
                found = 1;
                break;
            }
        }
        closedir(dir);
        if (!found) {
            return 0;
        }
        token = strtok(NULL, "/\\");
    }

    strcpy(r, current);
    return 1;
}

FILE *fcaseopen(char const *path, char const *mode) {
    if (!path || !mode) return NULL;
    FILE *f = fopen(path, mode);
    if (f) return f;

    char resolved[1024];
    if (casepath(path, resolved)) {
        return fopen(resolved, mode);
    }
    return NULL;
}

#ifdef __cplusplus
}
#endif
