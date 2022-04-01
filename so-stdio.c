#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "so_stdio.h"
struct _so_file
{
    int fd;
    int permissions;

    int pid;

    unsigned char *buffer;
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
    {
        aux->fd = open(pathname, O_RDONLY | O_WRONLY);
        aux->permissions = O_RDONLY | O_WRONLY;
    }
    else if (strcmp(mode, "w+") == 0)
    {
        aux->fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC | O_RDONLY);
        aux->permissions = O_WRONLY | O_CREAT | O_TRUNC | O_RDONLY;
    }
    else if (strcmp(mode, "a+") == 0)
    {
        aux->fd = open(pathname, O_APPEND | O_CREAT | O_RDONLY);
        aux->permissions = O_APPEND | O_CREAT | O_RDONLY;
    }
    else if (strcmp(mode, "r") == 0)
    {
        aux->fd = open(pathname, O_RDONLY);
        aux->permissions = O_RDONLY;
    }
    else if (strcmp(mode, "w") == 0)
    {
        aux->fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC);
        aux->permissions = O_WRONLY | O_CREAT | O_TRUNC;
    }
    else if (strcmp(mode, "a") == 0)
    {
        aux->fd = open(pathname, O_APPEND | O_CREAT);
        aux->permissions = O_APPEND | O_CREAT;
    }
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
    int aux;

    aux = so_fflush(stream);
    if (aux)
        return SO_EOF;

    aux = close(stream->fd);
    if (aux == -1)
        return SO_EOF;
    free(stream->buffer);
    free(stream);
    return 0;
}

int so_fflush(SO_FILE *stream)
{
    /* if (stream->last_operation != 2)
         return SO_EOF;
     int bytes_to_dump = stream->len_read - stream->start;
     int bytes_wrote;
     while (bytes_to_dump)
     {
         bytes_wrote = write(stream->fd, (stream->buffer) + stream->start, bytes_to_dump);
         if (bytes_wrote == -1)
         {
             stream->error = 2;
             return SO_EOF;
         }
         stream->start += bytes_wrote;
         bytes_to_dump -= bytes_wrote;
     }*/
    if (stream->start > 0 && stream->last_operation == 2)
    {
        int bytes_to_dump = stream->start;
        int bytes_wrote;
        while (bytes_to_dump)
        {
            bytes_wrote = write(stream->fd, (stream->buffer) + stream->start - bytes_to_dump, bytes_to_dump);
            if (bytes_wrote == -1)
            {
                stream->error = 2;
                return SO_EOF;
            }
            bytes_to_dump -= bytes_wrote;
        }
    }
    return 0;
}
int so_fseek(SO_FILE *stream, long offset, int whence)
{
    /* if (stream->last_operation == 1)
     {
         memset(stream->buffer, 0, 4096);
         stream->start = 0;
         stream->len_read = 0;
     }
     else if (stream->last_operation == 2)
     {
         int bytes_to_dump = stream->len_read - stream->start;
         int bytes_wrote;
         while (bytes_to_dump)
         {
             bytes_wrote = write(stream->fd, (stream->buffer) + stream->start, bytes_to_dump);
             if (bytes_wrote == -1)
             {
                 stream->error = 2;
                 return SO_EOF;
             }
             stream->start += bytes_wrote;
             bytes_to_dump -= bytes_wrote;
         }
     }
     else
     {*/
    /*int return_value;
    return_value = lseek(stream->fd, offset, whence);
    if (return_value == -1)
        return -1;*/
    return 0;
    //}
}
long so_ftell(SO_FILE *stream)
{
    return lseek(stream->fd, 0, SEEK_CUR);
}
size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
    int aux = 1;
    char character_value = 1;
    while ((aux <= nmemb * size) && stream->error == 0)
    {
        character_value = (unsigned char)so_fgetc(stream);
        memcpy(ptr + (aux - 1), (stream->buffer) + (stream->start - 1), 1);
        aux++;
    }
    if (stream->error == 2 || character_value == SO_EOF)
        return 0;
    return (aux - 1) / size;
}
size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream) { return NULL; }
int so_fgetc(SO_FILE *stream)
{
    /*if (!(stream->permissions && O_RDONLY))
    {
        stream->error = 2;
        return SO_EOF;
    }*/
    // if (stream->last_operation == 2)
    //     so_fseek(stream, -1, SEEK_CUR);

    if (stream->start == stream->len_read)
    {
        int bytes_read = 0;
        memset(stream->buffer, 0, 4096);
        stream->start = 0;
        stream->len_read = 0;
        bytes_read = read(stream->fd, stream->buffer, 4096);
        if (bytes_read == 0)
        {
            stream->error = 1;
            return 0;
        }
        else if (bytes_read == -1)
        {
            stream->error = 2;
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
            stream->start = 0;
            // memcpy((stream->buffer) + (stream->start), &aux, 1);
            stream->buffer[(stream->start)] = aux;
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
        // memcpy((stream->buffer) + (stream->start), &aux, 1);
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

SO_FILE *so_popen(const char *command, const char *type) { return NULL; }
int so_pclose(SO_FILE *stream) { return NULL; }