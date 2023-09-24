#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>

static void format_utc_time(time_t ts, char utc_time[13]);
static void del_file(const char *path_ptr);

time_t g_limit_t = NULL;

int main(int argc, const char* argv[]) {

    if(argc != 3)
    {
        fprintf(stderr, "usage: %s <dir> <days>\n", argv[0]);
        exit(-1);
    }
    const char *del_dir_ptr = argv[1];
    const int days = atoi(argv[2]);

    int offset_seconds = -days * 24 * 3600;
    time_t now = time(NULL);
    g_limit_t = now + offset_seconds;

    del_file(del_dir_ptr);

    return 0;
}

static void format_utc_time(time_t ts, char utc_time[17])
{
    struct tm *t = gmtime(&ts);

    sprintf(utc_time, "%04d", t->tm_year + 1900);
    sprintf(utc_time + 4, "-%02d", t->tm_mon + 1);
    sprintf(utc_time + 7, "-%02d", t->tm_mday);
    sprintf(utc_time + 10, " %02d", t->tm_hour);
    sprintf(utc_time + 13, ":%02d", t->tm_min);
    utc_time[16] = 0;
}

static void del_file(const char *path_ptr)
{
    DIR *d_ptr;
    struct dirent *file_ptr;
    struct stat st;
    char full_path[PATH_MAX] = {0};
    time_t ct = NULL;
    char utc_time[17] = {0};

    if(!(d_ptr = opendir(path_ptr)))
    {
        return;
    }

    while((file_ptr = readdir(d_ptr)) != NULL)
    {
        if(strncmp(file_ptr->d_name, ".", 1) == 0)
        {
            continue;
        }

        memset(full_path, 0, sizeof(full_path) / sizeof(char));
        sprintf(full_path, "%s/%s", path_ptr, file_ptr->d_name);

        if(stat(full_path, &st) == 0)
        {
            if(S_ISDIR(st.st_mode))
            {
                del_file(full_path);
            }
            else if(S_ISREG(st.st_mode))
            {
                ct = st.st_ctime;
                format_utc_time(ct, utc_time);
                if(ct < g_limit_t)
                {
                    printf("%s %s\n", utc_time, full_path);
                    remove(full_path);
                }
            }
        }
    }
    closedir(d_ptr);
}