/* utils.c: spidey utilities */

#include "spidey.h"

#include <ctype.h>
#include <errno.h>
#include <string.h>

#include <sys/stat.h>
#include <unistd.h>

/**
 * Determine mime-type from file extension.
 *
 * @param   path        Path to file.
 * @return  An allocated string containing the mime-type of the specified file.
 *
 * This function first finds the file's extension and then scans the contents
 * of the MimeTypesPath file to determine which mimetype the file has.
 *
 * The MimeTypesPath file (typically /etc/mime.types) consists of rules in the
 * following format:
 *
 *  <MIMETYPE>      <EXT1> <EXT2> ...
 *
 * This function simply checks the file extension version each extension for
 * each mimetype and returns the mimetype on the first match.
 *
 * If no extension exists or no matching mimetype is found, then return
 * DefaultMimeType.
 *
 * This function returns an allocated string that must be free'd.
 **/
char * determine_mimetype(const char *path) {
    char *ext;
    char *mimetype;
    char *token;
    char buffer[BUFSIZ];
    FILE *fs = NULL;

    if (path == NULL){
        return DefaultMimeType;
    }

    /* Find file extension */
    char *temp = strrchr(path, '.');
    if (temp == NULL){
        //fprintf(stderr, "Can't find file extenstion: %s\n", strerror(errno));
        return DefaultMimeType;
    }
    ext = ++temp;

    /* Open MimeTypesPath file */
    fs = fopen(MimeTypesPath, "r");
    if (fs == NULL) {
        fprintf(stderr, "fopen failed: %s\n", strerror(errno));
        return DefaultMimeType;
    }

    /* Scan file for matching file extensions */
    while(fgets(buffer, BUFSIZ, fs)){
        char *c = buffer;
        token = strtok(buffer, WHITESPACE);
        while (token != NULL){
            token = skip_whitespace(token);
            if (streq(token, ext)){
                char *back = c+strlen(c)-1;
                while(c < back && isspace(*back)){
                    back--;
                }
                *(back+1) = 0;
                mimetype = strdup(c);
                return mimetype;
            }
            token = strtok(NULL, WHITESPACE);
        }

    }

    return DefaultMimeType;
}

/**
 * Determine actual filesystem path based on RootPath and URI.
 *
 * @param   uri         Resource path of URI.
 * @return  An allocated string containing the full path of the resource on the
 * local filesystem.
 *
 * This function uses realpath(3) to generate the realpath of the
 * file requested in the URI.
 *
 * As a security check, if the real path does not begin with the RootPath, then
 * return NULL.
 *
 * Otherwise, return a newly allocated string containing the real path.  This
 * string must later be free'd.
 **/
char * determine_request_path(const char *uri) {

    char buffer[BUFSIZ];
    char *combined_path = buffer;
    sprintf(combined_path, "%s%s", RootPath, uri);

    combined_path = realpath(combined_path, NULL);
    if (combined_path == NULL){
        return NULL;
    }
    *(combined_path + strlen(combined_path)) = '\0';

    if (strncmp(combined_path, RootPath, strlen(RootPath)) != 0){
        return NULL;
    }

    return combined_path;
}

/**
 * Return static string corresponding to HTTP Status code.
 *
 * @param   status      HTTP Status.
 * @return  Corresponding HTTP Status string (or NULL if not present).
 *
 * http://en.wikipedia.org/wiki/List_of_HTTP_status_codes
 **/
const char * http_status_string(HTTPStatus status) {
    static char *StatusStrings[] = {
        "200 OK",
        "400 Bad Request",
        "404 Not Found",
        "500 Internal Server Error",
        "418 I'm A Teapot",
    };
    if (status == HTTP_STATUS_OK){
        return StatusStrings[0];
    }
    else if (status == HTTP_STATUS_BAD_REQUEST){
        return StatusStrings[1];
    }
    else if (status == HTTP_STATUS_NOT_FOUND){
        return StatusStrings[2];
    }
    else if (status == HTTP_STATUS_INTERNAL_SERVER_ERROR){
        return StatusStrings[3];
    }

    return NULL;
}

/**
 * Advance string pointer pass all nonwhitespace characters
 *
 * @param   s           String.
 * @return  Point to first whitespace character in s.
 **/
char * skip_nonwhitespace(char *s) {

    if (s == NULL){
        return NULL;
    }
    while(isspace(*s) == 0){
        s++;
    }

    return s;
}

/**
 * Advance string pointer pass all whitespace characters
 *
 * @param   s           String.
 * @return  Point to first non-whitespace character in s.
 **/
char * skip_whitespace(char *s) {

    if (s == NULL){
        return NULL;
    }
    while(isspace(*s)){
        s++;
    }

    return s;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
