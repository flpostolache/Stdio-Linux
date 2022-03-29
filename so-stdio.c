#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "so_stdio.h"
struct _so_file
{
    int fd;
    char *buffer;
    int start;
    int end;
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
    aux->end = 0;
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