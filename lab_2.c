#include <x86_64-linux-gnu/sys/types.h>
#include <x86_64-linux-gnu/sys/stat.h>

#include <stdlib.h>
#include <dirent.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define ARGS_COUNT 3

typedef struct
{
    char *dir_name;
    int files_amount;
    off_t dir_size;
    char *largest_file;
} dirinfo;

char *prog_name;

FILE *file;

void print_error(char *prog_name, char *message, char *file_name)
{
    fprintf(stderr, "%s: %s %s\n", prog_name, message, file_name ? file_name : "");
}

void print_dir_info(char *name, int amount, int size, char *largest_file)
{
    printf("%s %d %d %s\n", name, amount, (int)size, largest_file ? largest_file : "");
    fprintf(file, "%s %d %d %s\n", name, amount, (int)size, largest_file ? largest_file : "");
}

void get_dir_info(char *dir_name)
{
    DIR *directory = opendir(dir_name);
    if (!directory)
    {
        print_error(prog_name, strerror(errno), dir_name);
        return;
    }

    struct dirent *dir_entry;
    int max_size = 0;
    dirinfo dir_info;
    
    dir_info.dir_name = (char *)malloc((strlen(dir_name) + 1) * sizeof(char));
    strcpy(dir_info.dir_name, dir_name);
    dir_info.files_amount = 0;
    dir_info.dir_size = 0;
    dir_info.largest_file = NULL;

    errno = 0;

    while ((dir_entry = readdir(directory)) != NULL)
    {
        struct stat file_info;

        char *file_name = (char *)malloc((strlen(dir_entry->d_name) + 1) * sizeof(char));
        strcpy(file_name, dir_entry->d_name);
        char *full_path = malloc((strlen(dir_name) + strlen(file_name) + 2) * sizeof(char));	

        if (strcmp(file_name, ".") == 0 || strcmp(file_name, "..") == 0)
            continue;

	strcpy(full_path, dir_name);		
	strcat(full_path, "/");
        strcat(full_path, file_name);

        if (lstat(full_path, &file_info) == -1)
        {
            print_error(prog_name, strerror(errno), file_name);
            errno = 0;
            continue;
        }

        if (S_ISDIR(file_info.st_mode))
        {
            get_dir_info(full_path);
        }
        else if (S_ISREG(file_info.st_mode))
        {
            dir_info.dir_size += file_info.st_size;
            dir_info.files_amount++;

            if (file_info.st_size >= max_size)
            {
                dir_info.largest_file = malloc((strlen(file_name) + 1) * sizeof(char));
                strcpy(dir_info.largest_file, file_name);
                max_size = file_info.st_size;
            }
        }
    }

    print_dir_info(dir_info.dir_name, dir_info.files_amount, dir_info.dir_size, dir_info.largest_file);

    if (errno)
    {
        print_error(prog_name, strerror(errno), dir_name);
    }

    if (closedir(directory) == -1)
    {
        print_error(prog_name, strerror(errno), dir_name);
    }  
}

int main(int argc, char *argv[]) 
{
    prog_name = basename(argv[0]);

    if (argc != ARGS_COUNT)
    {
        print_error(prog_name, "Wrong args amount", NULL);
        return 1;
    }

    char *dir_name = realpath(argv[1], NULL);
    if (dir_name == NULL)
    {
        print_error(prog_name, "Cannot open directory", dir_name);
        return 1;
    }

    if ((file = fopen(argv[2], "w")) == NULL)
    {
        print_error(prog_name, "Cannot open file", realpath(argv[2], NULL));
        return 1;
    }

    get_dir_info(dir_name);
    
    if (fclose(file) == -1)
    {
        print_error(prog_name, "Cannot close file", realpath(argv[2], NULL));
        return 1;
    }

    return 0;
}

