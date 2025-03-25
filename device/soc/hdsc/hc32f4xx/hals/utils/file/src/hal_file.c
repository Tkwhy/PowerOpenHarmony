#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <hal_file.h>
#include <utils_file.h>

typedef struct {
    char *data;
    char *path;
    int   seek;
    int   size;
}File;

#define MAX_FILES           36
#define MAX_PATH_LEN        48
#define MAX_OPEN_FILES      32    // the maximum number of open files required for adapting to openharmony xts
#define FD_OFFSET           3     // std_in, std_out, std_err

File files[MAX_FILES] = {0};
int files_open_num    = 0;

int HalFileOpen(const char *path, int oflag, int mode)
{
    int fd = -1;
    int path_len = strlen(path);

    if (files_open_num == MAX_OPEN_FILES)
        return -1;

    if (path_len > MAX_PATH_LEN) {
        return -1;
    }

    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].path != NULL && strcmp(files[i].path, path) == 0) {
            fd = i;
            break;
        }
    }

    if (fd != -1) {
        if (oflag & O_CREAT_FS && oflag & O_EXCL_FS) {
            fd = -1;
        }
        else if (oflag & O_TRUNC_FS) {
            free(files[fd].data);
            files[fd].data = NULL;
            files[fd].size = 0;
            files[fd].seek = 0;
        }
        else if (oflag & O_APPEND_FS) {
            files[fd].seek = files[fd].size;
        }
    }
    else {
        if (oflag == O_RDONLY_FS) {
            return -1;
        }

        for (int i = 0; i < MAX_FILES; i++) {
            if (files[i].path == NULL) {
                fd = i;
                break;
            }
        }

        if (fd != -1) {
            files[fd].data = NULL;
            files[fd].size = 0;
            files[fd].seek = 0;

            files[fd].path = (char*)malloc(path_len + 1);
            memcpy(files[fd].path, path, path_len);
            files[fd].path[path_len] = 0;
        }
    }

    if (fd != -1) {
        fd = fd + FD_OFFSET;
        files_open_num++;
    }

    return fd;
}

int HalFileClose(int fd)
{
    fd = fd - FD_OFFSET;

    if (fd < 0 || fd >= MAX_FILES || files[fd].path == NULL) {
        return -1;
    }

    if (files_open_num !=0 ) {
        --files_open_num;
    }

    files[fd].seek = 0;

    return 0;
}

int HalFileRead(int fd, char *buf, unsigned int len) {
    fd = fd - FD_OFFSET;

    if (fd < 0 || fd >= MAX_FILES || files[fd].path == NULL) {
        return -1;
    }

    if (len == 0) {
        return 0;
    }

    int bytesRead = 0;

    if (files[fd].seek < files[fd].size) {
        bytesRead = len;
        if (bytesRead > files[fd].size - files[fd].seek) {
            bytesRead = files[fd].size - files[fd].seek;
        }
        memcpy(buf, files[fd].data + files[fd].seek, bytesRead);
        files[fd].seek += bytesRead;
    }

    return bytesRead;
}

int HalFileWrite(int fd, const char *buf, unsigned int len)
{
    fd = fd - FD_OFFSET;

    if (fd < 0 || fd >= MAX_FILES || files[fd].path == NULL) {
        return -1;
    }

    if (len == 0) {
        return 0;
    }

    int bytesWritten = 0;

    if (files[fd].seek + len > files[fd].size) {
        char *newData = (char*)realloc(files[fd].data, files[fd].seek + len);
        if (newData == NULL) {
            return -1;
        }
        files[fd].data = newData;
        files[fd].size = files[fd].seek + len;
    }

    memcpy(files[fd].data + files[fd].seek, buf, len);
    files[fd].seek += len;
    bytesWritten = len;

    return bytesWritten;
}

int HalFileDelete(const char *path)
{
    int result = -1;

    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].path != NULL && strcmp(files[i].path, path) == 0) {
            free(files[i].data);
            files[i].data = NULL;
            files[i].size = 0;
            free(files[i].path);
            files[i].path = NULL;
            files[i].seek = 0;
            result = 0;
            break;
        }
    }

    return result;
}

int HalFileStat(const char *path, unsigned int *fileSize)
{
    int result = -1;

    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].path != NULL && strcmp(files[i].path, path) == 0) {
            *fileSize = files[i].size;
            result = 0;
            break;
        }
    }

    return result;
}

int HalFileSeek(int fd, int offset, unsigned int whence)
{
    fd = fd - FD_OFFSET;

    if (fd < 0 || fd >= MAX_FILES || files[fd].path == NULL) {
        return -1;
    }

    int newSeek = 0;

    switch (whence) {
        case SEEK_SET:
            newSeek = offset;
            break;
        case SEEK_CUR:
            newSeek = files[fd].seek + offset;
            break;
        case SEEK_END:
            newSeek = files[fd].size + offset;
            break;
        default:
            return -1;
    }

    if (newSeek < 0 || newSeek > files[fd].size) {
        return -1;
    }

    files[fd].seek = newSeek;

    return 0;
}
