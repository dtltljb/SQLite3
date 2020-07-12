#include <stdio.h>
#include <stdint.h>

#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/file.h>

#include <dirent.h>
#include <unistd.h>



#include 		"forlder_util.h"

#ifdef Debug_Level
#undef Debug_Level
#endif

#define	Debug_Level		0		//0 disable printf


/**
 * @brief list_files_recursively
 * @param base_path
 * @param list
 */

void list_files_recursively(const char * base_path, std::vector<std::string> & list)
{
    char path[256];
    struct dirent *dp;
    DIR *dir = opendir(base_path);

    if (!dir) {
        std::cout << "Open DIR failed: " << base_path << std::endl;
        return;
    }

    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            //std::cout << "file name: " << dp->d_name << std::endl;
            strcpy(path, base_path);
            strcat(path, "/");
            strcat(path, dp->d_name);

            if (DT_DIR == dp->d_type) {
                //std::cout << "DIR: " << path << std::endl;
                list_files_recursively(path, list);
            } else if (DT_REG == dp->d_type) {
                //std::cout << "File: " << path << std::endl;
                if (strstr(dp->d_name,".png") != nullptr) {
                    std::string file_name_string(path);
                    list.push_back(file_name_string);
                }
            }
        }
    }
    closedir(dir);
}

/*
*		Function name:	create_multi_dir
*		brief :create multi dir
*		input param:			char* path
*		return reslut:			0 success, non 0 failure
*		eg:path = "./2017/11/"
******************************************************************/
int create_multi_dir(const char *path)
{
        int i, len,ret;

        len = strlen(path);
        char dir_path[len+1];
        dir_path[len] = '\0';

        strncpy(dir_path, path, len);

        for (i=0; i<len; i++)
        {
                if (dir_path[i] == '/' && i > 0)
                {
                        dir_path[i]='\0';
                        if( (ret=access(dir_path, F_OK) )< 0)
                        {
                                if (mkdir(dir_path, 0755) < 0)
                                {
                                        printf("mkdir=%s:msg=%s\n", dir_path, strerror(errno));
                                        return -1;
                                }
                        }
                        dir_path[i]='/';
                }
        }
        return 0;
}

/*
*		Function name:	GetFileFromDir
*		brief :					Get File From Dir 递归检索指定文件,返回完整路径
*		input param:		char* path
*		output param: 	char*	file,note:*file place length >= 128 bytes
*		return reslut:	0 success, non 0 failure
******************************************************************/
int GetFileFromDir(char* path,char* path_file) 
{
	DIR *dir;
	struct dirent *dt;
	char filename1[128];
	
  memset(filename1,0,sizeof(filename1));
  strcpy(filename1,path);

	if((dir = opendir(path)) == NULL)
	{
		#if (Debug_Level == 1)
		printf("%s, line:%d,[opendir %s error]\n",__FILE__,__LINE__,path); 
		#endif
        closedir(dir);
		return -1; 
	}

	while((dt = readdir(dir)) != NULL){ 
    if(strcmp(dt->d_name,".")==0||strcmp(dt->d_name,"..")==0){ 
				continue; 
		}
		//如果这个目录里 还有目录，可以在这加判断 
		strcat(filename1,dt->d_name);
		strcpy(path_file,filename1);
    closedir(dir);
		return	0;
	}
    closedir(dir);
	return	-2;
}
