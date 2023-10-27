#include <common_utils.h>

void *Malloc(size_t size,int n) {
    void* ptr = malloc(size*n);
    CHECK_ERROR_NULL(ptr, "Malloc");
    return ptr;
}

void *Calloc(size_t data,size_t size) {
    void* ptr = calloc(data,size);
    CHECK_ERROR_NULL(ptr, "Calloc");
    return ptr;
}

void Fopen(const char *filename, const char *mode, FILE **file) {
    CHECK_ERROR_NULL(*file=fopen(filename, mode),filename);
}

void *Fopen2(const char *filename, const char *mode, FILE **file)
{
    *file=fopen(filename, mode);
    return *file;
}

void myFree(void *ptr)
{
	if(ptr!=NULL)
		free(ptr);
}

void Fclose(FILE **file) {
    CHECK_ERROR_EOF(fclose(*file), "Closing file");
}

void Close(int server) {
    CHECK_ERROR_NEGATIVE(close(server), "Closing socket");
}



void cleanup(char *p, int fd) {
    
    if (p != NULL) {
        myFree(p);
        p = NULL; // Assegna NULL al puntatore per evitare un dangling pointer
    }
    if (fd != -1) {
        Close(fd);
    }
}