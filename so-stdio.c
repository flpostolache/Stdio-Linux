#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "so_stdio.h"
struct _so_file
{
    int fd;
    char *buffer;
    int start;
    int len_read;
    int last_operation;
};

SO_FILE *so_fopen(const char *pathname, const char *mode)
{
    SO_FILE *aux;
    aux = (SO_FILE *)calloc(1, sizeof(SO_FILE));
    if (aux == NULL)
        return NULL;
    if (strcmp(mode, "r+") == 0)
        aux->fd = open(pathname, O_RDONLY | O_WRONLY);
    else if (strcmp(mode, "w+") == 0)
        aux->fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC | O_RDONLY);
    else if (strcmp(mode, "a+") == 0)
        aux->fd = open(pathname, O_APPEND | O_CREAT | O_RDONLY);
    else if (strcmp(mode, "r") == 0)
        aux->fd = open(pathname, O_RDONLY);
    else if (strcmp(mode, "w") == 0)
        aux->fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC);
    else if (strcmp(mode, "a") == 0)
        aux->fd = open(pathname, O_APPEND | O_CREAT);
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

    aux->buffer = (char *)malloc(4096 * sizeof(char));
    if (aux->buffer == NULL)
    {
        free(aux);
        return NULL;
    }
    aux->start = 0;
    aux->len_read = 0;
    aux->last_operation = 0;
    return aux;
}

int so_fileno(SO_FILE *stream)
{
    return stream->fd;
}

int so_fclose(SO_FILE *stream)
{
    int aux;
    aux = close(stream->fd);
    if (aux == -1)
        return SO_EOF;
    free(stream->buffer);
    free(stream);
    return 0;
}

int so_fflush(SO_FILE *stream) { return NULL; }
int so_fseek(SO_FILE *stream, long offset, int whence) { return NULL; }
long so_ftell(SO_FILE *stream) { return NULL; }
size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream) { return NULL; }
size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream) { return NULL; }
int so_fgetc(SO_FILE *stream)
{
    /*if (stream->start == stream->end)
    {
        int bytes_to_read = 4096;
        int bytes_read = 0;
        memset(stream->buffer, 0, 4096);
        do
        {
            bytes_read = read(stream->fd, stream->buffer, bytes_to_read);
            bytes_to_read -= bytes_read;
        } while (bytes_to_read && bytes_read);
    }
    else
    {
    }*/
    /* int bytes_to_read = 1;
     int bytes_read = 1;
     while (bytes_to_read && bytes_read < 0)
     {
         bytes_read = read(stream->fd, (stream->buffer) + stream->end, 1);
     }*/
}
int so_fputc(int c, SO_FILE *stream) { return NULL; }

int so_feof(SO_FILE *stream) { return NULL; }
int so_ferror(SO_FILE *stream) { return NULL; }

SO_FILE *so_popen(const char *command, const char *type) { return NULL; }
int so_pclose(SO_FILE *stream) { return NULL; }