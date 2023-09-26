#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>

int str_compare(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void listFiles(const char *path, int showHidden, int longFormat, int sortBySize) {
    DIR *dir;
    struct dirent *entry;
    struct stat fileStat;
    char *files[1024];
    int fileCount = 0;

    if ((dir = opendir(path)) == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (!showHidden && entry->d_name[0] == '.') {
            continue;
        }

        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        if (stat(fullpath, &fileStat) < 0) {
            perror("stat");
            exit(EXIT_FAILURE);
        }

        files[fileCount++] = strdup(entry->d_name);
    }

    closedir(dir);

    if (sortBySize) {
        qsort(files, fileCount, sizeof(char *), str_compare);
    }

    for (int i = 0; i < fileCount; i++) {
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, files[i]);

        if (stat(fullpath, &fileStat) < 0) {
            perror("stat");
            exit(EXIT_FAILURE);
        }

        if (longFormat) {
            struct passwd *user = getpwuid(fileStat.st_uid);
            struct group *group = getgrgid(fileStat.st_gid);
            
            char timebuf[64];
            strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&fileStat.st_mtime));

            printf((S_ISDIR(fileStat.st_mode)) ? "d" : "-");
            printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
            printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
            printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
            printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
            printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
            printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
            printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
            printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
            printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");
            printf(" %2ld %s %s %8ld %s %s\n",
                   (long)fileStat.st_nlink,
                   (user != NULL) ? user->pw_name : "",
                   (group != NULL) ? group->gr_name : "",
                   (long)fileStat.st_size,
                   timebuf,
                   files[i]);
        } else {
            printf("%s\n", files[i]);
        }

        free(files[i]);
    }
}

int main(int argc, char *argv[]) {
    int showHidden = 0;
    int longFormat = 0;
    int sortBySize = 0;
    const char *directory = ".";

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            for (int j = 1; argv[i][j] != '\0'; j++) {
                if (argv[i][j] == 'a') {
                    showHidden = 1;
                } else if (argv[i][j] == 'l') {
                    longFormat = 1;
                } else if (argv[i][j] == 'S') {
                    sortBySize = 1;
                }
            }
        } else {
            directory = argv[i];
        }
    }

    listFiles(directory, showHidden, longFormat, sortBySize);

    return 0;
}
