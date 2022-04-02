#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "so_stdio.h"
struct _so_file
{
    int fd;

    int pid;

    unsigned char *buffer;
    char mode[3];
    int start;
    int len_read;
    int error;
    int last_operation;
};

SO_FILE *so_fopen(const char *pathname, const char *mode)
{
    SO_FILE *aux;
    aux = (SO_FILE *)calloc(1, sizeof(SO_FILE));
    if (aux == NULL)
        return NULL;
    if (strcmp(mode, "r+") == 0)
        aux->fd = open(pathname, O_RDWR);
    else if (strcmp(mode, "w+") == 0)
        aux->fd = open(pathname, O_RDWR | O_CREAT | O_TRUNC);
    else if (strcmp(mode, "a+") == 0)
        aux->fd = open(pathname, O_RDWR | O_CREAT | O_APPEND);
    else if (strcmp(mode, "r") == 0)
        aux->fd = open(pathname, O_RDONLY);
    else if (strcmp(mode, "w") == 0)
        aux->fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC);
    else if (strcmp(mode, "a") == 0)
        aux->fd = open(pathname, O_WRONLY | O_APPEND | O_CREAT);
    else
    {
        free(aux);
        return NULL;
    }

    if (aux->fd == -1)
    {
        free(aux);
        return NULL;
    }

    aux->buffer = (unsigned char *)malloc(4096 * sizeof(unsigned char));
    if (aux->buffer == NULL)
    {
        free(aux);
        return NULL;
    }
    strcpy(aux->mode, mode);
    aux->start = 0;
    aux->len_read = 0;
    aux->last_operation = 0;
    aux->pid = 0;
    aux->error = 0;
    return aux;
}

int so_fileno(SO_FILE *stream)
{
    return stream->fd;
}

int so_fclose(SO_FILE *stream)
{
    int return_value = 0;
    int aux;

    aux = so_fflush(stream);
    if (aux != 0)
        return_value = SO_EOF;
    aux = close(stream->fd);
    if (aux != 0)
        return_value = SO_EOF;
    free(stream->buffer);
    free(stream);
    return return_value;
}

int so_fflush(SO_FILE *stream)
{
    if (stream->start > 0 && stream->last_operation == 2)
    {
        int bytes_to_dump = stream->start;
        int bytes_wrote;
        while (bytes_to_dump)
        {
            bytes_wrote = write(stream->fd, (stream->buffer) + stream->start - bytes_to_dump, bytes_to_dump);
            if (bytes_wrote == -1)
            {
                stream->start = 0;
                stream->error = 2;
                return SO_EOF;
            }
            bytes_to_dump -= bytes_wrote;
        }
    }
    memset(stream->buffer, 0, 4096);
    stream->start = 0;
    stream->last_operation = 0;
    return 0;
}
int so_fseek(SO_FILE *stream, long offset, int whence)
{
    if (stream->last_operation == 2)
    {
        int aux = so_fflush(stream);
        if (aux)
            return -1;
        stream->last_operation = 0;
    }
    else if (stream->last_operation == 1)
    {
        int aux = lseek(stream->fd, stream->start - stream->len_read, SEEK_CUR);
        memset(stream->buffer, 0, 4096);
        stream->start = 0;
        stream->last_operation = 0;
        stream->len_read = 0;
    }
    int val = lseek(stream->fd, offset, whence);
    // printf("Value of error: %d", errno);
    if (errno != 0)
        return -1;
    return 0;
}
long so_ftell(SO_FILE *stream)
{
    so_fseek(stream, 0, SEEK_CUR);
    return lseek(stream->fd, 0, SEEK_CUR);
}
size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
    int aux = 0;
    char character_value;
    while ((aux < nmemb * size) && stream->error == 0)
    {
        character_value = (unsigned char)so_fgetc(stream);
        if (stream->error == 1 || stream->error == 2)
            break;
        memcpy(ptr + aux, (stream->buffer) + (stream->start - 1), 1);
        aux++;
    }
    if (stream->error == 2)
        return 0;
    return aux / size;
}
size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
    int aux = 0;
    char character_value;
    while ((aux < nmemb * size) && stream->error == 0)
    {
        character_value = (unsigned char)so_fputc((int)(((unsigned char *)ptr)[aux]), stream);
        aux++;
    }
    if (stream->error == 2)
        return 0;
    return aux / size;
}
int so_fgetc(SO_FILE *stream)
{
    if (stream->start == stream->len_read)
    {
        int bytes_read = 0;
        memset(stream->buffer, 0, 4096);
        stream->start = 0;
        stream->len_read = 0;
        bytes_read = read(stream->fd, stream->buffer, 4096);
        // printf("Value of errno: %d\n", errno);
        if (bytes_read == 0)
        {
            stream->error = 1;
            stream->last_operation = 1;
            return SO_EOF;
        }
        else if (bytes_read == -1)
        {
            stream->error = 2;
            stream->last_operation = 1;
            return SO_EOF;
        }
        else
        {
            stream->len_read = bytes_read;
            stream->last_operation = 1;
            stream->error = 0;
            stream->start++;
            return (int)stream->buffer[stream->start - 1];
        }
    }
    else
    {
        stream->start++;
        stream->last_operation = 1;
        stream->error = 0;
        return (int)stream->buffer[stream->start - 1];
    }
}
int so_fputc(int c, SO_FILE *stream)
{
    unsigned char aux = (unsigned char)c;
    stream->last_operation = 2;
    if (stream->start == 4096)
    {
        int res = so_fflush(stream);
        if (!res)
        {
            memcpy((stream->buffer) + (stream->start), &aux, 1);
            stream->start++;
        }
        else
        {
            stream->error = 2;
            return SO_EOF;
        }
    }
    else
    {
        stream->buffer[stream->start] = aux;
        memcpy((stream->buffer) + (stream->start), &aux, 1);
        stream->start++;
    }
    return aux;
}

int so_feof(SO_FILE *stream)
{
    if (stream->error == 1)
        return SO_EOF;
    return 0;
}
int so_ferror(SO_FILE *stream)
{
    if (stream->error == 2)
    {
        return -1;
    }
    return 0;
}

SO_FILE *so_popen(const char *command, const char *type)
{
    int rc;
    SO_FILE *stream;
    stream = (SO_FILE *)calloc(1, sizeof(SO_FILE));
    stream->start = 0;
    stream->len_read = 0;
    stream->last_operation = 0;
    stream->pid = 0;
    stream->error = 0;
    if (stream == NULL)
        return NULL;
    stream->buffer = (unsigned char *)calloc(4096, sizeof(unsigned char));
    if (!strcmp(type, "r"))
    {
        strcpy(stream->mode, "r");
    }
    else if (!strcmp(type, "w"))
    {
        strcpy(stream->mode, "w");
    }
    else
    {
        free(stream->buffer);
        free(stream);
        return NULL;
    }

    int fds[2];

    rc = pipe(fds);
    if (rc != 0)
    {
        free(stream->buffer);
        free(stream);
        return NULL;
    }
    if (!strcmp(stream->mode, "r"))
        stream->fd = fds[0];
    else
        stream->fd = fds[1];

    int pid = fork();

    switch (pid)
    {
    case -1:
        close(fds[0]);
        close(fds[1]);
        free(stream->buffer);
        free(stream);

        return NULL;
    case 0:
        if (!strcmp(stream->mode, "r"))
        {
            close(fds[0]);
            dup2(fds[1], STDOUT_FILENO);
        }
        else
        {
            close(fds[1]);
            dup2(fds[0], STDIN_FILENO);
        }
        rc = execl("/bin/sh", "sh", "-c", command, (char *)0);
        if (rc)
            return NULL;

        break;
    default:
        stream->pid = pid;

        if (!strcmp(stream->mode, "r"))
            close(fds[1]);
        else
            close(fds[0]);

        break;
    }
    return stream;
}
int so_pclose(SO_FILE *stream)
{
    int status, rc;
    int pid = stream->pid;
    int fd = stream->fd;

    rc = so_fflush(stream);
    if (rc == -1)
    {
        free(stream->buffer);
        free(stream);
        return rc;
    }
    free(stream->buffer);
    free(stream);
    close(fd);

    rc = waitpid(pid, &status, 0);
    if (rc < 0)
        return -1;

    return status;
}