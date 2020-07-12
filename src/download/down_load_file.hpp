
/// @cond EXCLUDE
#if defined(__cplusplus)
 extern "C" {
#endif

#if !defined(_DOWN_LOAD_FILE_H)
#define _DOWN_LOAD_FILE_H
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int getFileName(const char * remotepath,char *fname);
/**
 * @brief down_load_file_entry
 * @param Path
 * @param url
 * @param type :device type number
 * @return
 */
int down_load_file_entry(char *Path, char* url, uint8_t type);

/**
 * @brief download_library_file, and unzip file
 * @param Path
 * @param url
 * @param type :device type number
 * @return
 */

int download_library_file(char *Path, char* url,uint8_t type);

bool getUrl(char *filename,char *url);
bool postUrl(char *filename, char *g_acHmServerAdDomain, char *field);

#endif
#ifdef __cplusplus
     }
#endif
